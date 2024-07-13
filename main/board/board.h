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
    bool play_audio(const std::vector<uint8_t>::const_iterator &buffer_begin,
                    const std::vector<uint8_t>::const_iterator &buffer_end, uint32_t sample_rate,
                    uint32_t bits_per_sample, size_t channels);

    enum MessageType { HELLO, SAY_COMMAND, TIMEOUT, COMMAND_ACCEPTED };

    bool show_message(MessageType type, const char *message = nullptr);
    bool hide_message();

  private:
    struct Data;
    std::unique_ptr<Data> m_data;
};