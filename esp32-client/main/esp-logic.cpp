#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp-lib.h"

#include "../../shared/fin-client.h"

namespace fin {

    static const char * TAG = "LOGIC";
    static Strategy strategy = Strategy::Random;

    // When the button is pressed, swap the strategy
    void swap_strategy() {
        strategy = (Strategy)( ( (int)strategy + 1 ) % (int)Strategy::__COUNT__ );
        storage_save_int( "strategy", (int)strategy );
    }

    void logic_think_task( void * arg ) {
        DeviceInterface * device = device_interface();
        std::unique_ptr<MarketClient> client = StrategyFactory::Create( strategy, device );
        while ( true ) {
            vTaskDelay( pdMS_TO_TICKS( 1000 ) );
            client->Think();

            if ( strategy != client->GetStrategy() ) {
                ESP_LOGI( TAG, "Strategy changed to %d", (int)strategy );
                client = StrategyFactory::Create( strategy, device );
            }
        }
    }

    void logic_init() {
        strategy = (Strategy)storage_load_int( "strategy", (int)Strategy::Random );
        xTaskCreate( logic_think_task, "logic_think", 4096, NULL, 5, NULL );
    }
}


