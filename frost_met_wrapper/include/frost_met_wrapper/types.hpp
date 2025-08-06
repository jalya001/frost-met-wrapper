#pragma once

namespace frost_met_wrapper {

// Why not use a pointer to location? There generally aren't enough duplicates for me to care
// And with 64 byte segments, it probably wouldn't save any space anyways
// Pretty sure an AoS is fine here?
struct StationProps { // 64 bytes
    char stationid[8];
    char parameterid[4];
    int elevation; // 4 bytes
    double latitude; // 8 bytes
    double longitude; // 8 bytes
    char available_from[16];
    char available_to[16];
};

}