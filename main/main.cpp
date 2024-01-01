#include "board/board.h"
#include "esp_log.h"
#include "speech_recognition.h"
#include <thread>
extern "C"
{
#include "app_led.h"
}

#include "sanity_checks.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "main";

void add_commands()
{
    SpeechRecognition::instance().begin_add_commands();

    const auto turn_on_the_light = []() { app_pwm_led_set_power(1); };
    SpeechRecognition::instance().add_command({"Turn On the Light", "Switch On the Light"}, turn_on_the_light);
    const auto turn_off_the_light = []() { app_pwm_led_set_power(0); };
    SpeechRecognition::instance().add_command({"Turn Off the Light", "Switch Off the Light"}, turn_off_the_light);
    const auto turn_red = []() { app_pwm_led_set_all(128, 0, 0); };
    SpeechRecognition::instance().add_command({"Turn Red"}, turn_red);
    const auto turn_green = []() { app_pwm_led_set_all(0, 128, 0); };
    SpeechRecognition::instance().add_command({"Turn Green"}, turn_green);
    const auto turn_blue = []() { app_pwm_led_set_all(0, 0, 128); };
    SpeechRecognition::instance().add_command({"Turn Blue"}, turn_blue);
    const auto turn_pink = []() { app_pwm_led_set_all(199, 21, 133); };
    SpeechRecognition::instance().add_command({"Turn Pink"}, turn_pink);

    SpeechRecognition::instance().end_add_commands();
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
        SpeechRecognition::instance().initialize();
        add_commands();

        ESP_LOGI(TAG, "Initialize LED");
        app_pwm_led_init(GPIO_NUM_39, GPIO_NUM_40, GPIO_NUM_41);

        TaskHandle_t feed_task;
        TaskHandle_t detect_task;

        ESP_LOGI(TAG, "Start tasks");
        TaskFunction_t audio_feed_task_adapter = [](void *) {
            SpeechRecognition::instance().audio_feed_task();
            vTaskDelete(NULL);
        };
        ENSURE_TRUE(xTaskCreatePinnedToCore(audio_feed_task_adapter, "Feed Task", 8 * 1024, nullptr, 5, &feed_task, 0));

        TaskFunction_t audio_detect_task_adapter = [](void *arg) {
            SpeechRecognition::instance().audio_detect_task();
            vTaskDelete(NULL);
        };
        ENSURE_TRUE(xTaskCreatePinnedToCore(audio_detect_task_adapter, "Speech Recognition Task", 8 * 1024, nullptr, 5,
                                            &detect_task, 1));

        std::this_thread::sleep_for(std::chrono::seconds(2));
        Board::instance().hide_message();

        ESP_LOGI(TAG, "Ready!");

        SpeechRecognition::instance().handle_task();
    }
}