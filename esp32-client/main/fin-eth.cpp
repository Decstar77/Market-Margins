#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "rom/gpio.h"
#include "fin-lib.h"
#include <string.h>
#define TAG "Eth"

static char ip_address[32] = {};

#define RESET_COUNT_THRESHOLD 10
static int eth_down_count = 0;

bool fin_eth_connected() {
    return ip_address[0] != '\0';
}

const char * fin_eth_ip() {
    return ip_address;
}

static void eth_event_handler( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data ) {
    uint8_t mac_addr[6] = { 0 };
    esp_eth_handle_t eth_handle = *(esp_eth_handle_t *)event_data;

    switch ( event_id ) {
    case ETHERNET_EVENT_CONNECTED: {
        esp_eth_ioctl( eth_handle, ETH_CMD_G_MAC_ADDR, mac_addr );
        ESP_LOGI( TAG, "Ethernet Link Up" );
        ESP_LOGI( TAG, "Ethernet HW Addr %02x:%02x:%02x:%02x:%02x:%02x",
            mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5] );
        break;
    }
    case ETHERNET_EVENT_DISCONNECTED: {
        ESP_LOGI( TAG, "Ethernet Link Down" );
        memset( ip_address, 0, sizeof( ip_address ) );
        eth_down_count++;
        if ( eth_down_count >= RESET_COUNT_THRESHOLD ) {
            ESP_LOGE( TAG, "Ethernet Link Down count threshold reached, resetting" );
            esp_restart();
        }
        break;
    }
    case ETHERNET_EVENT_START: {
        ESP_LOGI( TAG, "Ethernet Started" );
        break;
    }
    case ETHERNET_EVENT_STOP: {
        ESP_LOGI( TAG, "Ethernet Stopped" );
        break;
    }
    default:
        break;
    }
}

static void eth_got_ip_event_handler( void * arg, esp_event_base_t event_base, int32_t event_id, void * event_data ) {
    ip_event_got_ip_t * event = (ip_event_got_ip_t *)event_data;
    const esp_netif_ip_info_t * ip_info = &event->ip_info;

    ESP_LOGI( TAG, "Ethernet Got IP Address" );
    ESP_LOGI( TAG, "~~~~~~~~~~~" );
    ESP_LOGI( TAG, "ETHIP:" IPSTR, IP2STR( &ip_info->ip ) );
    ESP_LOGI( TAG, "ETHMASK:" IPSTR, IP2STR( &ip_info->netmask ) );
    ESP_LOGI( TAG, "ETHGW:" IPSTR, IP2STR( &ip_info->gw ) );
    ESP_LOGI( TAG, "~~~~~~~~~~~" );
    esp_ip4addr_ntoa( &event->ip_info.ip, ip_address, sizeof( ip_address ) );
}

void fin_ethernet_init() {
    eth_mac_config_t mac_config = ETH_MAC_DEFAULT_CONFIG();
    eth_phy_config_t phy_config = ETH_PHY_DEFAULT_CONFIG();
    phy_config.phy_addr = 0;
    phy_config.reset_gpio_num = 12;

    eth_esp32_emac_config_t esp32_emac_config = ETH_ESP32_EMAC_DEFAULT_CONFIG();
    esp32_emac_config.smi_gpio.mdc_num = 23;
    esp32_emac_config.smi_gpio.mdio_num = 18;

    esp_eth_mac_t * mac = esp_eth_mac_new_esp32( &esp32_emac_config, &mac_config );
    esp_eth_phy_t * phy = esp_eth_phy_new_lan87xx( &phy_config );

    esp_eth_handle_t eth_handle = NULL;
    esp_eth_config_t config = ETH_DEFAULT_CONFIG( mac, phy );
    esp_err_t res = esp_eth_driver_install( &config, &eth_handle );

    if ( res != ESP_OK ) {
        ESP_LOGE( TAG, "FAILED TO INSTALL DRIVER" );
        return;
    }
    else {
        ESP_LOGI( TAG, "Successfully installed driver" );
    }

    ESP_ERROR_CHECK( esp_netif_init() );
    ESP_ERROR_CHECK( esp_event_loop_create_default() );

    esp_netif_config_t cfg = ESP_NETIF_DEFAULT_ETH();
    esp_netif_t * eth_netif = esp_netif_new( &cfg );
    ESP_ERROR_CHECK( esp_netif_attach( eth_netif, esp_eth_new_netif_glue( eth_handle ) ) );

    ESP_ERROR_CHECK( esp_event_handler_register( ETH_EVENT, ESP_EVENT_ANY_ID, &eth_event_handler, NULL ) );
    ESP_ERROR_CHECK( esp_event_handler_register( IP_EVENT, IP_EVENT_ETH_GOT_IP, &eth_got_ip_event_handler, NULL ) );

    ESP_ERROR_CHECK( esp_eth_start( eth_handle ) );
}