#pragma once

#include <MQTTRemote.h>
#include <HaBridge.h>
#include <entities/HaEntityEvent.h>

const constexpr std::string VOICE_COMMAND_EVENT_TYPE = "voice_command";

class MqttManager
{
  public:
    MqttManager(const std::string &device_name);
    ~MqttManager();

    std::shared_ptr<HaEntityEvent> add_event(const char *name, const char *id = nullptr);

  private:
    nlohmann::json m_json_this_device_doc;
    MQTTRemote m_mqtt_remote;
    HaBridge m_ha_bridge;
    std::vector<std::shared_ptr<HaEntityEvent>> m_ha_events;
};