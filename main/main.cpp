#include "board/board.h"
#include "esp_log.h"
#include "sanity_checks.h"
#include "secrets.h"
#include "speech_recognition.h"

#include <HaBridge.h>
#include <MQTTRemote.h>
#include <entities/HaEntityEvent.h>

#include <WiFiHelper.h>
#include <nlohmann/json.hpp>

#include <memory>
#include <thread>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";
static const char *DEVICE_NAME = "nossat_kitchen";
static const std::string VOICE_COMMAND_EVENT_TYPE = "voice_command";

WiFiHelper
    wifi_helper(DEVICE_NAME, []() { ESP_LOGI(TAG, "WiFI Connected"); }, []() { ESP_LOGI(TAG, "WiFI Disconnected"); });

nlohmann::json make_device_doc_json();

MQTTRemote mqtt_remote(DEVICE_NAME, MQTT_HOSTNAME, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD);
nlohmann::json json_this_device_doc = make_device_doc_json();
HaBridge ha_bridge(mqtt_remote, "test", json_this_device_doc);
std::vector<std::shared_ptr<HaEntityEvent>> ha_events;

std::string command_to_event_id(std::string input)
{
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
    std::replace(input.begin(), input.end(), ' ', '_');
    return input + "_event";
}

void add_command(std::vector<const char *> commands)
{
    ENSURE_TRUE(commands.size() > 0);

    const std::string command = commands[0];
    const std::string id = command_to_event_id(command);

    std::shared_ptr<HaEntityEvent> ha_event(new HaEntityEvent(ha_bridge, command, id, {VOICE_COMMAND_EVENT_TYPE}));
    ha_event->publishConfiguration();
    ha_events.push_back(ha_event);

    const auto handler = [ha_event]() { ha_event->publishEvent(VOICE_COMMAND_EVENT_TYPE); };

    SpeechRecognition::instance().add_command(commands, handler);
}

void add_commands()
{
    SpeechRecognition::instance().begin_add_commands();

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

    SpeechRecognition::instance().end_add_commands();
}

nlohmann::json make_device_doc_json()
{
    nlohmann::json json;
    json["identifiers"] = DEVICE_NAME;
    json["name"] = "Nossat Kitchen";
    json["sw_version"] = "1.0.0";
    json["model"] = "Nossat";
    json["manufacturer"] = "Vadym";
    return json;
}

extern "C"
{
void app_main(void)
{
    ESP_LOGI(TAG, "Nosyna Satelite is starting!");
    ESP_LOGI(TAG, "Compile time: %s %s", __DATE__, __TIME__);

    ESP_LOGI(TAG, "******* Initialize Board *******");
    Board::instance().initialize();
    Board::instance().show_message(Board::HELLO, "Hello!");

    ESP_LOGI(TAG, "******* Initialize Networking *******");
    ESP_LOGI(TAG, "Connect to WiFi");
    ENSURE_TRUE(wifi_helper.connectToAp(WIFI_SSID, WIFI_PASSWORD));

    ESP_LOGI(TAG, "Connect to MQTT");
    mqtt_remote.start();
    while (!mqtt_remote.connected())
        vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGI(TAG, "******* Initialize Speech Recognition *******");
    SpeechRecognition::instance().initialize();

    ESP_LOGI(TAG, "******* Start tasks *******");
    TaskHandle_t feed_task;
    TaskFunction_t audio_feed_task_adapter = [](void *)
    {
        SpeechRecognition::instance().audio_feed_task();
        vTaskDelete(NULL);
    };
    ENSURE_TRUE(xTaskCreatePinnedToCore(audio_feed_task_adapter, "Feed Task", 4 * 1024, nullptr, 5, &feed_task, 0));

    TaskHandle_t detect_task;
    TaskFunction_t audio_detect_task_adapter = [](void *arg)
    {
        SpeechRecognition::instance().audio_detect_task();
        vTaskDelete(NULL);
    };
    ENSURE_TRUE(xTaskCreatePinnedToCore(audio_detect_task_adapter, "Speech Recognition Task", 4 * 1024, nullptr, 5,
                                        &detect_task, 1));

    TaskHandle_t handle_task;
    TaskFunction_t handle_task_adapter = [](void *arg)
    {
        add_commands();
        SpeechRecognition::instance().handle_task();
        vTaskDelete(NULL);
    };
    ENSURE_TRUE(xTaskCreatePinnedToCore(handle_task_adapter, "Handle Task", 8 * 1024, nullptr, 1, &handle_task, 1));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Board::instance().hide_message();

    ESP_LOGI(TAG, "******* Ready! *******");
}
}