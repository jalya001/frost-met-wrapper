#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <array>
#include <unordered_map>

#include "frost_utils.hpp"
#include "frost_templates.hpp"

namespace frost_met_wrapper {
namespace internal {

int count_headers(const char* json, size_t size) {
    const char* p = json;
    int header_count = 0;
    while (static_cast<size_t>(p - json) < size-1600) {
        find_key(p, STR_AND_LEN("header"));
        ++header_count;
        LOG("Found header block #" << header_count << "\n");
        p += 1600;
    }
    LOG("Headers counter: " << header_count << "\n\n");
    return header_count;
}

constexpr std::array<KeyInfo, 8> keys{{
    {"stationid", 0, 4, STRINGZ, true},
    {"value", 0, 0, MILEPOST, true},
    {"elevation(masl/hs)", 4, 4, INTEGERZ, true},
    {"latitude", 8, 8, DECIMALZ, true},
    {"longitude", 16, 8, DECIMALZ, true},
    {"available", 0, 0, MILEPOST, true},
    {"from", 24, 20, DATEZ, true},
    {"to", 44, 20, DATEZ, false}
}};

void extract_from_json(const char* json, int header_count, StationProps* tseries) {
    LOG("There are " << header_count << " headers\n");
    const char* p = json;

    for (int i = 0; i < header_count; ++i) {
        find_key(p, STR_AND_LEN("header"));
        LOG("Found header\n");
        char* data_as_bytes = reinterpret_cast<char*>(&tseries[i]);

        extract_all<keys>(p, data_as_bytes);

        p += 10;
        
        LOG("Finished parsing header block\n\n");
    }

    LOG("Finished parsing all headers.\n");
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