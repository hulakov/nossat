#include "board/board.h"

#include "system/event_loop.h"
#include "system/interrupt_manager.h"
#include "system/resource_manager.h"
#include "system/task.h"

#include "hal/led.h"
#include "hal/audio_input.h"
#include "hal/audio_output.h"

#if CONFIG_NOSSAT_LVGL_GUI
#include "gui/gui_one.h"
#endif

#include "WiFiHelper.h"
#include "network/mqtt_manager.h"

#include "gui/nossat-one/src/ui/screens.h"

#if CONFIG_NOSSAT_SPEECH_RECOGNITION
#include "sound/speech_recognition.h"
#else
#endif

#include <thread>
#include <mutex>

#include "bsp/esp-bsp.h"
#include "secrets.h"
#include "esp_sntp.h"

static const char *DEVICE_NAME = "nossat_one";
static const char *TAG = "board";

auto event_loop = std::make_shared<EventLoop>();
auto interrupt_manager = std::make_shared<InterruptManager>(event_loop);
ResourceManager resource_manager;

auto led = std::make_shared<Led>();

std::shared_ptr<Display> display;
std::shared_ptr<Gui> gui;

auto audio_input = std::make_shared<AudioInput>();
auto audio_output = std::make_shared<AudioOutput>();

WiFiHelper
    wifi_helper(DEVICE_NAME, []() { ESP_LOGI(TAG, "WiFI Connected"); }, []() { ESP_LOGI(TAG, "WiFI Disconnected"); });
std::unique_ptr<MqttManager> mqtt_manager;

#ifdef __cplusplus
extern "C"
{
#endif

std::atomic<bool> recording = false;
AudioData recorded_audio;
std::mutex recorded_audio_mutex;

void action_on_start_recording(lv_event_t *e)
{
    ESP_LOGI(TAG, "Start recording");
    if (!recording)
    {
        {
            std::unique_lock<std::mutex> lock(recorded_audio_mutex);
            recorded_audio = {};
        }
        gui->show_recording_screen();
        recording = true;
    }
}

void action_on_stop_recording(lv_event_t *e)
{
    ESP_LOGI(TAG, "Stop recording");
    if (recording)
    {
        // drop ending to avoid click
        const uint32_t samples_per_250ms =
            std::min(static_cast<size_t>(recorded_audio.get_sample_rate() / 4), recorded_audio.get_num_samples());
        recorded_audio.resize(recorded_audio.get_num_samples() - samples_per_250ms);

        {
            ESP_LOGI(TAG, "Start playing");
            std::unique_lock<std::mutex> lock(recorded_audio_mutex);
            audio_output->play(recorded_audio);
            ESP_LOGI(TAG, "End playing");
        }
        gui->show_current_page();
        recording = false;
    }
}

#ifdef __cplusplus
}
#endif

#if CONFIG_NOSSAT_SPEECH_RECOGNITION

std::shared_ptr<SpeechRecognition> speech_recognition;

class SpeechRecognitionObserver : public SpeechRecognition::IObserver
{
public:
    void on_command_not_detected() override
    {
#if CONFIG_NOSSAT_LVGL_GUI
        gui->show_message("Timeout");
#endif
        led->solid(255, 0, 0);
        audio_output->play(resource_manager.not_recognized_wav);
        vTaskDelay(pdMS_TO_TICKS(1000));
#if CONFIG_NOSSAT_LVGL_GUI
        gui->hide_message();
#endif
        led->clear();
    }

    void on_waiting_for_command() override
    {
#if CONFIG_NOSSAT_LVGL_GUI
        gui->show_message("Say command");
#endif
        led->solid(255, 255, 255);
        audio_output->play(resource_manager.wake_wav);
    }

    void on_command_handling_started(const char *message) override
    {
        led->solid(0, 255, 0);
#if CONFIG_NOSSAT_LVGL_GUI
        gui->show_message(message);
#endif
    }

    void on_command_handling_finished() override
    {
        audio_output->play(resource_manager.recognized_wav);
        vTaskDelay(pdMS_TO_TICKS(1000));
#if CONFIG_NOSSAT_LVGL_GUI
        gui->hide_message();
#endif
        led->clear();
    }
};

void add_command(std::vector<const char *> commands)
{
    ESP_TRUE_CHECK(commands.size() > 0);

    auto ha_event = mqtt_manager->add_event(commands[0]);
    const auto handler = [ha_event]() { ha_event->publishEvent(VOICE_COMMAND_EVENT_TYPE); };

    speech_recognition->add_command(commands, handler);
}

void add_commands()
{
    speech_recognition->begin_add_commands();

    add_command({"Toggle the Light", "Light"});
    add_command({"Turn On the Light", "Switch On the Light"});
    add_command({"Turn Off the Light", "Switch Off the Light"});
    add_command({"Turn White"});
    add_command({"Turn Warm White"});
    add_command({"Turn Red"});
    add_command({"Turn Green"});
    add_command({"Turn Blue"});
    add_command({"Turn Pink"});
    add_command({"Tiny Light", "Turn On the Tiny Light"});

    add_command({"Play Cartoons", "Cartoons"});
    add_command({"Play Music", "Music"});

    add_command({"Play"});
    add_command({"Pause"});
    add_command({"Resume"});
    add_command({"Stop"});
    add_command({"Mute"});
    add_command({"Unmute"});

    speech_recognition->end_add_commands();
}

