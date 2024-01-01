#include "speech_recognition.h"
#include "board/board.h"
#include "sanity_checks.h"

extern "C"
{
#include "esp_log.h"

#include "esp_afe_sr_iface.h"
#include "esp_afe_sr_models.h"
#include "esp_mn_iface.h"
#include "esp_mn_models.h"
#include "esp_mn_speech_commands.h"
#include "model_path.h"
}

#include <string.h>
#include <sys/stat.h>
#include <unordered_map>
#include <vector>

const constexpr char *TAG = "speech_recognition";

constexpr const int MULTINET_TIMEOUT_MS = 3000;

const constexpr char *WAKE_WAV_PATH = "/spiffs/echo_en_wake.wav";
const constexpr char *RECOGNIZED_WAV_PATH = "/spiffs/echo_en_recognized.wav";
const constexpr char *NOT_RECOGNIZED_WAV_PATH = "/spiffs/echo_en_not_recognized.wav";

enum class State
{
    NONE,
    COMMAND_NOT_DETECTED,
    WAITING_FOR_COMMAND,
    COMMAND_DETECTED,
};

struct Command
{
    const char *message = nullptr;
    std::function<void()> handler;
};

struct Result
{
    State state = State::NONE;
    int command_id = 0;
};

struct SpeechRecognition::Data
{
    const esp_afe_sr_iface_t *afe_handle;
    esp_afe_sr_data_t *afe_data;
    model_iface_data_t *model_data;
    const esp_mn_iface_t *multinet;
    QueueHandle_t result_que;

    std::vector<uint8_t> wake_wav;
    std::vector<uint8_t> recognized_wav;
    std::vector<uint8_t> not_recognized_wav;

    std::vector<Command> commands;
};

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

bool play_wav(const std::vector<uint8_t> &buffer)
{

    auto wav_head = reinterpret_cast<const wav_header_t *>(&buffer[0]);
    ESP_LOGI(TAG, "Play WAV: sample_rate=%d, channels=%d, bits_per_sample=%d", (int)wav_head->SampleRate,
             (int)wav_head->NumChannels, (int)wav_head->BitsPerSample);

    return Board::instance().play_audio(buffer.begin() + sizeof(wav_header_t), buffer.end(), wav_head->SampleRate,
                                        wav_head->BitsPerSample, wav_head->NumChannels);
}

size_t get_file_size(const char *path)
{
    struct stat siz = {};
    stat(path, &siz);
    return siz.st_size;
}

bool load_wav_file(const char *path, std::vector<uint8_t> &buffer)
{
    auto fp = fopen(path, "rb");
    if (fp == nullptr)
    {
        return false;
    }

    const size_t file_size = get_file_size(path);
    buffer.resize(file_size);
    fread(&*buffer.begin(), 1, buffer.size(), fp);
    fclose(fp);

    return true;
}

SpeechRecognition::SpeechRecognition()
{
}

SpeechRecognition::~SpeechRecognition()
{
}

SpeechRecognition &SpeechRecognition::instance()
{
    static SpeechRecognition sr;
    return sr;
}

void SpeechRecognition::audio_feed_task()
{
    int audio_chunksize = m_data->afe_handle->get_feed_chunksize(m_data->afe_data);
    const int CHANNEL_COUNT = Board::MICROPHONE_CHANNEL_COUNT + Board::REFERENCE_CHANNEL_COUNT;
    ESP_LOGI(TAG, "audio_chunksize=%d, feed_channel=%d", audio_chunksize, CHANNEL_COUNT);

    std::vector<int16_t> audio_buffer(audio_chunksize * CHANNEL_COUNT);

    while (true)
    {
        if (Board::instance().capture_audio(audio_buffer, audio_chunksize))
        {
            m_data->afe_handle->feed(m_data->afe_data, &audio_buffer[0]);
        }
    }
}

