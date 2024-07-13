#include "board/board.h"
#include "esp_log.h"
#include "sanity_checks.h"
#include "secrets.h"
#include "speech_recognition.h"
#include "encoder.h"
#include "file_system.h"
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

auto event_loop = std::make_shared<EventLoop>();
auto interrupt_manager = std::make_shared<InterruptManager>(event_loop);

auto audio_input = std::make_shared<AudioInput>();

typedef struct
{
    // The "RIFF" chunk descriptor
    uint8_t ChunkID[4];
    int32_t ChunkSize;
    uint8_t Format[4];
    // The "fmt" sub-chunk
    uint8_t Subchunk1ID[4];
    int32_t Subchunk1Size;
    int16_t AudioFormat;
    int16_t NumChannels;
    int32_t SampleRate;
    int32_t ByteRate;
    int16_t BlockAlign;
    int16_t BitsPerSample;
    // The "data" sub-chunk
    uint8_t Subchunk2ID[4];
    int32_t Subchunk2Size;
} wav_header_t;

bool play_wav(const std::vector<uint8_t> &buffer)
{

    auto wav_head = reinterpret_cast<const wav_header_t *>(&buffer[0]);
    ESP_LOGI(TAG, "Play WAV: sample_rate=%d, channels=%d, bits_per_sample=%d", (int)wav_head->SampleRate,
             (int)wav_head->NumChannels, (int)wav_head->BitsPerSample);

    return Board::instance().play_audio(buffer.begin() + sizeof(wav_header_t), buffer.end(), wav_head->SampleRate,
                                        wav_head->BitsPerSample, wav_head->NumChannels);
}

class SpeechRecognitionObserver : public SpeechRecognition::IObserver
{
  public:
    const char *WAKE_WAV_PATH = "/spiffs/echo_en_wake.wav";
    const char *RECOGNIZED_WAV_PATH = "/spiffs/echo_en_recognized.wav";
    const char *NOT_RECOGNIZED_WAV_PATH = "/spiffs/echo_en_not_recognized.wav";

    SpeechRecognitionObserver()
    {
        FileSystem file_system;
        ESP_LOGI(TAG, "Load WAVs");
        ESP_TRUE_CHECK(file_system.load_file(WAKE_WAV_PATH, m_wake_wav));
        ESP_TRUE_CHECK(file_system.load_file(RECOGNIZED_WAV_PATH, m_recognized_wav));
        ESP_TRUE_CHECK(file_system.load_file(NOT_RECOGNIZED_WAV_PATH, m_not_recognized_wav));
    }

    void on_command_not_detected() override
    {
        Board::instance().show_message(Board::TIMEOUT, "Timeout");
        play_wav(m_not_recognized_wav);
        vTaskDelay(pdMS_TO_TICKS(1000));
        Board::instance().hide_message();
    }

    void on_waiting_for_command() override
    {
        Board::instance().show_message(Board::SAY_COMMAND, "Say command");
        play_wav(m_wake_wav);
    }

    void on_command_handling_started(const char *message) override
    {
        Board::instance().show_message(Board::COMMAND_ACCEPTED, message);
    }

    void on_command_handling_finished() override
    {
        play_wav(m_recognized_wav);
        vTaskDelay(pdMS_TO_TICKS(1000));
        Board::instance().hide_message();
    }

  private:
    std::vector<uint8_t> m_wake_wav;
    std::vector<uint8_t> m_recognized_wav;
    std::vector<uint8_t> m_not_recognized_wav;
};

std::unique_ptr<SpeechRecognition> speech_recognition;

auto encoder1 = std::make_shared<Encoder>(GPIO_NUM_14, GPIO_NUM_13, GPIO_NUM_9);
auto encoder2 = std::make_shared<Encoder>(GPIO_NUM_17, GPIO_NUM_18, GPIO_NUM_8);

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

void set_lvgl_value(int32_t v);

void app_main(void)
{
    ESP_LOGI(TAG, "Nosyna Satelite is starting!");
    ESP_LOGI(TAG, "Compile time: %s %s", __DATE__, __TIME__);

    ESP_LOGI(TAG, "******* Initialize Board *******");
    interrupt_manager->initialize();
    Board::instance().initialize();
    Board::instance().show_message(Board::HELLO, "Hello!");

    ESP_LOGI(TAG, "******* Initialize Networking *******");
    ESP_LOGI(TAG, "Connect to WiFi");
    ENSURE_TRUE(wifi_helper.connectToAp(WIFI_SSID, WIFI_PASSWORD));

    ESP_LOGI(TAG, "Connect to MQTT");
    mqtt_remote.start();
    while (!mqtt_remote.connected())
        vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "******* Initialize Speech Recognition *******");
    speech_recognition =
        std::make_unique<SpeechRecognition>(event_loop, std::make_unique<SpeechRecognitionObserver>(), audio_input);

    ESP_LOGI(TAG, "******* Start tasks *******");
    TaskHandle_t feed_task;
    TaskFunction_t audio_feed_task_adapter = [](void *)
    {
        speech_recognition->audio_feed_task();
        vTaskDelete(NULL);
    };
    ENSURE_TRUE(xTaskCreatePinnedToCore(audio_feed_task_adapter, "Feed Task", 4 * 1024, nullptr, 5, &feed_task, 0));

    TaskHandle_t detect_task;
    TaskFunction_t audio_detect_task_adapter = [](void *arg)
    {
        speech_recognition->audio_detect_task();
        vTaskDelete(NULL);
    };
    ENSURE_TRUE(xTaskCreatePinnedToCore(audio_detect_task_adapter, "Speech Recognition Task", 4 * 1024, nullptr, 5,
                                        &detect_task, 1));

    TaskHandle_t handle_task;
    TaskFunction_t handle_task_adapter = [](void *arg)
    {
        ESP_LOGI(TAG, "******* Add Speech Recognition Commands *******");
        add_commands();

        ESP_LOGI(TAG, "******* Run Event Loop *******");
        event_loop->run();

        vTaskDelete(NULL);
    };
    ENSURE_TRUE(xTaskCreatePinnedToCore(handle_task_adapter, "Handle Task", 8 * 1024, nullptr, configMAX_PRIORITIES - 1,
                                        &handle_task, 0));

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Board::instance().hide_message();

    encoder1->set_step_value(4);
    encoder1->set_value_changed_handler(
        [](int previous_value, int new_value)
        {
            ESP_LOGI(TAG, "[left] changed %d -> %d", previous_value, new_value);
            set_lvgl_value(new_value);
        });
    encoder1->set_click_handler([]() { ESP_LOGI(TAG, "[left] click"); });
    encoder1->initialize(interrupt_manager);

    encoder2->set_step_value(4);
    encoder2->set_value_changed_handler([](int previous_value, int new_value)
                                        { ESP_LOGI(TAG, "[right] changed %d -> %d", previous_value, new_value); });
    encoder2->set_click_handler([]() { ESP_LOGI(TAG, "[right] click"); });
    encoder2->initialize(interrupt_manager);

    ESP_LOGI(TAG, "******* Ready! *******");
}
}