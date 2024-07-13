#include "audio_output.h"
#include "esp_log.h"

static const char *TAG = "audio_output";

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

bool AudioOutput::play_wav(const std::vector<uint8_t> &buffer)
{
    auto wav_head = reinterpret_cast<const wav_header_t *>(&buffer[0]);
    ESP_LOGI(TAG, "Play WAV: sample_rate=%d, channels=%d, bits_per_sample=%d", (int)wav_head->SampleRate,
             (int)wav_head->NumChannels, (int)wav_head->BitsPerSample);

    return play_audio(buffer.begin() + sizeof(wav_header_t), buffer.end(), wav_head->SampleRate,
                      wav_head->BitsPerSample, wav_head->NumChannels);
}