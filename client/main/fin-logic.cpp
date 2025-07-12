#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "fin-lib.h"

#include "../../market/src/fin-api.h"

#include <random>

namespace fin {

    static const char * TAG = "LOGIC";
    static std::random_device rd;
    static std::mt19937 gen( rd() );
    static std::uniform_real_distribution<> dis( 0.0, 1.0 );

    inline void PlaceOrder( RpcCall call, const OrderEntry & order ) {
        RpcBuilder builder( call );
        builder.WriteArg( order );
        client_send( builder.GetData(), builder.GetSize() );
    }

    enum class Strategy {
        Invalid = 0,
        MarketMaker = 1,
        MarketTaker = 2,
        Random = 3,
    };

    static Strategy strategy = Strategy::Random;

    inline void DoStrategy( const OrderEntry & bestBid, const OrderEntry & bestAsk ) {
        if ( strategy == Strategy::MarketMaker ) {
            constexpr i32 bid_price = 50;
            if ( bestBid.price < bid_price ) {
                OrderEntry order = {};
                order.symbol = Symbol( "AAPL" );
                order.price = bid_price;
                order.quantity = 1;
                PlaceOrder( RpcCall::PlaceOrder_Bid, order );
            }
        }
        else if ( strategy == Strategy::MarketTaker ) {
            constexpr i32 ask_price = 100;
            if ( bestAsk.price > ask_price ) {
                OrderEntry order = {};
                order.symbol = Symbol( "AAPL" );
                order.price = ask_price;
                order.quantity = 1;
                PlaceOrder( RpcCall::PlaceOrder_Ask, order );
            }
        }
        else if ( strategy == Strategy::Random ) {
            float decision = dis( gen );
            if ( decision < 0.5 ) {
                float price = 50 + dis( gen ) * 50;
                OrderEntry order = {};
                order.symbol = Symbol( "AAPL" );
                order.price = price;
                order.quantity = 1;
                PlaceOrder( RpcCall::PlaceOrder_Bid, order );
                ESP_LOGI( TAG, "Bid: %d %d", (int)order.price, (int)order.quantity );
                display_append_trade( "Bid", (int)order.price, (int)order.quantity );
            }
            else {
                float price = 50 + dis( gen ) * 50;
                OrderEntry order = {};
                order.symbol = Symbol( "AAPL" );
                order.price = price;
                order.quantity = 1;
                PlaceOrder( RpcCall::PlaceOrder_Ask, order );
                ESP_LOGI( TAG, "Ask: %d %d", (int)order.price, (int)order.quantity );
                display_append_trade( "Ask", (int)order.price, (int)order.quantity );
            }
        }
    }

    void logic_think_task( void * arg ) {
        switch ( strategy ) {
            case Strategy::Invalid: display_set_header_text( "Invalid Strategy" ); break;
            case Strategy::MarketMaker: display_set_header_text( "Market Maker" ); break;
            case Strategy::MarketTaker: display_set_header_text( "Market Taker" ); break;
            case Strategy::Random: display_set_header_text( "Random Trader" ); break;
        }

        while ( true ) {
            PacketBuffer buffer = {};
            client_udp_recv( buffer );

            Symbol symbol = {};
            buffer.Read( symbol );

            if ( symbol != Symbol( "AAPL" ) ) {
                continue;
            }

            OrderEntry bestBid = {};
            OrderEntry bestAsk = {};
            buffer.Read( bestBid );
            buffer.Read( bestAsk );

            DoStrategy( bestBid, bestAsk );

            vTaskDelay( 1000 );
        }
    }

    void logic_init() {
        storage_save_int( "strategy", (int)Strategy::Random );
        strategy = (Strategy)storage_load_int( "strategy", (int)Strategy::Random );
        xTaskCreate( logic_think_task, "logic_think", 4096, NULL, 5, NULL );
    }
}


