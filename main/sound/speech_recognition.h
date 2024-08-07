#pragma once

#include "esp_afe_sr_iface.h"
#include "esp_mn_iface.h"

#include "system/event_loop.h"
#include "hal/audio_input.h"

#include <functional>
#include <memory>

class SpeechRecognition
{
public:
    static const AudioFormat AUDIO_FORMAT;
    static const uint32_t INPUT_CHANNEL_COUNT;
    static const uint32_t REFERENCE_CHANNEL_COUNT;

    struct IObserver
    {
        virtual void on_command_not_detected() = 0;
        virtual void on_waiting_for_command() = 0;
        virtual void on_command_handling_started(const char *message) = 0;
        virtual void on_command_handling_finished() = 0;
    };

public:
    SpeechRecognition(std::shared_ptr<EventLoop> event_loop, std::shared_ptr<IObserver> observer,
                      std::shared_ptr<AudioInput> audio_input);
    ~SpeechRecognition();

public:
    using Handler = std::function<void()>;
    void begin_add_commands();
    void add_command(std::vector<const char *> commands, Handler handler, const char *message = nullptr);
    void end_add_commands();

public:
    size_t get_feed_chunksize() const;
    void feed(const AudioData &audio);
    void audio_detect_task();

private:
    std::shared_ptr<EventLoop> m_event_loop;
    std::shared_ptr<IObserver> m_observer;
    std::shared_ptr<AudioInput> m_audio_input;

private:
    struct Command
    {
        const char *message = nullptr;
        Handler handler;
    };
    std::vector<Command> m_commands;

private:
    const esp_afe_sr_iface_t *m_afe_handle;
    esp_afe_sr_data_t *m_afe_data;
    model_iface_data_t *m_model_data;
    const esp_mn_iface_t *m_multinet;
};
