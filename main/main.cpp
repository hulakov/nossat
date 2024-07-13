#include "board/board.h"
#include "esp_log.h"

static const char *TAG = "main";
extern "C"
{

void app_main(void)
{
    ESP_LOGI(TAG, "Nosyna Satelite is starting!");
    ESP_LOGI(TAG, "Compile time: %s %s", __DATE__, __TIME__);

    start();
}
}