#include "fin-pch.h"
#include "fin-platform.h"
#include "fin-rpc.h"
#include "fin-orderbook.h"
#include "fin-replay.h"
#include "fin-queues.h"
#include "fin-log.h"

#include <random>
std::random_device rd;
std::mt19937 gen( rd() );
std::uniform_int_distribution<u64> dis( 1, std::numeric_limits<u64>::max() );

/*
    ======= Some AI ideas that I think will be interesting to implement/explore =======

    1. Order Matching Engine
    * (DONE) Order Matching Engine: Basic implementation of price-time priority, partial fills, cancellations, etc.
    * (DONE) Latency Modeling: Simulate geographic latency between traders and the exchange (e.g. delay ESP32 responses).

    2. Analytics & Quant Research
    * (DONE) Replay Engine: Record all order flow and allow replaying for backtesting strategies.
    * Microstructure Metrics: Order-to-trade ratio, bid-ask spread, volume, Price impact per order size, etc.
    * Order Flow Analysis: Visualize the impact of spoofing, front-running, etc.

    3. Architecture & Distributed Systems
    * Clustered Matching Engine: Use message queues or gRPC to shard matching engines per symbol.
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

using namespace fin;

int main() {
    RpcTable table;
    OrderBook book( Symbol( "AAPL" ) );
    ReplayEngine replay( std::format( "replay_{}.bin", std::chrono::system_clock::now().time_since_epoch().count() ) );

    OrderEntry startingBid = {};
    startingBid.id = dis( gen );
    startingBid.time = std::chrono::system_clock::now().time_since_epoch().count();
    startingBid.price = 40;
    startingBid.quantity = 100;
    startingBid.symbol = Symbol( "AAPL" );
    book.AddBid( startingBid );

    OrderEntry startingAsk = {};
    startingAsk.id = dis( gen );
    startingAsk.time = std::chrono::system_clock::now().time_since_epoch().count();
    startingAsk.price = 110;
    startingAsk.quantity = 100;
    startingAsk.symbol = Symbol( "AAPL" );
    book.AddAsk( startingAsk );

    table.Register( static_cast<i32>(RpcCall::PlaceOrder_Bid), new RpcFunction<void, OrderEntry>( [&book, &replay]( OrderEntry entry )
        {
            entry.id = dis( gen );
            entry.time = std::chrono::system_clock::now().time_since_epoch().count();
            book.AddBid( entry );

            replay.Append( RpcCall::PlaceOrder_Bid );
            replay.Append( entry );

            PacketBuffer buffer;
            buffer.Write( entry.id );
            TCPServerSendPacket( buffer );
        } ) );

    table.Register( static_cast<i32>(RpcCall::PlaceOrder_Ask), new RpcFunction<void, OrderEntry>( [&book, &replay]( OrderEntry entry )
        {
            entry.id = dis( gen );
            entry.time = std::chrono::system_clock::now().time_since_epoch().count();
            book.AddAsk( entry );

            replay.Append( RpcCall::PlaceOrder_Ask );
            replay.Append( entry );

            PacketBuffer buffer;
            buffer.Write( entry.id );

            TCPServerSendPacket( buffer );
        } ) );


    table.Register( static_cast<i32>(RpcCall::CancelOrder_Bid), new RpcFunction<void, i64, Symbol>( [&book, &replay]( i64 id, Symbol symbol )
        {
            bool result = book.RemoveBid( id );

            replay.Append( RpcCall::CancelOrder_Bid );
            replay.Append( id );

            PacketBuffer buffer;
            buffer.Write( result );
            TCPServerSendPacket( buffer );
        } ) );

    table.Register( static_cast<i32>(RpcCall::CancelOrder_Ask), new RpcFunction<void, i64, Symbol>( [&book, &replay]( i64 id, Symbol symbol )
        {
            bool result = book.RemoveAsk( id );

            replay.Append( RpcCall::CancelOrder_Ask );
            replay.Append( id );

            PacketBuffer buffer;
            buffer.Write( result );
            TCPServerSendPacket( buffer );
        } ) );

    TCPServerSocket( "54000" );
    UDPServerSocket( "54001", "239.255.0.1" );

    auto lastUpdate = std::chrono::steady_clock::now();

    while ( true ) {
        // std::this_thread::sleep_for( std::chrono::milliseconds( 500 ) );
        bool recOrder = false;
        PacketBuffer recBuffer = {};
        if ( TCPServerReceivePacket( recBuffer ) ) {
            recOrder = true;
            fin::RpcCallData rpcCall( recBuffer.data, recBuffer.length );
            table.Call( rpcCall );
        }

        auto now = std::chrono::steady_clock::now();
        bool shouldUpdate = std::chrono::duration_cast<std::chrono::seconds>(now - lastUpdate).count() >= 10;

        if ( recOrder || shouldUpdate ) {
            lastUpdate = now;

            OrderEntry bestBid = {};
            OrderEntry bestAsk = {};
            book.GetL1MarketData( bestBid, bestAsk );

            PacketBuffer sendBuffer = {};
            sendBuffer.Write( bestBid );
            sendBuffer.Write( bestAsk );
            UDPServerMulticastPacket( sendBuffer );

            const i64 midPrice = (bestAsk.price - bestBid.price) / 2;
            const i64 spread = bestAsk.price - bestBid.price;
            LOG_INFO( " {:>3} | {:>3} | {:>3} | {:>3} | {:>3} | {:>3} | {:>3} | {:>3}",
                "Best Bid", bestBid.price, "Best Ask", bestAsk.price, "Mid Price", midPrice, "Spread", spread );
        }
    }

    return 0;
}

