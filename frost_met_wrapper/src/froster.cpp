#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

#include "frost_utils.hpp"
#include "frost_templates.hpp"

namespace frost_met_wrapper {
using namespace internal;

void printStations(const StationProps* stations, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        const StationProps& s = stations[i];

        std::cout << "Station " << i + 1 << ":\n";
        std::cout << "  Station ID:     " << std::string(s.stationid, 8) << "\n";
        std::cout << "  Parameter ID:   " << std::string(s.parameterid, 4) << "\n";
        std::cout << "  Elevation:      " << s.elevation << "\n";
        std::cout << "  Latitude:       " << s.latitude << "\n";
        std::cout << "  Longitude:      " << s.longitude << "\n";
        std::cout << "  Available From: " << std::string(s.available_from, 16) << "\n";
        std::cout << "  Available To:   " << std::string(s.available_to, 16) << "\n";
        std::cout << "-------------------------\n";
    }
}

int get_stations() {
    return 1;
}

int approximate(StationProps* input = nullptr) {
    if /*constexpr*/ (input == nullptr) {
        std::string stations_url = "http://localhost:8080/stations";
        std::string observations_url = "http://localhost:8080/observations";
        char* stations_json_ptr;
        size_t stations_bytes = http_get_response(stations_url, &stations_json_ptr);
        int stations_count;
        StationProps* tseries = parse_stations(stations_json_ptr, stations_bytes, &stations_count);
    
        printStations(tseries, stations_count);
    }
    return 0;
}

}