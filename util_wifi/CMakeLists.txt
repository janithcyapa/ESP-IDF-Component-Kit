idf_component_register(SRCS "util_wifi.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_wifi esp_event esp_netif nvs_flash)