void initialize_speech_recognition()
{
    speech_recognition =
        std::make_shared<SpeechRecognition>(event_loop, std::make_unique<SpeechRecognitionObserver>(), audio_input);

    ESP_LOGI(TAG, "******* Start tasks *******");
    auto detect_task = []()
    {
        ESP_LOGI(TAG, "******* Add Speech Recognition Commands *******");
        add_commands();
        ESP_LOGI(TAG, "******* Start Speech Recognition *******");
        speech_recognition->audio_detect_task();
    };
    create_task(detect_task, "Detect Task", 8 * 1024, 5, 0);
}
#endif

void audio_feed_task()
{
#if CONFIG_NOSSAT_SPEECH_RECOGNITION
    const size_t audio_chunksize = speech_recognition->get_feed_chunksize();
#else
    // 16000
    const size_t audio_chunksize = 1024;
#endif

    const AudioFormat audio_format = audio_input->get_audio_format();
    ESP_LOGI(TAG, "Run audio feed task: num_channels %lu, bits_per_sample %lu, sample_rate %lu",
             audio_format.num_channels, audio_format.bits_per_sample, audio_format.sample_rate);

    // num samples per 100 ms
    const size_t vis_step = audio_format.sample_rate * 0.05;
    size_t vis_pos = 0;

    AudioData audio;

    while (true)
    {
        audio.set_format(audio_format, audio_chunksize);
        audio_input->capture_audio(audio);

        if (recording)
        {
            std::vector<int32_t> values;
            while (vis_pos < audio.get_num_samples())
            {
                values.push_back(audio.get_value(vis_pos, 0));
                vis_pos += vis_step;
            }
            vis_pos %= audio.get_num_samples();
            gui->add_recording_data(values);

            std::unique_lock<std::mutex> lock(recorded_audio_mutex);
            if (recorded_audio.is_empty())
                recorded_audio = audio;
            else
                recorded_audio.join(audio);
        }
#if CONFIG_NOSSAT_SPEECH_RECOGNITION
        audio.add_channels(SpeechRecognition::REFERENCE_CHANNEL_COUNT);
        speech_recognition->feed(audio);
#endif
    }
}

void initialize_sntp()
{
    ESP_LOGI(TAG, "Initializing SNTP");
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_init();

    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void start()
{
    ESP_LOGI(TAG, "******* Initialize Interrupts and Events *******");
    interrupt_manager->initialize();
    create_task(std::bind(&EventLoop::run, event_loop), "Handle Task", 4 * 1024, configMAX_PRIORITIES - 1, 0);

    led->solid(0, 0, 255);

#if CONFIG_NOSSAT_LVGL_GUI
    display = std::make_shared<Display>();
    gui = std::make_shared<Gui>(display, event_loop);
    gui->show_message("Hello!");
    display->enable_backlight();
    std::this_thread::sleep_for(std::chrono::seconds(1));
#else
    ESP_LOGI(TAG, "******* GUI is disabled *******");
#endif

    ESP_LOGI(TAG, "******* Initialize Networking *******");
    gui->show_message("Connecting to Wi-Fi...");
    ESP_LOGI(TAG, "Connect to Wi-Fi");
    ESP_TRUE_CHECK(wifi_helper.connectToAp(WIFI_SSID, WIFI_PASSWORD, true, 5 * 60 * 1000));

    // // FIXME add support of different time zones
    setenv("TZ", "EET-2EEST,M3.5.0/3,M10.5.0/4", 1);
    tzset();

    ESP_LOGI(TAG, "Initializing SNTP");
    gui->show_message("Initializing SNTP...");
    initialize_sntp();

    ESP_LOGI(TAG, "Connect to MQTT");
    gui->show_message("Connecting to MQTT...");
    mqtt_manager = std::make_unique<MqttManager>(DEVICE_NAME);

#if CONFIG_NOSSAT_SPEECH_RECOGNITION
    gui->show_message("Configuring Speech Recognition...");
    ESP_LOGI(TAG, "******* Initialize Speech Recognition *******");
    initialize_speech_recognition();
#else
    ESP_LOGI(TAG, "******* Speech Recognition is disabled *******");
#endif

    ESP_LOGI(TAG, "******* Start audio capturing *******");
    create_task(audio_feed_task, "Feed Task", 4 * 1024, 5, 1);

    ESP_LOGI(TAG, "******* Ready! *******");

    gui->show_message("Ready!");
    std::this_thread::sleep_for(std::chrono::seconds(1));

#if CONFIG_NOSSAT_LVGL_GUI
    gui->show_current_page();
#endif
    led->clear();
}