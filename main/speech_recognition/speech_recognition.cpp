#include "speech_recognition.h"

#include "sanity_checks.h"

#include "esp_afe_sr_models.h"
#include "esp_mn_models.h"
#include "esp_mn_speech_commands.h"
#include "model_path.h"

#include <cstring>

const constexpr char *TAG = "speech_recognition";

constexpr const int MULTINET_TIMEOUT_MS = 3000;

const uint32_t SpeechRecognition::INPUT_CHANNEL_COUNT = 2;
const uint32_t SpeechRecognition::REFERENCE_CHANNEL_COUNT = 1;
const AudioFormat SpeechRecognition::AUDIO_FORMAT = {
    .num_channels = INPUT_CHANNEL_COUNT + REFERENCE_CHANNEL_COUNT,
    .bits_per_sample = 16,
    .sample_rate = 16000,
};

SpeechRecognition::SpeechRecognition(std::shared_ptr<EventLoop> event_loop, std::shared_ptr<IObserver> observer,
                                     std::shared_ptr<AudioInput> audio_input)
    : m_event_loop(event_loop), m_observer(std::move(observer)), m_audio_input(audio_input)
{
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
        .wakenet_mode = DET_MODE_2CH_95,
        .afe_mode = SR_MODE_LOW_COST,
        .afe_perferred_core = 0,
        .afe_perferred_priority = 5,
        .afe_ringbuf_size = 50,
        .memory_alloc_mode = AFE_MEMORY_ALLOC_MORE_PSRAM,
        .afe_linear_gain = 1.0,
        .agc_mode = AFE_MN_PEAK_AGC_MODE_2,
        .pcm_config =
            {
                .total_ch_num = static_cast<int>(INPUT_CHANNEL_COUNT + REFERENCE_CHANNEL_COUNT),
                .mic_num = static_cast<int>(INPUT_CHANNEL_COUNT),
                .ref_num = static_cast<int>(REFERENCE_CHANNEL_COUNT),
                .sample_rate = 16000,
            },
        .debug_init = false,
        .debug_hook = {{AFE_DEBUG_HOOK_MASE_TASK_IN, NULL}, {AFE_DEBUG_HOOK_FETCH_TASK_IN, NULL}},
        .afe_ns_mode = NS_MODE_SSP,
        .afe_ns_model_name = NULL,
    };

    m_afe_handle = &ESP_AFE_SR_HANDLE;
    ESP_TRUE_CHECK(m_afe_handle);
    m_afe_data = m_afe_handle->create_from_config(&afe_config);
    ESP_TRUE_CHECK(m_afe_data);

    char *mn_name = esp_srmodel_filter(models, ESP_MN_PREFIX, ESP_MN_ENGLISH);
    ESP_LOGI(TAG, "Load multinet: %s", mn_name);
    ESP_TRUE_CHECK(strstr(mn_name, "mn7_en") != nullptr);

    m_multinet = esp_mn_handle_from_name(mn_name);
    ESP_TRUE_CHECK(m_multinet);
    m_model_data = m_multinet->create(mn_name, MULTINET_TIMEOUT_MS);
    ESP_TRUE_CHECK(m_model_data);
}

SpeechRecognition::~SpeechRecognition()
{
}

size_t SpeechRecognition::get_feed_chunksize() const
{
    return m_afe_handle->get_feed_chunksize(m_afe_data);
}

void SpeechRecognition::feed(const AudioData &audio)
{
    assert(audio.get_format() == AUDIO_FORMAT);
    m_afe_handle->feed(m_afe_data, audio.get_data_typed<int16_t>());
}

void SpeechRecognition::audio_detect_task()
{
    bool detect_flag = false;
    int afe_chunksize = m_afe_handle->get_fetch_chunksize(m_afe_data);

    int mu_chunksize = m_multinet->get_samp_chunksize(m_model_data);
    ESP_TRUE_CHECK(mu_chunksize == afe_chunksize);

    while (true)
    {
        afe_fetch_result_t *res = m_afe_handle->fetch(m_afe_data);
        if (!res || res->ret_value == ESP_FAIL)
        {
            continue;
        }

        switch (res->wakeup_state)
        {
        case WAKENET_NO_DETECT:
            break;

        case WAKENET_DETECTED:
            ESP_LOGI(TAG, "Wake word detected");
            m_event_loop->post(std::bind(&IObserver::on_waiting_for_command, m_observer));
            break;

        case WAKENET_CHANNEL_VERIFIED:
            detect_flag = true;
            m_afe_handle->disable_wakenet(m_afe_data);
            ESP_LOGI(TAG, "Channel verified: index %d", res->trigger_channel_id);
            break;
        }

        if (!detect_flag)
            continue;

        esp_mn_state_t mn_state = m_multinet->detect(m_model_data, res->data);
        switch (mn_state)
        {
        case ESP_MN_STATE_DETECTING:
            break;
        case ESP_MN_STATE_TIMEOUT: {
            ESP_LOGW(TAG, "Timeout");
            m_event_loop->post(std::bind(&IObserver::on_command_not_detected, m_observer));
            m_afe_handle->enable_wakenet(m_afe_data);
            detect_flag = false;
            break;
        }
        case ESP_MN_STATE_DETECTED: {
            esp_mn_results_t *mn_result = m_multinet->get_results(m_model_data);
            for (int i = 0; i < mn_result->num; i++)
            {
                ESP_LOGI(TAG, "TOP %d, command_id: %d, phrase_id: %d, prob: %f\n", i + 1, mn_result->command_id[i],
                         mn_result->phrase_id[i], mn_result->prob[i]);
            }

            const int command_id = mn_result->command_id[0];
            ESP_LOGI(TAG, "Deteted command : %d", command_id);
            const auto on_command_detected = [this, command_id]
            {
                ESP_TRUE_CHECK(command_id < m_commands.size());
                const auto &command = m_commands[command_id];

                ESP_LOGI(TAG, "Command: %s (%d)", command.message, command_id);
                m_observer->on_command_handling_started(command.message);
                command.handler();
                m_observer->on_command_handling_finished();
            };
            m_event_loop->post(on_command_detected);

            m_afe_handle->enable_wakenet(m_afe_data);
            detect_flag = false;
            break;
        }
        default: {
            ESP_LOGE(TAG, "Exception unhandled");
            break;
        }
        }
    }
}

void SpeechRecognition::begin_add_commands()
{
    ESP_LOGI(TAG, "Add commands");
    esp_mn_commands_alloc(m_multinet, m_model_data);
}

void SpeechRecognition::add_command(std::vector<const char *> commands, std::function<void()> handler,
                                    const char *message)
{
    Command command = {.message = message ? message : commands[0], .handler = handler};
    const int command_id = m_commands.size();
    m_commands.push_back(std::move(command));

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
