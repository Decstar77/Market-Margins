#pragma once

#include "fin-api.h"

namespace fin {
    enum class Strategy {
        MarketMaker = 0,
        MarketTaker = 1,
        Random = 2,

        __COUNT__
    };

    class DeviceInterface {
    public:
        virtual void        Send( const void * data, int size ) = 0;
        virtual void        Recv( PacketBuffer & buffer ) = 0;
        virtual void        DisplayHeaderText( const char * text ) = 0;
        virtual void        DisplayTrade( const char * text, int price, int quantity ) = 0;
    };

    class MarketClient {
    public:
        MarketClient( Strategy strat, DeviceInterface * device ) : device( device ), strategy( strat ) {}

        void                Think();
        inline Strategy     GetStrategy() const { return strategy; }

    protected:
        void                PlaceOrder( RpcCall call, const OrderEntry & order );
        virtual void        DoStrategy( const OrderEntry & bestBid, const OrderEntry & bestAsk ) = 0;

    protected:
        DeviceInterface * device;

    private:
        Strategy strategy;
    };

    class MarketMakerClient : public MarketClient {
    public:
        MarketMakerClient( DeviceInterface * device ) : MarketClient( Strategy::MarketMaker, device ) { device->DisplayHeaderText( "Market Maker" ); }
        virtual void DoStrategy( const OrderEntry & bestBid, const OrderEntry & bestAsk ) override;
    };

    class MarketTakerClient : public MarketClient {
    public:
        MarketTakerClient( DeviceInterface * device ) : MarketClient( Strategy::MarketTaker, device ) { device->DisplayHeaderText( "Market Taker" ); }
        virtual void DoStrategy( const OrderEntry & bestBid, const OrderEntry & bestAsk ) override;
    };

    class RandomClient : public MarketClient {
    public:
        RandomClient( DeviceInterface * device ) : MarketClient( Strategy::Random, device ), gen( rd() ), dis( 0.0, 1.0 ) { device->DisplayHeaderText( "Random Trader" ); }
        virtual void DoStrategy( const OrderEntry & bestBid, const OrderEntry & bestAsk ) override;

    private:
        std::random_device rd;
        std::mt19937 gen;
        std::uniform_real_distribution<> dis;
    };

    class StrategyFactory {
    public:
        static std::unique_ptr<MarketClient> Create( Strategy strat, DeviceInterface * device ) {
            switch ( strat ) {
            case Strategy::MarketMaker: return std::make_unique<MarketMakerClient>( device );
            case Strategy::MarketTaker: return std::make_unique<MarketTakerClient>( device );
            case Strategy::Random: return std::make_unique<RandomClient>( device );
            default: return std::make_unique<MarketMakerClient>( device );
            }
        }
    };
}

