#include "fin-pch.h"
#include "fin-orderbook.h"

#include "fin-log.h"

namespace fin {
    OrderResult OrderBook::AddBid( OrderEntry & entry ) {
        // LOG_INFO( "AddBid: {} {}, {}", entry.symbol.AsString(), entry.price, entry.quantity );
        CompleteOrder( entry );
        bids.Push( entry );
        const i64 amount = ResolveBook();

        orderCount++;
        volume += entry.quantity;

        OrderResult result = {};
        result.amount = amount;
        GetL1MarketData( result.bestBid, result.bestAsk );
        return result;
    }

    OrderResult OrderBook::AddAsk( OrderEntry & entry ) {
        // LOG_INFO( "AddAsk: {} {}, {}", entry.symbol.AsString(), entry.price, entry.quantity );
        CompleteOrder( entry );
        asks.Push( entry );
        const i64 amount = ResolveBook();

        orderCount++;
        volume += entry.quantity;

        OrderResult result = {};
        result.amount = amount;
        GetL1MarketData( result.bestBid, result.bestAsk );
        return result;
    }

    OrderResult OrderBook::RemoveBid( i64 id ) {
        // LOG_INFO( "RemoveBid: {}", id );
        cancelCount++;
        OrderResult result = {};
        result.amount = static_cast<i64>(bids.RemoveOnce( [id]( const OrderEntry * entry ) { return entry->id == id; } ));
        GetL1MarketData( result.bestBid, result.bestAsk );
        return result;
    }

    OrderResult OrderBook::RemoveAsk( i64 id ) {
        // LOG_INFO( "RemoveAsk: {}", id );
        cancelCount++;
        OrderResult result = {};
        result.amount = asks.RemoveOnce( [id]( const OrderEntry * entry ) { return entry->id == id; } );
        GetL1MarketData( result.bestBid, result.bestAsk );
        return result;
    }

    std::pair<bool, bool> OrderBook::GetL1MarketData( OrderEntry & bid, OrderEntry & ask ) const {
        return std::make_pair( bids.PeekCopy( bid ), asks.PeekCopy( ask ) );
    }

    void OrderBook::GetL2MarketData( std::vector<L2MarketData> & l2bids, std::vector<L2MarketData> & l2asks, int depth ) {

    }

    void OrderBook::GetL3MarketData( std::vector<OrderEntry> & l3bids, std::vector<OrderEntry> & l3asks, int depth ) {

    }

    void OrderBook::CompleteOrder( OrderEntry & entry ) {
        entry.id = orderCount;
        entry.time = std::chrono::system_clock::now().time_since_epoch().count();
    }

    i64 OrderBook::ResolveBook() {
        OrderEntry * bid = nullptr;
        OrderEntry * ask = nullptr;
        bids.Peek( bid );
        asks.Peek( ask );

        i64 value = 0;
        while ( bid != nullptr && ask != nullptr && bid->price >= ask->price ) {
            const i64 aq = ask->quantity;
            ask->quantity = std::max( 0ll, ask->quantity - bid->quantity );
            bid->quantity = std::max( 0ll, bid->quantity - aq );

            value += (aq - ask->quantity) * ask->price;

            if ( ask->quantity == 0 ) { asks.Pop(); }
            if ( bid->quantity == 0 ) { bids.Pop(); }

            tradeCount++;

            bid = nullptr;
            ask = nullptr;
            bids.Peek( bid );
            asks.Peek( ask );
        }

        return value;
    }
}
