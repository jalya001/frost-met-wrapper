namespace frost_met_wrapper {
namespace internal {
/*
constexpr ObservationsKeys keys[] = {
    {"sourceId", 8, 4, STRINGZ, true},
    {"referenceTime", 0, 8, DATEZ, true},
    {"elementId", 16, 8, DECIMALZ, true},
    {"value", 24, 8, DECIMALZ, true}, // There are multiple observations each source+time, but we only get the first
};

struct NameValueStore {
    const char** names;
    int* sums;
    int* counters;
}

void extract_from_json(const char* json, int header_count, StationProps* tseries) {
    std::cout << "There are " << header_count << " observations\n";
    const char* p = json;

    for (int i = 0; i < header_count; ++i) {
        find_key(p, STR_AND_LEN("header"));
        std::cout << "Found header\n";
        char* data_as_bytes = reinterpret_cast<char*>(&tseries[i]);

        extract_all(p, data_as_bytes);

        p += 10;
        
        std::cout << "Finished parsing header block\n\n";
    }

    std::cout << "Finished parsing all headers.\n";
}

DoubleMap* parse_observations(const char* json_ptr, const size_t max_size, int* out_count) {
    find_key(json_ptr, STR_AND_LEN("currentItemCount"));
    int observations_count;
    extract_value<INTEGERZ>(json_ptr; observations_count, 4);

    StationProps* tseries = (StationProps*) malloc(header_count * sizeof(StationProps));
    extract_from_json(json_ptr, observations_count, tseries);
    *out_count = header_count;
    return tseries;
}
*/
}
}