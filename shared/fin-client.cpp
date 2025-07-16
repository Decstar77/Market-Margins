#include "fin-client.h" 

#include <thread>

namespace fin {
    void MarketClient::PlaceOrder( RpcCall call, const OrderEntry & order ) {
        RpcBuilder builder( call );
        builder.WriteArg( order );
        device->Send( builder.GetData(), builder.GetSize() );
    }

    void MarketClient::Think() {
        PacketBuffer buffer = {};
        device->Recv( buffer );

        OrderEntry bestBid = {};
        OrderEntry bestAsk = {};
        buffer.Read( bestBid );
        buffer.Read( bestAsk );

        DoStrategy( bestBid, bestAsk );
    }

    void MarketMakerClient::DoStrategy( const OrderEntry & bestBid, const OrderEntry & bestAsk ) {
        constexpr i32 bid_price = 50;
        if ( bestBid.price < bid_price ) {
            OrderEntry order = {};
            order.symbol = Symbol( "AAPL" );
            order.price = bid_price;
            order.quantity = 1;
            PlaceOrder( RpcCall::PlaceOrder_Bid, order );
        }
    }

    void MarketTakerClient::DoStrategy( const OrderEntry & bestBid, const OrderEntry & bestAsk ) {
        constexpr i32 ask_price = 100;
        if ( bestAsk.price > ask_price ) {
            OrderEntry order = {};
            order.symbol = Symbol( "AAPL" );
            order.price = ask_price;
            order.quantity = 1;
            PlaceOrder( RpcCall::PlaceOrder_Ask, order );
        }
    }

    void RandomClient::DoStrategy( const OrderEntry & bestBid, const OrderEntry & bestAsk ) {
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

        float sleepTime = 5 + (float)dis( gen ) * 30;
        std::this_thread::sleep_for( std::chrono::milliseconds( static_cast<i32>(sleepTime) ) );
    }
}
