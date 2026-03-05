#include <stdio.h>
#include "util_wifi.h"

static const char *TAG = "WIFI";


void app_main(void) {

    // Configure Wifi using menuconfig 

    // Component config -> Wi-Fi Configuration
    // Wi-Fi SSID
    // Wi-Fi Password
    // Maximum Retry
    WifiUtil::init_wifi();

}