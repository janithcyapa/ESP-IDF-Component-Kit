#include "util_mqtt.hpp"
#include "esp_log.h"

static const char *TAG = "MQTT_UTIL";
esp_mqtt_client_handle_t MqttUtil::s_client = nullptr;
MqttDataCallback MqttUtil::s_data_callback = nullptr;
std::string MqttUtil::s_sub_topic = ""; // Initialize empty

void MqttUtil::event_handler(void* handler_args, esp_event_base_t base, 
                            int32_t event_id, void* event_data) {
    auto event = static_cast<esp_mqtt_event_handle_t>(event_data);
    
    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            
            // Auto-subscribe once connection is established (or re-established)
            if (!s_sub_topic.empty()) {
                esp_mqtt_client_subscribe(s_client, s_sub_topic.c_str(), 1);
                ESP_LOGI(TAG, "Subscribed to topic: %s", s_sub_topic.c_str());
            }
            break;

        case MQTT_EVENT_DATA: {
            // Convert buffers to null-terminated strings safely
            char topic[64] = {0};
            char data[128] = {0};
            snprintf(topic, sizeof(topic), "%.*s", event->topic_len, event->topic);
            snprintf(data, sizeof(data), "%.*s", event->data_len, event->data);

            if (s_data_callback) {
                s_data_callback(topic, data); // Notify main app
            }
            break;
        }
        default:
            break;
    }
}

void MqttUtil::subscribe(const char* topic, MqttDataCallback callback) {
    // Store topic and callback for the event loop to use
    s_sub_topic = topic;
    s_data_callback = callback;
    
    // If the client is already running, try subscribing immediately too
    if (s_client) {
        esp_mqtt_client_subscribe(s_client, s_sub_topic.c_str(), 1);
    }
}

void MqttUtil::init(const char* uri) {
    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri = uri;
    mqtt_cfg.broker.address.port = 1883;

    s_client = esp_mqtt_client_init(&mqtt_cfg);
    
    // Correct macro for MQTT events is MQTT_EVENT_ANY
    esp_mqtt_client_register_event(s_client, MQTT_EVENT_ANY, 
                                   MqttUtil::event_handler, NULL);
    esp_mqtt_client_start(s_client);
}

void MqttUtil::publish(const char* topic, const char* data) {
    if (s_client) {
        esp_mqtt_client_publish(s_client, topic, data, 0, 1, 0);
    }
}