void SpeechRecognition::audio_detect_task()
{
    bool detect_flag = false;
    int afe_chunksize = m_data->afe_handle->get_fetch_chunksize(m_data->afe_data);

    int mu_chunksize = m_data->multinet->get_samp_chunksize(m_data->model_data);
    ENSURE_TRUE(mu_chunksize == afe_chunksize);

    while (true)
    {
        afe_fetch_result_t *res = m_data->afe_handle->fetch(m_data->afe_data);
        if (!res || res->ret_value == ESP_FAIL)
        {
            continue;
        }

        if (res->wakeup_state == WAKENET_DETECTED)
        {
            ESP_LOGI(TAG, "Wake word detected");
            Result result = {.state = State::WAITING_FOR_COMMAND};
            xQueueSend(m_data->result_que, &result, 0);
        }
        else if (res->wakeup_state == WAKENET_CHANNEL_VERIFIED)
        {
            detect_flag = true;
            m_data->afe_handle->disable_wakenet(m_data->afe_data);
            ESP_LOGI(TAG, "Channel verified: index %d", res->trigger_channel_id);
        }

        if (detect_flag)
        {
            esp_mn_state_t mn_state = m_data->multinet->detect(m_data->model_data, res->data);

            if (ESP_MN_STATE_DETECTING == mn_state)
            {
                continue;
            }

            if (ESP_MN_STATE_TIMEOUT == mn_state)
            {
                ESP_LOGW(TAG, "Timeout");
                Result result = {.state = State::COMMAND_NOT_DETECTED};
                xQueueSend(m_data->result_que, &result, 0);
                m_data->afe_handle->enable_wakenet(m_data->afe_data);
                detect_flag = false;
                continue;
            }

            if (ESP_MN_STATE_DETECTED == mn_state)
            {
                esp_mn_results_t *mn_result = m_data->multinet->get_results(m_data->model_data);
                for (int i = 0; i < mn_result->num; i++)
                {
                    ESP_LOGI(TAG, "TOP %d, command_id: %d, phrase_id: %d, prob: %f\n", i + 1, mn_result->command_id[i],
                             mn_result->phrase_id[i], mn_result->prob[i]);
                }

                const int command_id = mn_result->command_id[0];
                ESP_LOGI(TAG, "Deteted command : %d", command_id);
                Result result = {
                    .state = State::COMMAND_DETECTED,
                    .command_id = command_id,
                };
                xQueueSend(m_data->result_que, &result, 0);

                m_data->afe_handle->enable_wakenet(m_data->afe_data);
                detect_flag = false;

                continue;
            }
            ESP_LOGE(TAG, "Exception unhandled");
        }
    }
}

void SpeechRecognition::handle_task()
{
    while (true)
    {
        Result result;
        xQueueReceive(m_data->result_que, &result, portMAX_DELAY);

        switch (result.state)
        {
        case State::COMMAND_NOT_DETECTED: {
            Board::instance().show_message(Board::TIMEOUT, "Timeout");
            play_wav(m_data->not_recognized_wav);
            vTaskDelay(pdMS_TO_TICKS(2000));
            Board::instance().hide_message();
            break;
        }
        case State::WAITING_FOR_COMMAND: {
            Board::instance().show_message(Board::SAY_COMMAND, "Say command");
            play_wav(m_data->wake_wav);
            break;
        }
        case State::COMMAND_DETECTED: {
            ENSURE_TRUE(result.command_id < m_data->commands.size());
            const auto &command = m_data->commands[result.command_id];

            ESP_LOGI(TAG, "Command: %s (%d)", command.message, result.command_id);
            Board::instance().show_message(Board::COMMAND_ACCEPTED, command.message);
            command.handler();
            play_wav(m_data->recognized_wav);
            vTaskDelay(pdMS_TO_TICKS(2000));
            Board::instance().hide_message();
            break;
        }
        case State::NONE: {
            break;
        }
        }
    }
}

