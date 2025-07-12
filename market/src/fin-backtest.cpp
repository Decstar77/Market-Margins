#include "fin-pch.h"

using json = nlohmann::json;

class MarketBacktestData {
public:
    std::string symbol;
    std::vector<u64> timestamps;
    std::vector<f64> closings;
    std::vector<f64> openings;

    MarketBacktestData(const std::string& symbol) : symbol(symbol) {
    }

    void LoadFromJsonFile(const std::string& filename) {
        std::ifstream f(filename);
        if ( f.is_open() == false ) { 
            throw std::runtime_error("Failed to open file");
        }

        json data = json::parse(f);

        auto close_data = data["('Close', 'AAPL')"];
        for ( auto it = close_data.begin(); it != close_data.end(); ++it ) {
            closings.push_back(it.value());
            timestamps.push_back(std::stoull(it.key()));
        }

        auto open_data = data["('Open', 'AAPL')"];
        for ( auto it = open_data.begin(); it != open_data.end(); ++it ) {
            openings.push_back(it.value());
        }
    }

    void PrintMarketData() {
        std::cout << std::setw(15) << "Timestamp" << " | " 
                  << std::setw(10) << "Open" << " | " 
                  << std::setw(10) << "Close" << std::endl;

        for ( size_t i = 0; i < timestamps.size(); i++ ) {  
            std::cout << std::setw(15) << timestamps[i] << " | "
                      << std::setw(10) << std::fixed << std::setprecision(2) << openings[i] << " | " 
                      << std::setw(10) << std::fixed << std::setprecision(2) << closings[i] << std::endl;
        }
    }
};