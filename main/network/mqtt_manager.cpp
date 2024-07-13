#include "mqtt_manager.h"
#include "secrets.h"

#include <nlohmann/json.hpp>

static nlohmann::json make_device_doc_json(const std::string &device_name)
{
    nlohmann::json json;
    json["identifiers"] = device_name;
    json["name"] = "Nossat Kitchen";
    json["sw_version"] = "1.0.0";
    json["model"] = "Nossat";
    json["manufacturer"] = "Vadym";
    return json;
}

static std::string command_to_event_id(std::string input)
{
    std::transform(input.begin(), input.end(), input.begin(), ::tolower);
    std::replace(input.begin(), input.end(), ' ', '_');
    return input + "_event";
}

MqttManager::MqttManager(const std::string &device_name)
    : m_json_this_device_doc(make_device_doc_json(device_name)),
      m_mqtt_remote(device_name, MQTT_HOSTNAME, MQTT_PORT, MQTT_USERNAME, MQTT_PASSWORD),
      m_ha_bridge(m_mqtt_remote, "foo_node_id", m_json_this_device_doc)
{
    m_mqtt_remote.start();
    while (!m_mqtt_remote.connected())
        vTaskDelay(pdMS_TO_TICKS(1000));
}

MqttManager::~MqttManager()
{
}

std::shared_ptr<HaEntityEvent> MqttManager::add_event(const char *name, const char *id)
{
    const std::string id_str = id == nullptr ? command_to_event_id(name) : id;

    std::shared_ptr<HaEntityEvent> ha_event(new HaEntityEvent(m_ha_bridge, name, id_str, {VOICE_COMMAND_EVENT_TYPE}));
    ha_event->publishConfiguration();
    m_ha_events.push_back(ha_event);

    return ha_event;
}