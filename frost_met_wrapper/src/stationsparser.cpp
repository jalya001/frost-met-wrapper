#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <array>

#include "frost_utils.hpp"
#include "frost_templates.hpp"

namespace frost_met_wrapper {
namespace internal {

int count_headers(const char* json, size_t size) {
    const char* p = json;
    int header_count = 0;
    while (static_cast<size_t>(p - json) < size-1600) {
        find_key(p, STR_AND_LEN("header"));
        std::cout << "Found header block #" << ++header_count << "\n";
        p += 1600;
    }
    std::cout << "Headers counter: " << header_count << "\n\n";
    return header_count;
}

constexpr std::array<KeyInfo, 9> keys{{
    {"parameterid", 8, 4, STRINGZ, true},
    {"stationid", 0, 8, STRINGZ, true},
    {"value", 0, 0, MILEPOST, true},
    {"elevation(masl/hs)", 12, 4, INTEGERZ, true},
    {"latitude", 16, 8, DECIMALZ, true},
    {"longitude", 24, 8, DECIMALZ, true},
    {"available", 0, 0, MILEPOST, true},
    {"from", 32, 16, DATEZ, true},
    {"to", 48, 16, DATEZ, false}
}};

void extract_from_json(const char* json, int header_count, StationProps* tseries) {
    std::cout << "There are " << header_count << " headers\n";
    const char* p = json;

    for (int i = 0; i < header_count; ++i) {
        find_key(p, STR_AND_LEN("header"));
        std::cout << "Found header\n";
        char* data_as_bytes = reinterpret_cast<char*>(&tseries[i]);

        extract_all<keys>(p, data_as_bytes);

        p += 10;
        
        std::cout << "Finished parsing header block\n\n";
    }

    std::cout << "Finished parsing all headers.\n";
}

StationProps* parse_stations(const char* json_ptr, const size_t max_size, int* out_count) {
    int header_count = count_headers(json_ptr, max_size);
    StationProps* tseries = (StationProps*) malloc(header_count * sizeof(StationProps));
    extract_from_json(json_ptr, header_count, tseries);
    *out_count = header_count;
    return tseries;
}

}
}