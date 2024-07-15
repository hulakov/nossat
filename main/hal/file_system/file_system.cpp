#include "file_system.h"

#include "esp_log.h"
#include "esp_spiffs.h"
#include <sys/stat.h>

const constexpr char *TAG = "file_system";
const constexpr char *SPIFFS_MOUNT_POINT = "/spiffs";
const constexpr char *SPIFFS_PARTITION_LABEL = "storage";
const constexpr int SPIFFS_MAX_FILES = 5;

esp_err_t bsp_spiffs_mount()
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = SPIFFS_MOUNT_POINT,
        .partition_label = SPIFFS_PARTITION_LABEL,
        .max_files = SPIFFS_MAX_FILES,
        .format_if_mount_failed = false,
    };

    esp_err_t ret_val = esp_vfs_spiffs_register(&conf);

    ESP_ERROR_CHECK(ret_val);

    size_t total = 0, used = 0;
    ret_val = esp_spiffs_info(conf.partition_label, &total, &used);
    if (ret_val != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret_val));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }

    return ret_val;
}

esp_err_t bsp_spiffs_unmount()
{
    return esp_vfs_spiffs_unregister(SPIFFS_PARTITION_LABEL);
}

FileSystem::FileSystem()
{
    ESP_LOGI(TAG, "Initialize SPIFFS");
    ESP_ERROR_CHECK(bsp_spiffs_mount());
}

FileSystem::~FileSystem()
{
    ESP_LOGI(TAG, "Deinitialize SPIFFS");
    ESP_ERROR_CHECK(bsp_spiffs_unmount());
}

bool FileSystem::load_file(const char *path, std::vector<uint8_t> &buffer)
{
    auto fp = fopen(path, "rb");
    if (fp == nullptr)
        return false;

    const size_t file_size = get_file_size(path);
    buffer.resize(file_size);
    fread(&*buffer.begin(), 1, buffer.size(), fp);
    fclose(fp);

    return true;
}

size_t FileSystem::get_file_size(const char *path)
{
    struct stat siz = {};
    stat(path, &siz);
    return siz.st_size;
}