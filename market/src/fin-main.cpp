#include "fin-pch.h"
#include "fin-platform.h"
#include "fin-rpc.h"
#include "fin-orderbook.h"
#include "fin-replay.h"
#include "fin-queues.h"
#include "fin-log.h"

#include <random>

using namespace fin;

/*
    ======= Some AI ideas that I think will be interesting to implement/explore =======

    1. Order Matching Engine
    * (DONE) Order Matching Engine: Basic implementation of price-time priority, partial fills, cancellations, etc.
    * (DONE) Latency Modeling: Simulate geographic latency between traders and the exchange (e.g. delay ESP32 responses).

    2. Analytics & Quant Research
    * (DONE) Replay Engine: Record all order flow and allow replaying for backtesting strategies.
    * (DONE) Microstructure Metrics: Order-to-trade ratio, bid-ask spread, volume, Price impact per order size, etc.
    * Order Flow Analysis: Visualize the impact of spoofing, front-running, etc.

    3. Architecture & Distributed Systems
    * (DONE) Clustered Matching Engine: Use message queues or gRPC to shard matching engines per symbol.
    * Exchange Gateway: Separate matching from networking, simulate FIX or ITCH protocol compatibility.
    * (DONE) Horizontal Scaling: Add more ESP32 “trader” nodes (or even Raspberry Pi clusters), simulate co-location vs. edge trading.

    4. Optimization
    * Benchmark current orderbook implementation
    * Explore different data structures / algorithms for the orderbook
    * Explore better multithreading / async implementation
    * Explore lock free / lockless implementation.
    * Explore latency optimization.

    5. Visualization & Frontend
    * Live Order Book UI (React + WebSockets)
    * Heatmaps of volume, latency, or volatility
    * Dashboard for Traders with latency, fill rate, PnL per node

    6. Trading Strategy Framework
    * Implement a simple trading strategy framework that can be used to test and backtest strategies.

    7. Machine Learning Angle
    * Predict short-term price movement from L1/L2 data
    * Classify trader behavior (e.g. aggressive vs passive)
    * Reinforcement learning: Agent that learns to trade profitably in your market
*/

namespace fin {
    struct CompletedOrder {
        RpcCall         call;
        OrderEntry      entry;
        OrderResult     result;
    };

    inline static void AddToCompletedQueue( SPSCQueue<CompletedOrder> & queue, const RpcCall & call, const OrderEntry & entry, const OrderResult & result ) {
        CompletedOrder order;
        order.entry = entry;
        order.call = call;
        order.result = result;
        queue.Push( order );
    }
}

int main() {
    RpcTable        table;
    OrderBook       book( Symbol( "AAPL" ) );
    ReplayEngine    replay( std::format( "replay_{}.bin", std::chrono::system_clock::now().time_since_epoch().count() ) );

    OrderEntry startingBid = {};
    startingBid.price = 40;
    startingBid.quantity = 100;
    startingBid.symbol = Symbol( "AAPL" );
    book.AddBid( startingBid );

    OrderEntry startingAsk = {};
    startingAsk.price = 110;
    startingAsk.quantity = 100;
    startingAsk.symbol = Symbol( "AAPL" );
    book.AddAsk( startingAsk );

    SPSCQueue<CompletedOrder>   orderCompleteQueue( 10000 );

    std::thread replayAndReplyThread( [&orderCompleteQueue, &replay]()
        {
            auto lastUpdate = std::chrono::steady_clock::now();
            while ( true ) {
                OrderEntry bestBid = {};
                OrderEntry bestAsk = {};
                CompletedOrder entry = {};
                bool rec = false;
                while ( orderCompleteQueue.Pop( entry ) ) {
                    rec = true;

                    replay.Append( RpcCall::PlaceOrder_Bid );
                    replay.Append( entry );

                    bestBid = entry.result.bestBid;
                    bestAsk = entry.result.bestAsk;

                    PacketBuffer buffer;
                    buffer.Write( entry.entry.id );
                    TCPServerSendPacket( buffer );
                }

                auto now = std::chrono::steady_clock::now();
                bool shouldUpdate = std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate).count() >= 10;

                if ( rec || shouldUpdate ) {
                    lastUpdate = now;

                    PacketBuffer sendBuffer = {};
                    sendBuffer.Write( bestBid );
                    sendBuffer.Write( bestAsk );
                    UDPServerMulticastPacket( sendBuffer );

                    const i64 midPrice = (bestAsk.price - bestBid.price) / 2;
                    const i64 spread = bestAsk.price - bestBid.price;
                    LOG_INFO( " {:>3} | {:>3} | {:>3} | {:>3} | {:>3} | {:>3} | {:>3} | {:>3}",
                        "Best Bid", bestBid.price, "Best Ask", bestAsk.price, "Mid Price", midPrice, "Spread", spread );

                    // LOG_INFO( "Order Count: {:>3} | Trade Count: {:>3} | Volume: {:>3} | Order/Trade: {:>3.2f} | Volume/Trade: {:>3.2f}",
                        // book.GetOrderCount(), book.GetTradeCount(), book.GetVolume(), book.GetOrderToTradeRatio(), book.GetVolumePerTrade() );
                }
            }
        } );

    table.Register( static_cast<i32>(RpcCall::PlaceOrder_Bid), new RpcFunction<void, OrderEntry>( [&book, &orderCompleteQueue]( OrderEntry entry )
        {
            const OrderResult result = book.AddBid( entry );
            AddToCompletedQueue( orderCompleteQueue, RpcCall::PlaceOrder_Bid, entry, result );
        } ) );

    table.Register( static_cast<i32>(RpcCall::PlaceOrder_Ask), new RpcFunction<void, OrderEntry>( [&book, &orderCompleteQueue]( OrderEntry entry )
        {
            const OrderResult result = book.AddAsk( entry );
            AddToCompletedQueue( orderCompleteQueue, RpcCall::PlaceOrder_Ask, entry, result );
        } ) );

    table.Register( static_cast<i32>(RpcCall::CancelOrder_Bid), new RpcFunction<void, i64, Symbol>( [&book, &orderCompleteQueue]( i64 id )
        {
            const OrderResult result = book.RemoveBid( id );
            OrderEntry entry = {}; entry.id = id;
            AddToCompletedQueue( orderCompleteQueue, RpcCall::CancelOrder_Bid, entry, result );
        } ) );

    table.Register( static_cast<i32>(RpcCall::CancelOrder_Ask), new RpcFunction<void, i64, Symbol>( [&book, &orderCompleteQueue]( i64 id )
        {
            const OrderResult result = book.RemoveAsk( id );
            OrderEntry entry = {}; entry.id = id;
            AddToCompletedQueue( orderCompleteQueue, RpcCall::CancelOrder_Ask, entry, result );
        } ) );

    TCPServerSocket( "54000" );
    UDPServerSocket( "54001", "239.255.0.1" );

    while ( true ) {
        // std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
        bool recOrder = false;
        PacketBuffer recBuffer = {};
        if ( TCPServerReceivePacket( recBuffer ) ) {
            recOrder = true;
            fin::RpcCallData rpcCall( recBuffer.data, recBuffer.length );
            table.Call( rpcCall );
        }
    }

    return 0;
}

