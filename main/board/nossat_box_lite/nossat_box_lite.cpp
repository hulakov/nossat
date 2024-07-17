#include "board/board.h"

#include "system/event_loop.h"
#include "system/resource_manager.h"
#include "system/task.h"

#include "hal/audio_input/audio_input.h"
#include "hal/audio_output/audio_output.h"

#include "WiFiHelper.h"
#include "network/mqtt_manager.h"
#include "speech_recognition/speech_recognition.h"
#include "lvgl/speech/lvgl_ui.h"
#include "esp_log.h"
#include "bsp/esp-bsp.h"

#include "secrets.h"
#include "sanity_checks.h"

#include <thread>
#include <memory>

static const char *DEVICE_NAME = "nossat_box_lite";
static const char *TAG = "board";

auto event_loop = std::make_shared<EventLoop>();
ResourceManager resource_manager;

button_handle_t button = nullptr;
int btn_num = 0;

std::shared_ptr<AudioInput> audio_input;
std::shared_ptr<AudioOutput> audio_output;

WiFiHelper
    wifi_helper(DEVICE_NAME, []() { ESP_LOGI(TAG, "WiFI Connected"); }, []() { ESP_LOGI(TAG, "WiFI Disconnected"); });
std::unique_ptr<MqttManager> mqtt_manager;
std::shared_ptr<SpeechRecognition> speech_recognition;

class SpeechRecognitionObserver : public SpeechRecognition::IObserver
{
public:
    void on_command_not_detected() override
    {
        ui_show_message("Timeout");
        bsp_display_backlight_on();
        audio_output->play(resource_manager.not_recognized_wav);
        vTaskDelay(pdMS_TO_TICKS(1000));
        ui_hide_message();
        bsp_display_backlight_off();
    }

    void on_waiting_for_command() override
    {
        ui_show_message("Say command", true);
        bsp_display_backlight_on();
        audio_output->play(resource_manager.wake_wav);
    }

    void on_command_handling_started(const char *message) override
    {
        ui_show_message(message);
        bsp_display_backlight_on();
    }

    void on_command_handling_finished() override
    {
        audio_output->play(resource_manager.recognized_wav);
        vTaskDelay(pdMS_TO_TICKS(1000));
        ui_hide_message();
        bsp_display_backlight_off();
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
    create_task(std::bind(&SpeechRecognition::audio_feed_task, speech_recognition), "Feed Task", 4 * 1024, 5, 1);
    auto detect_task = []()
    {
        ESP_LOGI(TAG, "******* Add Speech Recognition Commands *******");
        add_commands();
        ESP_LOGI(TAG, "******* Start Speech Recognition *******");
        speech_recognition->audio_detect_task();
    };
    create_task(detect_task, "Detect Task", 8 * 1024, 5, 0);
}

void start()
{
    ESP_LOGI(TAG, "******* Initialize Events *******");
    create_task(std::bind(&EventLoop::run, event_loop), "Handle Task", 4 * 1024, configMAX_PRIORITIES - 1, 0);

    ESP_LOGI(TAG, "******* Initialize UI *******");

    ESP_LOGI(TAG, "Initialize display");
    const bsp_display_cfg_t cfg = {
        .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
        .buffer_size = BSP_LCD_H_RES * CONFIG_BSP_LCD_DRAW_BUF_HEIGHT,
        .double_buffer = 0,
        .flags =
            {
                .buff_dma = true,
                .buff_spiram = false,
            },
    };
    lv_disp_t *disp = bsp_display_start_with_config(&cfg);

    ESP_LOGI(TAG, "Configure LVGL");
    ui_initialize(disp);

    ui_show_message("Hello!");

    ESP_LOGI(TAG, "******* Initialize Audio *******");
    audio_input = std::make_shared<AudioInput>();
    audio_output = std::make_shared<AudioOutput>();

    ESP_LOGI(TAG, "******* Initialize Controls *******");
    ESP_ERROR_CHECK(bsp_iot_button_create(&button, &btn_num, BSP_BUTTON_NUM));

    ESP_LOGI(TAG, "******* Initialize Networking *******");
    ESP_LOGI(TAG, "Connect to WiFi");
    ESP_TRUE_CHECK(wifi_helper.connectToAp(WIFI_SSID, WIFI_PASSWORD, true, 5 * 60 * 1000));

    ESP_LOGI(TAG, "Connect to MQTT");
    mqtt_manager = std::make_unique<MqttManager>(DEVICE_NAME);

    ESP_LOGI(TAG, "******* Initialize Speech Recognition *******");
    initialize_speech_recognition();

    ESP_LOGI(TAG, "******* Ready! *******");

    std::this_thread::sleep_for(std::chrono::seconds(1));
    ui_hide_message();
    bsp_display_backlight_off();
}