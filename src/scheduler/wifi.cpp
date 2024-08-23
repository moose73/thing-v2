#include "wifi.h"

wifi_config_t cfg; 

typedef enum {
    DISCONNECTED,
    CONNECTED,
    CONNECTING
} WifiState;

const int NUM_APS = 2;
WiFi_AP wifi_aps[NUM_APS] = {
    {
        .ssid = "Nacho iPhone",
        .password = "password"
    },
    {
        .ssid = "Nacho iPhone2",
        .password = "password"
    }
};

WifiState wifi_state = DISCONNECTED;

void set_wifi_ap(WiFi_AP* ap) {
    wifi_config_t cfg2 = {
        .sta = {
            .listen_interval = 3,
        },
    };
    strcpy((char*) cfg2.sta.ssid, ap->ssid.c_str());
    strcpy((char*) cfg2.sta.password, ap->password.c_str());
    esp_wifi_set_config(WIFI_IF_STA, &cfg2);
    esp_wifi_connect();

}

void init_wifi() {
    // WiFi.mode(WIFI_STA);
    // WiFi.disconnect();
    esp_netif_init();
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg_init = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg_init);
    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_wifi_start();
    esp_wifi_set_inactive_time(WIFI_IF_STA, 6);
    esp_wifi_set_ps(WIFI_PS_MAX_MODEM);


    delay(100);

    add_task(reconnect_wifi, 0);
}

void format_ip_addr(char* str, uint32_t ip) {
    sprintf(str, "%d.%d.%d.%d\0", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
}

bool check_for_known_ssid() {

    uint16_t num_ap;
    esp_wifi_scan_get_ap_num(&num_ap);
    Serial.printf("Found %d APs\n", num_ap);

    wifi_ap_record_t* aps = (wifi_ap_record_t*) malloc(sizeof(wifi_ap_record_t) * num_ap);
    esp_wifi_scan_get_ap_records(&num_ap, aps);
    for (int i = 0; i < num_ap; i++) {
        Serial.printf("AP %d: %s\n", i, (char*) aps[i].ssid);
        for (int j = 0; j < NUM_APS; j++) {
            if (strcmp((char*) aps[i].ssid, wifi_aps[j].ssid.c_str()) == 0) {
                Serial.printf("Found known AP %s\n", (char*) aps[i].ssid);
                set_wifi_ap(&wifi_aps[j]);
                return true;
            }
        }
    }
    free(aps);
    Serial.printf("no known APs found\n");
    return false;

}





void http_post() {
    char local_response_buffer[2048] = {0};

    const char *post_data = "{\"field1\":\"value1\"}";
    esp_http_client_config_t config = {
        .host = "httpbin.org",
        .path = "/get",
        .query = "esp",
        .user_data = local_response_buffer,        // Pass address of local buffer to get response
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    esp_http_client_set_url(client, "http://httpbin.org/post");
    esp_http_client_set_method(client, HTTP_METHOD_POST);
    esp_http_client_set_header(client, "Content-Type", "application/json");
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    esp_http_client_perform(client);
    //print response
    Serial.printf("status code: %d\n", esp_http_client_get_status_code(client));

    esp_http_client_cleanup(client);
}


uint32_t reconnect_wifi() {


    if (wifi_state == CONNECTING) {
        bool success = check_for_known_ssid();
        if (success) {
            wifi_state = CONNECTED; //not quite, but skips the loops. Disconnected turns wifi on (which we don't want to do). Connecting comes back here.
            Serial.printf("found wifi. coming back in 10s to verify ?.\n");
            return 15 * 1000 * 1000; // give 10 seconds to connect before going back into scan loop
        } else {
            Serial.printf("didn't find wifi. Turning wifi off.\n");
            wifi_state = DISCONNECTED;
            esp_wifi_stop();
            return 15 * 1000 * 1000; // give 15 seconds before trying again
        }
    }

    if (wifi_state == DISCONNECTED) {
        esp_wifi_start();
    }

    wifi_ap_record_t ap_info;
    esp_err_t rtn = esp_wifi_sta_get_ap_info(&ap_info);
    if (rtn != ESP_OK) {
        wifi_state = CONNECTING;
        Serial.printf("starting to scan\n");
        esp_wifi_scan_start(NULL, false);
        return 2 * 1000 * 1000; // scan takes 1.5 seconds. Come back after 2s and go into the check_for_known_ssid() function
        
    } else {

        wifi_state = CONNECTED;

        // esp_netif_ip_info_t ip;
        // tcpip_adapter_if_t if_t;
        // esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        // esp_netif_get_ip_info(netif, &ip);
        // tcpip_adapter_ip_info_t ipInfo;
        // rtn = tcpip_adapter_get_ip_info(TCPIP_ADAPTER_IF_TEST, &ipInfo);
        // if (rtn != ESP_OK) {
        //     Serial.printf("Failed to get IP info\n");
        // } 
        //     char ip_str[20];
        //     format_ip_addr(ip_str, ipInfo.ip.addr);
        //     Serial.printf("IP address: %s. Raw: %x\n", ip_str, ipInfo.ip.addr);
        
        // Serial.printf("ssid %s\n", (char*) &(ap_info.ssid));
        
    }
    // if (esp_wifi) {
    //     WiFi.begin(WIFI_SSID, WIFI_PWD);
    //     Serial.print("Attempting connection to WiFi ..\n");
    // } else {
    //     Serial.printf("Wifi connected: IP address: %s\n", WiFi.localIP().toString().c_str());
    // }

    return 5 * 1000 * 1000;

}

uint8_t get_rssi_bars() {
    // if (WiFi.status() != WL_CONNECTED) {
    //     return 0;
    // } else {
    //     int rssi = WiFi.RSSI();
    //     Serial.printf("RSSI is %d\n");
    // }
    return 0;
}