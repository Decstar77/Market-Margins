#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "rom/gpio.h"

#include "fin-lib.h"

namespace fin {
    constexpr gpio_num_t BUTTON_GPIO = gpio_num_t( 34 );

    static void button_task( void * pvParameters ) {
        vTaskDelay( pdMS_TO_TICKS( 5000 ) );
        while ( true ) {
            vTaskDelay( pdMS_TO_TICKS( 100 ) );
            if ( gpio_get_level( BUTTON_GPIO ) == 0 ) {
                ESP_LOGI( "Button", "Button pressed" );
                swap_strategy();
                vTaskDelay( pdMS_TO_TICKS( 200 ) );
            }
        }
    }

    void button_init() {
        gpio_pad_select_gpio( BUTTON_GPIO );
        gpio_set_direction( BUTTON_GPIO, GPIO_MODE_INPUT );
        xTaskCreate( button_task, "button_task", 2048, NULL, 5, NULL );
    }

}
