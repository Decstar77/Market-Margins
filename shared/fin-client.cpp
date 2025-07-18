#include "fin-client.h" 
//#include "esp_log.h"

#include <thread>

namespace fin {
    void MarketClient::PlaceOrder( RpcCall call, const OrderEntry & order ) {
        RpcCallData callData;
        callData.Call( static_cast<i32>(call), order );
        device->Send( callData.GetFullBuffer(), callData.GetFullLength() );
    }

    void MarketMakerClient::Think() {
        PacketBuffer buffer = {};
        while ( device->Recv( buffer ) == false ) { std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) ); }

        OrderEntry bestBid = {};
        OrderEntry bestAsk = {};
        buffer.Read( bestBid );
        buffer.Read( bestAsk );

        constexpr i32 bid_price = 50;
        if ( bestBid.price < bid_price ) {
            OrderEntry order = {};
            order.symbol = Symbol( "AAPL" );
            order.price = bid_price;
            order.quantity = 1;
            PlaceOrder( RpcCall::PlaceOrder_Bid, order );
        }
    }

    void MarketTakerClient::Think() {
        PacketBuffer buffer = {};
        while ( device->Recv( buffer ) == false ) { std::this_thread::sleep_for( std::chrono::milliseconds( 10 ) ); }

        OrderEntry bestBid = {};
        OrderEntry bestAsk = {};
        buffer.Read( bestBid );
        buffer.Read( bestAsk );

        constexpr i32 ask_price = 100;
        if ( bestAsk.price > ask_price ) {
            OrderEntry order = {};
            order.symbol = Symbol( "AAPL" );
            order.price = ask_price;
            order.quantity = 1;
            PlaceOrder( RpcCall::PlaceOrder_Ask, order );
        }

        std::this_thread::sleep_for( std::chrono::milliseconds( 15 ) );
    }

    void RandomClient::Think() {
        auto start = std::chrono::steady_clock::now();

        float decision = (float)dis( gen );
        if ( decision < 0.5 ) {
            float price = 50 + (float)dis( gen ) * 50;
            OrderEntry order = {};
            order.symbol = Symbol( "AAPL" );
            order.price = static_cast<i32>( price );
            order.quantity = 1;
            PlaceOrder( RpcCall::PlaceOrder_Bid, order );
            device->DisplayTrade( "Bid", (int)order.price, (int)order.quantity );
        }
        else {
            float price = 50 + (float)dis( gen ) * 50;
            OrderEntry order = {};
            order.symbol = Symbol( "AAPL" );
            order.price = static_cast<i32>(price);
            order.quantity = 1;
            PlaceOrder( RpcCall::PlaceOrder_Ask, order );
            device->DisplayTrade( "Ask", (int)order.price, (int)order.quantity );
        }

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        //SP_LOGI( "BENCHMARK", "Think() took %lld milliseconds", duration.count() );

        std::this_thread::sleep_for( std::chrono::milliseconds( 300 ) );
    }
}
