#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include <string.h>
#include "fin-lib.h"

namespace fin {
    static const char * WIFI_TAG = "WIFI";
    static char fin_wifi_ip_str[32] = {};

    static void wifi_event_handler( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data ) {
        if ( event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START ) {
            ESP_LOGI( WIFI_TAG, "Wifi started!" );
            esp_wifi_connect();
        }
        else if ( event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED ) {
            ESP_LOGI( WIFI_TAG, "Wifi Disconnected" );
            esp_wifi_connect();
            memset( fin_wifi_ip_str, 0, sizeof( fin_wifi_ip_str ) );
        }
        else if ( event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP ) {
            ip_event_got_ip_t * event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI( WIFI_TAG, "Wifi Connected" );
            ESP_LOGI( WIFI_TAG, "Got ip:" IPSTR, IP2STR( &event->ip_info.ip ) );
            esp_ip4addr_ntoa( &event->ip_info.ip, fin_wifi_ip_str, sizeof( fin_wifi_ip_str ) );
        }
        else if ( event_base == IP_EVENT && event_id == IP_EVENT_STA_LOST_IP ) {
            ESP_LOGI( WIFI_TAG, "Wifi Lost IP" );
            memset( fin_wifi_ip_str, 0, sizeof( fin_wifi_ip_str ) );
        }
    }

    void wifi_init() {
        char hostname[32] = {};
        sprintf( hostname, "Eranest-FinMarket" );
        ESP_LOGI( WIFI_TAG, "Hostname: %s", hostname );

        ESP_ERROR_CHECK( esp_netif_init() );
        ESP_ERROR_CHECK( esp_event_loop_create_default() );
        esp_netif_t * netif = esp_netif_create_default_wifi_sta();
        esp_netif_set_hostname( netif, hostname );

        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        ESP_ERROR_CHECK( esp_wifi_init( &cfg ) );

        esp_event_handler_instance_t instance_any_id;
        esp_event_handler_instance_t instance_got_ip;

        ESP_ERROR_CHECK( esp_event_handler_instance_register( WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL,
            &instance_any_id ) );

        ESP_ERROR_CHECK( esp_event_handler_instance_register( IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL,
            &instance_got_ip ) );

        ESP_ERROR_CHECK( esp_wifi_set_mode( WIFI_MODE_STA ) );

        wifi_config_t sta_config = {};
        sta_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

        ESP_LOGI( WIFI_TAG, "Using default WiFi credentials" );
        strcpy( (char *)sta_config.sta.ssid, "Techtres" );
        strcpy( (char *)sta_config.sta.password, "Tech2542" );

        ESP_ERROR_CHECK( esp_wifi_set_config( (wifi_interface_t)ESP_IF_WIFI_STA, &sta_config ) );
        ESP_ERROR_CHECK( esp_wifi_start() );
    }

    bool wifi_connected() {
        return fin_wifi_ip_str[0] != '\0';
    }

    const char * wifi_ip() {
        return fin_wifi_ip_str;
    }
}