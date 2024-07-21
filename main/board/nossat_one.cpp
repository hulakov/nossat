#include "board/board.h"

#include "system/event_loop.h"
#include "system/interrupt_manager.h"
#include "system/resource_manager.h"
#include "system/task.h"

#include "hal/led.h"
#include "hal/knob.h"
#include "hal/audio_input.h"
#include "hal/audio_output.h"

#if CONFIG_NOSSAT_LVGL_GUI
#include "lvgl/gui_one.h"
#endif

#include "WiFiHelper.h"
#include "network/mqtt_manager.h"

#if CONFIG_NOSSAT_SPEECH_RECOGNITION
#include "sound/speech_recognition.h"
#else
#endif

#include <thread>

#include "bsp/esp-bsp.h"
#include "secrets.h"

static const char *DEVICE_NAME = "nossat_one";
static const char *TAG = "board";

auto event_loop = std::make_shared<EventLoop>();
auto interrupt_manager = std::make_shared<InterruptManager>(event_loop);
ResourceManager resource_manager;

auto led = std::make_shared<Led>();
auto left_encoder = std::make_shared<Knob>(GPIO_NUM_14, GPIO_NUM_13, GPIO_NUM_9);
auto right_encoder = std::make_shared<Knob>(GPIO_NUM_17, GPIO_NUM_18, GPIO_NUM_8);

std::shared_ptr<Display> display;
std::shared_ptr<Gui> gui;

auto audio_input = std::make_shared<AudioInput>();
auto audio_output = std::make_shared<AudioOutput>();

WiFiHelper
    wifi_helper(DEVICE_NAME, []() { ESP_LOGI(TAG, "WiFI Connected"); }, []() { ESP_LOGI(TAG, "WiFI Disconnected"); });
std::unique_ptr<MqttManager> mqtt_manager;

void initialize_knobs()
{
    left_encoder->set_step_value(2);
    left_encoder->set_value_changed_handler([](int value) { ESP_LOGI(TAG, "[left] changed %d", value); });
    left_encoder->set_click_handler([]() { ESP_LOGI(TAG, "[left] click"); });
    left_encoder->initialize(event_loop);

    right_encoder->set_step_value(2);
    right_encoder->set_value_changed_handler(
        [](int value)
        {
            ESP_LOGI(TAG, "[right] volume %d", value);
#if CONFIG_NOSSAT_LVGL_GUI
            gui->set_value(value);
#endif
        });
    right_encoder->set_click_handler(
        []()
        {
            ESP_LOGI(TAG, "[right] click");
            {
                auto copy = resource_manager.not_recognized_wav;
                copy.adjust_volume(right_encoder->get_value() / 100.0);
                audio_output->play(copy);
            }
        });
    right_encoder->initialize(event_loop);
    right_encoder->set_value(10);
}

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

void audio_feed_task()
{
    const size_t audio_chunksize = speech_recognition->get_feed_chunksize();
    const AudioFormat audio_format = audio_input->get_audio_format();

    ESP_LOGI(TAG, "Run audio feed task: num_channels %lu, bits_per_sample %lu, sample_rate %lu",
             audio_format.num_channels, audio_format.bits_per_sample, audio_format.sample_rate);

    AudioData audio;
    while (true)
    {
        audio.set_format(audio_format, audio_chunksize);
        audio_input->capture_audio(audio);
        audio.add_channels(SpeechRecognition::REFERENCE_CHANNEL_COUNT);
        speech_recognition->feed(audio);
    }
}

void initialize_speech_recognition()
{
    speech_recognition =
        std::make_shared<SpeechRecognition>(event_loop, std::make_unique<SpeechRecognitionObserver>(), audio_input);

    ESP_LOGI(TAG, "******* Start tasks *******");
    create_task(audio_feed_task, "Feed Task", 4 * 1024, 5, 1);
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

void start()
{
    ESP_LOGI(TAG, "******* Initialize Interrupts and Events *******");
    interrupt_manager->initialize();
    create_task(std::bind(&EventLoop::run, event_loop), "Handle Task", 4 * 1024, configMAX_PRIORITIES - 1, 0);

    led->solid(0, 0, 255);

#if CONFIG_NOSSAT_LVGL_GUI
    display = std::make_shared<Display>();
    gui = std::make_shared<Gui>(display);
    gui->show_message("Hello!");
#else
    ESP_LOGI(TAG, "******* GUI is disabled *******");
#endif

    ESP_LOGI(TAG, "******* Initialize Controls *******");
    ESP_LOGI(TAG, "Initialize Encoders");
    initialize_knobs();

    ESP_LOGI(TAG, "******* Initialize Networking *******");
    ESP_LOGI(TAG, "Connect to WiFi");
    ESP_TRUE_CHECK(wifi_helper.connectToAp(WIFI_SSID, WIFI_PASSWORD, true, 5 * 60 * 1000));

    ESP_LOGI(TAG, "Connect to MQTT");
    mqtt_manager = std::make_unique<MqttManager>(DEVICE_NAME);

#if CONFIG_NOSSAT_SPEECH_RECOGNITION
    ESP_LOGI(TAG, "******* Initialize Speech Recognition *******");
    initialize_speech_recognition();
#else
    ESP_LOGI(TAG, "******* Speech Recognition is disabled *******");
#endif

    ESP_LOGI(TAG, "******* Ready! *******");

    std::this_thread::sleep_for(std::chrono::seconds(1));
#if CONFIG_NOSSAT_LVGL_GUI
    gui->hide_message();
#endif
    led->clear();
}