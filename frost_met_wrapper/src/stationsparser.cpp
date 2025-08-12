#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <array>
#include <unordered_map>
#include <bitset>

#include "frost_utils.hpp"
#include "frost_templates.hpp"

namespace frost_met_wrapper {
namespace internal {

const int MAX_STATIONID = 100000;

inline int get_stationid_exp(const char*& p) {
    find_key(p, STR_AND_LEN("stationid"));
    char key[4];
    extract_value<INTEGERZ>(p, key, 4);
    return *reinterpret_cast<int*>(key);
}

int count_unique_headers(const char* json, size_t size, int* header_count, std::bitset<MAX_STATIONID>& stationid_seen) {
    const char* p = json;
    int station_count = 0;

    while (static_cast<size_t>(p - json) < size-1400) {
        int key = get_stationid_exp(p);

        if (key > MAX_STATIONID) {
            std::cout << "Stationnumber too big error. This shouldn't ever happen";
            return -1;
        } else {
            if (stationid_seen.test(key)) {
                LOG("Found a repeat header with key " << key << "\n");
            } else {
                stationid_seen.set(key);
                ++station_count;
                LOG("Found novel header block #" << station_count << " with key " << key << "\n");
            }
        }
        ++(*header_count);
        p += 1400;
    }
    LOG("Unique headers counter: " << station_count << "\nNot unique headers counter: " << *header_count << "\n\n");
    return station_count;
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

void extract_from_json(const char* json, int header_count, StationProps* tseries_buffer, std::unordered_map<int, StationProps*> tseries_map, std::bitset<MAX_STATIONID>& stationid_seen) {
    LOG("There are " << header_count << " headers\n");
    const char* p = json;
    int station_counter = 0;

    for (int i = 0; i < header_count; ++i) {
        int key = get_stationid_exp(p);
        p -= 100; // TEMPORARY WORKAROUND
        LOG("Station " << key << " found.\n");
        if (stationid_seen.test(key)) {
            LOG("Novel station, initiating parse.\n");
            stationid_seen.reset(key);
            char* data_as_bytes = reinterpret_cast<char*>(&tseries_buffer[station_counter]);
            extract_all<keys>(p, data_as_bytes);
            tseries_map[key] = &tseries_buffer[station_counter];
            p += 10;
            LOG("Finished parsing header block\n\n");
            ++station_counter;
        } else {
            LOG("Station has been parsed already, skipping.\n\n");
            p += 1400;
        }
    }

    LOG("Finished parsing all headers.\n");
}

StationProps* parse_stations(const char* json_ptr, const size_t max_size, int* out_count) {
    std::bitset<MAX_STATIONID> stationid_seen;
    int header_count = 0;

    int station_count = count_unique_headers(json_ptr, max_size, &header_count, stationid_seen);
    if (station_count == -1) return nullptr;
    std::unordered_map<int, StationProps*> tseries_map;
    // It's a rather big flaw not to use mph here, but no libs I found were suitable
    tseries_map.reserve(station_count);
    StationProps* tseries_buffer = (StationProps*) malloc(station_count * sizeof(StationProps));
    extract_from_json(json_ptr, header_count, tseries_buffer, tseries_map, stationid_seen);
    *out_count = station_count;
    return tseries_buffer;
}

}
}