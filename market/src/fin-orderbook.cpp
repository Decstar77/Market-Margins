#include "fin-pch.h"
#include "fin-orderbook.h"

#include "fin-log.h"

namespace fin {
    i64 OrderBook::AddBid( OrderEntry entry ) {
        // LOG_INFO( "AddBid: {} {}, {}", entry.symbol.AsString(), entry.price, entry.quantity );
        bids.Push( entry );
        return ResolveBook();
    }

    i64 OrderBook::AddAsk( OrderEntry entry ) {
        //  LOG_INFO( "AddAsk: {} {}, {}", entry.symbol.AsString(), entry.price, entry.quantity );
        asks.Push( entry );
        return ResolveBook();
    }

    bool OrderBook::RemoveBid( i64 id ) {
        LOG_INFO( "RemoveBid: {}", id );
        return bids.RemoveOnce( [id]( const OrderEntry * entry ) { return entry->id == id; } );
    }

    bool OrderBook::RemoveAsk( i64 id ) {
        LOG_INFO( "RemoveAsk: {}", id );
        return asks.RemoveOnce( [id]( const OrderEntry * entry ) { return entry->id == id; } );
    }

    std::pair<bool, bool> OrderBook::GetL1MarketData( OrderEntry & bid, OrderEntry & ask ) const {
        return std::make_pair( bids.PeekCopy( bid ), asks.PeekCopy( ask ) );
    }

    void OrderBook::GetL2MarketData( std::vector<L2MarketData> & l2bids, std::vector<L2MarketData> & l2asks, int depth ) {

    }

    void OrderBook::GetL3MarketData( std::vector<OrderEntry> & l3bids, std::vector<OrderEntry> & l3asks, int depth ) {

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

            bid = nullptr;
            ask = nullptr;
            bids.Peek( bid );
            asks.Peek( ask );
        }

        return value;
    }
}
