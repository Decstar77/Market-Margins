#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp-lib.h"

#include "ssd1306.h"
#include "font8x8_basic.h"

#include <chrono>
#include <cstring>
#include <string>

#include <condition_variable>
#include <mutex>

namespace fin {

    static const char * TAG = "DISPLAY";

    constexpr int SDA_GPIO = 13;
    constexpr int SCL_GPIO = 16;
    constexpr int RESET_GPIO = -1;
    constexpr int DISPLAY_WIDTH = 128;
    constexpr int DISPLAY_HEIGHT = 64;

    static SSD1306_t dev = {};

    void display_trades_task( void * arg );

    void display_init() {
        ESP_LOGI( TAG, "Initializing display" );
        i2c_master_init( &dev, SDA_GPIO, SCL_GPIO, RESET_GPIO );
        ssd1306_init( &dev, DISPLAY_WIDTH, DISPLAY_HEIGHT );
        ssd1306_clear_screen( &dev, false );
        ssd1306_contrast( &dev, 0xff );
        ssd1306_display_text_x3( &dev, 0, "Hello", 5, false );
        vTaskDelay( 3000 / portTICK_PERIOD_MS );
        xTaskCreate( display_trades_task, "Display Trades", 4096, NULL, configMAX_PRIORITIES - 1, NULL );
    }

    static void display_text( SSD1306_t * dev, int page, int seg, const char * text, int text_len, bool invert ) {
        if ( page >= dev->_pages ) return;
        int _text_len = text_len;
        if ( _text_len > 16 ) _text_len = 16;

        uint8_t image[8];
        for ( int i = 0; i < _text_len; i++ ) {
            memcpy( image, font8x8_basic_tr[(uint8_t)text[i]], 8 );
            if ( invert ) ssd1306_invert( image, 8 );
            if ( dev->_flip ) ssd1306_flip( image, 8 );
            ssd1306_display_image( dev, page, seg, image, 8 );
            seg = seg + 8;
        }
    }

    inline int get_text_width( const char * text ) {
        return strlen( text ) * 8;
    }

    static std::string trades[4] = {};
    static std::string header_text = {};
    static bool display = false;
    static std::mutex mut = {};
    static std::condition_variable cv = {};

    void display_set_header_text( const char * text ) {
        header_text = text;
        ssd1306_clear_screen( &dev, false );
        ssd1306_contrast( &dev, 0xff );
        int text_width = get_text_width( text );
        int seg = (DISPLAY_WIDTH - text_width) / 2;
        if ( seg < 0 ) { seg = 0; }
        display_text( &dev, 0, seg, text, strlen( text ), false );
    }

    void display_trades_task( void * arg ) {
        while ( true ) {
            {
                std::unique_lock<std::mutex> lock( mut );
                cv.wait( lock, []() { return display; } );

                for ( int i = 0; i < 4; i++ ) {
                    if ( trades[i].empty() ) continue;
                    int text_width = get_text_width( trades[i].c_str() );
                    int seg = (DISPLAY_WIDTH - text_width) / 2;
                    if ( seg < 0 ) { seg = 0; }
                    ssd1306_clear_line( &dev, 2 + i, false );
                    display_text( &dev, 2 + i, seg, trades[i].c_str(), strlen( trades[i].c_str() ), false );
                }

                display = false;
            }
        }
    }

    void display_append_trade( const char * text, int price, int quantity ) {
        if ( mut.try_lock() ) {
            trades[3] = trades[2];
            trades[2] = trades[1];
            trades[1] = trades[0];
            trades[0] = text;
            trades[0] += "|" + std::to_string( price ) + "|" + std::to_string( quantity );
            display = true;
            mut.unlock();
            cv.notify_one();
        }
    }
}