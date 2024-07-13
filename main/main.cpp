#include "board/board.h"
#include "esp_log.h"
#include "sanity_checks.h"
#include "secrets.h"
#include "speech_recognition.h"
#include <HaBridge.h>
#include <MQTTRemote.h>
#include <entities/HaEntityEvent.h>
#include "system/resource_manager.h"

#include "hal/audio_output/audio_output.h"

#include <WiFiHelper.h>
#include <nlohmann/json.hpp>

#include <memory>
#include <thread>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "system/task.h"

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

ResourceManager resource_manager;
auto event_loop = std::make_shared<EventLoop>();
auto interrupt_manager = std::make_shared<InterruptManager>(event_loop);
std::unique_ptr<Board> board;

auto audio_input = std::make_shared<AudioInput>();
auto audio_output = std::make_shared<AudioOutput>();

class SpeechRecognitionObserver : public SpeechRecognition::IObserver
{
  public:
    void on_command_not_detected() override
    {
        board->show_message(Board::TIMEOUT, "Timeout");
        audio_output->play_wav(resource_manager.not_recognized_wav);
        vTaskDelay(pdMS_TO_TICKS(1000));
        board->hide_message();
    }

    void on_waiting_for_command() override
    {
        board->show_message(Board::SAY_COMMAND, "Say command");
        audio_output->play_wav(resource_manager.wake_wav);
    }

    void on_command_handling_started(const char *message) override
    {
        board->show_message(Board::COMMAND_ACCEPTED, message);
    }

    void on_command_handling_finished() override
    {
        audio_output->play_wav(resource_manager.recognized_wav);
        vTaskDelay(pdMS_TO_TICKS(1000));
        board->hide_message();
    }
};

std::shared_ptr<SpeechRecognition> speech_recognition;

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
    interrupt_manager->initialize();

    board = std::make_unique<Board>(interrupt_manager);

    ESP_LOGI(TAG, "******* Run Event Loop *******");
    create_task(std::bind(&EventLoop::run, event_loop), "Handle Task", 4 * 1024, configMAX_PRIORITIES - 1, 0);

    board->show_message(Board::HELLO, "Hello!");

    ESP_LOGI(TAG, "******* Initialize Networking *******");
    ESP_LOGI(TAG, "Connect to WiFi");
    ENSURE_TRUE(wifi_helper.connectToAp(WIFI_SSID, WIFI_PASSWORD, true, 5 * 60 * 1000));

    ESP_LOGI(TAG, "Connect to MQTT");
    mqtt_remote.start();
    while (!mqtt_remote.connected())
        vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "******* Initialize Speech Recognition *******");
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

    std::this_thread::sleep_for(std::chrono::seconds(1));
    board->hide_message();

    ESP_LOGI(TAG, "******* Ready! *******");
}
}