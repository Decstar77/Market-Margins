#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "esp-lib.h"

using namespace fin;

extern "C" void app_main() {
    storage_init();
    button_init();
    //wifi_init();
    ethernet_init();
    client_init();
    display_init();
    logic_init();
}