void SpeechRecognition::initialize()
{
    ESP_LOGI(TAG, "******* Initialize Speech Recognition *******");
    ENSURE_TRUE(!m_data);

    ESP_LOGI(TAG, "Initialize result queue");
    QueueHandle_t result_que = xQueueCreate(3, sizeof(Result));
    ENSURE_TRUE(result_que);

    ESP_LOGI(TAG, "Load models");
    srmodel_list_t *models = esp_srmodel_init("model");

    char *wm_name = esp_srmodel_filter(models, ESP_WN_PREFIX, NULL);
    ESP_LOGI(TAG, "Load wakenet: %s", wm_name);

    // copied from AFE_CONFIG_DEFAULT
    afe_config_t afe_config = {
        .aec_init = true,
        .se_init = true,
        .vad_init = true,
        .wakenet_init = true,
        .voice_communication_init = false,
        .voice_communication_agc_init = false,
        .voice_communication_agc_gain = 15,
        .vad_mode = VAD_MODE_3,
        .wakenet_model_name = wm_name,
        .wakenet_model_name_2 = NULL,
        .wakenet_mode = DET_MODE_2CH_90,
        .afe_mode = SR_MODE_LOW_COST,
        .afe_perferred_core = 0,
        .afe_perferred_priority = 5,
        .afe_ringbuf_size = 50,
        .memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM,
        .afe_linear_gain = 1.0,
        .agc_mode = AFE_MN_PEAK_AGC_MODE_2,
        .pcm_config =
            {
                .total_ch_num = Board::MICROPHONE_CHANNEL_COUNT + Board::REFERENCE_CHANNEL_COUNT,
                .mic_num = Board::MICROPHONE_CHANNEL_COUNT,
                .ref_num = Board::REFERENCE_CHANNEL_COUNT,
                .sample_rate = 16000,
            },
        .debug_init = false,
        .debug_hook = {{AFE_DEBUG_HOOK_MASE_TASK_IN, NULL}, {AFE_DEBUG_HOOK_FETCH_TASK_IN, NULL}},
        .afe_ns_mode = NS_MODE_SSP,
        .afe_ns_model_name = NULL,
    };

    const esp_afe_sr_iface_t *afe_handle = &ESP_AFE_SR_HANDLE;
    esp_afe_sr_data_t *afe_data = afe_handle->create_from_config(&afe_config);
    ENSURE_TRUE(afe_data);

    char *mn_name = esp_srmodel_filter(models, ESP_MN_PREFIX, ESP_MN_ENGLISH);
    ESP_LOGI(TAG, "Load multinet: %s", mn_name);
    ENSURE_TRUE(strstr(mn_name, "mn7_en") != nullptr);

    esp_mn_iface_t *multinet = esp_mn_handle_from_name(mn_name);
    model_iface_data_t *model_data = multinet->create(mn_name, MULTINET_TIMEOUT_MS);
    ENSURE_TRUE(model_data);

    ESP_LOGI(TAG, "Load WAVs");
    std::vector<uint8_t> wake_wav;
    ENSURE_TRUE(load_wav_file(WAKE_WAV_PATH, wake_wav));
    std::vector<uint8_t> recognized_wav;
    ENSURE_TRUE(load_wav_file(RECOGNIZED_WAV_PATH, recognized_wav));
    std::vector<uint8_t> not_recognized_wav;
    ENSURE_TRUE(load_wav_file(NOT_RECOGNIZED_WAV_PATH, not_recognized_wav));

    m_data = std::unique_ptr<Data>(new Data{
        .afe_handle = afe_handle,
        .afe_data = afe_data,
        .model_data = model_data,
        .multinet = multinet,
        .result_que = result_que,

        .wake_wav = std::move(wake_wav),
        .recognized_wav = std::move(recognized_wav),
        .not_recognized_wav = std::move(not_recognized_wav),

        .commands = {},
    });
}

void SpeechRecognition::begin_add_commands()
{
    ESP_LOGI(TAG, "Add commands");
    esp_mn_commands_alloc(m_data->multinet, m_data->model_data);
}

void SpeechRecognition::add_command(std::vector<const char *> commands, std::function<void()> handler,
                                    const char *message)
{
    Command command = {.message = message ? message : commands[0], .handler = handler};
    const int command_id = m_data->commands.size();
    m_data->commands.push_back(std::move(command));

    for (const auto &command : commands)
    {
        esp_mn_commands_add(command_id, const_cast<char *>(command));
    }
}

void SpeechRecognition::end_add_commands()
{
    esp_mn_commands_print();
    esp_mn_commands_update();
}
