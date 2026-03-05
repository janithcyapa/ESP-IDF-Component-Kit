#pragma once

#include "esp_err.h"
#include "esp_event.h" 

class WifiUtil {
public:
    static void init_wifi(); // Your log shows you named it init_wifi

private:
    // Ensure the signature matches exactly: void*, esp_event_base_t, int32_t, void*
    static void event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data);
    static int s_retry_num;
};