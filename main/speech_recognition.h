#pragma once

#include <functional>
#include <memory>

class SpeechRecognition
{
  private:
    SpeechRecognition();
    ~SpeechRecognition();

  public:
    static SpeechRecognition &instance();

    void initialize();

    void begin_add_commands();
    void add_command(std::vector<const char *> commands, std::function<void()> handler, const char *message = nullptr);
    void end_add_commands();

    void handle_task();
    void audio_feed_task();
    void audio_detect_task();

  private:
    struct Data;

    std::unique_ptr<Data> m_data;
};
