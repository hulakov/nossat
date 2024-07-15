#pragma once

#include "hal/file_system/file_system.h"
#include "sanity_checks.h"

#include <vector>

struct ResourceManager
{
    const char *WAKE_WAV_PATH = "/spiffs/echo_en_wake.wav";
    const char *RECOGNIZED_WAV_PATH = "/spiffs/echo_en_recognized.wav";
    const char *NOT_RECOGNIZED_WAV_PATH = "/spiffs/echo_en_not_recognized.wav";

    ResourceManager()
    {
        FileSystem file_system;
        ESP_TRUE_CHECK(file_system.load_file(WAKE_WAV_PATH, wake_wav));
        ESP_TRUE_CHECK(file_system.load_file(RECOGNIZED_WAV_PATH, recognized_wav));
        ESP_TRUE_CHECK(file_system.load_file(NOT_RECOGNIZED_WAV_PATH, not_recognized_wav));
    }

    std::vector<uint8_t> wake_wav;
    std::vector<uint8_t> recognized_wav;
    std::vector<uint8_t> not_recognized_wav;
};