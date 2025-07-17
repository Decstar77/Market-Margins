#pragma once

#include "fin-platform.h"
#include "fin-priority-queue.h"

namespace fin {

    struct OrderResult {
        OrderEntry bestBid;
        OrderEntry bestAsk;
        i64 amount;
    };

    struct L2MarketData {
        i64 price;
        i64 quantity;
    };

    inline static const OrderEntry * CompareEntries_Bids( const OrderEntry * a, const OrderEntry * b ) {
        if ( a->price > b->price ) return a; // Higher price wins
        if ( a->price < b->price ) return b;
        return (a->time < b->time) ? a : b; // Earlier timestamp wins
    }

    inline static const OrderEntry * CompareEntries_Asks( const OrderEntry * a, const OrderEntry * b ) {
        if ( a->price < b->price ) return a; // Lower price wins
        if ( a->price > b->price ) return b;
        return (a->time < b->time) ? a : b; // Earlier timestamp wins
    }

    class OrderBook {
    public:
        OrderBook() {};
        OrderBook( const Symbol & symbol ) : symbol( symbol ) {}

        OrderResult AddBid( OrderEntry & entry );
        OrderResult AddAsk( OrderEntry & entry );

        OrderResult RemoveBid( i64 id );
        OrderResult RemoveAsk( i64 id );

        // L1 market data. Just the top of the book
        std::pair<bool, bool> GetL1MarketData( OrderEntry & bid, OrderEntry & ask ) const;

        // L2 market data. An aggregated view, just prices and quantities
        void GetL2MarketData( std::vector<L2MarketData> & l2bids, std::vector<L2MarketData> & l2asks, int depth );

        // L3 market data. A full view into the order book.
        void GetL3MarketData( std::vector<OrderEntry> & l3bids, std::vector<OrderEntry> & l3asks, int depth );

        inline u64 GetOrderCount() const { return orderCount; }
        inline u64 GetCancelCount() const { return cancelCount; }
        inline u64 GetTradeCount() const { return tradeCount; }
        inline u64 GetVolume() const { return volume; }

        inline double GetOrderToTradeRatio() const { return static_cast<double>(orderCount) / static_cast<double>(tradeCount); }
        inline double GetVolumePerTrade() const { return static_cast<double>(volume) / static_cast<double>(tradeCount); }

    private:
        void CompleteOrder( OrderEntry & entry );
        i64 ResolveBook();

    private:
        u64 orderCount = 0;
        u64 cancelCount = 0;
        u64 tradeCount = 0;
        u64 volume = 0;
        Symbol symbol = Symbol();
        PriorityQueue<OrderEntry> bids = PriorityQueue<OrderEntry>( &CompareEntries_Bids );
        PriorityQueue<OrderEntry> asks = PriorityQueue<OrderEntry>( &CompareEntries_Asks );
    };

    namespace tests {
        void RunOrderBookTests();
    }
}


