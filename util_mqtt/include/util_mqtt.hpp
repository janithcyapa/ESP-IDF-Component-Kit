#pragma once

#include "mqtt_client.h"
#include "esp_event.h"
#include <functional> // Required for std::function
#include <string>     // Required for storing the subscription topic

// Define a callback type for incoming data
using MqttDataCallback = std::function<void(const char* topic, const char* data)>;

class MqttUtil {
public:
    static void init(const char* uri);
    static void publish(const char* topic, const char* data);
    static void subscribe(const char* topic, MqttDataCallback callback);

private:
    static void event_handler(void* handler_args, esp_event_base_t base, 
                             int32_t event_id, void* event_data);
    static esp_mqtt_client_handle_t s_client;
    static MqttDataCallback s_data_callback; 
    static std::string s_sub_topic; // Stores the topic to auto-subscribe on connect
};