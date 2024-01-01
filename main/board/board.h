#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

class Board final
{
  private:
    Board();
    ~Board();

  public:
    static Board &instance();
    void initialize();
    bool capture_audio(std::vector<int16_t> &buffer, size_t chunk_size);
    bool play_audio(const std::vector<uint8_t>::const_iterator &buffer_begin,
                    const std::vector<uint8_t>::const_iterator &buffer_end, uint32_t sample_rate,
                    uint32_t bits_per_sample, size_t channels);

    enum MessageType
    {
        HELLO,
        SAY_COMMAND,
        TIMEOUT,
        COMMAND_ACCEPTED
    };

    bool show_message(MessageType type, const char *message = nullptr);
    bool hide_message();

    static const int MICROPHONE_CHANNEL_COUNT;
    static const int REFERENCE_CHANNEL_COUNT;

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
};