#include <file_system.h>
#include "bsp/esp-bsp.h"
#include "esp_log.h"
#include <sys/stat.h>

const constexpr char *TAG = "file_system";

FileSystem::FileSystem()
{
    ESP_LOGI(TAG, "Initialize SPIFFS");
    ESP_ERROR_CHECK(bsp_spiffs_mount());
}

FileSystem::~FileSystem()
{
    ESP_LOGI(TAG, "Deinitialize SPIFFS");
    ESP_ERROR_CHECK(bsp_spiffs_umount());
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