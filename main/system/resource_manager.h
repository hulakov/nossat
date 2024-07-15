#pragma once

#include "hal/file_system/file_system.h"
#include "sanity_checks.h"
#include "utils/audio_data/audio_data.h"

#include <vector>

struct ResourceManager
{
    const char *WAKE_WAV_PATH = "/spiffs/echo_en_wake.wav";
    const char *RECOGNIZED_WAV_PATH = "/spiffs/echo_en_recognized.wav";
    const char *NOT_RECOGNIZED_WAV_PATH = "/spiffs/echo_en_not_recognized.wav";

    ResourceManager()
    {
        FileSystem file_system;
        std::vector<uint8_t> buffer;

        const auto load_resource_wav = [&buffer, &file_system](const char *name)
        {
            ESP_TRUE_CHECK(file_system.load_file(name, buffer));
            return AudioData::load_wav(buffer);
        };

        wake_wav = load_resource_wav(WAKE_WAV_PATH);
        recognized_wav = load_resource_wav(RECOGNIZED_WAV_PATH);
        not_recognized_wav = load_resource_wav(NOT_RECOGNIZED_WAV_PATH);
    }

    AudioData wake_wav;
    AudioData recognized_wav;
    AudioData not_recognized_wav;
};