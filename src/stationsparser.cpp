#include <iostream>
#include <string>
#include <cstring>
#include <cmath>

#include "frost_utils.h"

constexpr std::size_t strlen_constexpr(const char* str) {
    std::size_t len = 0;
    while (str[len] != '\0') ++len;
    return len;
}

#define STR_AND_LEN(str) str, strlen_constexpr(str)

enum ExtractionMode {
    MILEPOST = 0,
    STRINGZ = 1,
    INTEGERZ = 2,
    DECIMALZ = 3,
    DATEZ = 4
};

template <typename T>
T froststring_to_num(const char*& p) {
    while (*p == ' ' || *p == '"') ++p;
    int sign = 1;
    if (*p == '+' || *p == '-') {
        if (*p == '-') sign = -1;
        ++p;
    }

    T result = 0;
    while (*p >= '0' && *p <= '9') {
        int digit = *p - '0';
        result = result * 10 + digit;
        ++p;
    }

    if constexpr (std::is_same_v<T, double>) {
        if (*p == '.') {
            ++p;
            double frac_factor = 0.1;
            while (*p >= '0' && *p <= '9') {
                int digit = *p - '0';
                result += digit * frac_factor;
                frac_factor *= 0.1;
                ++p;
            }
        }
    }
    return sign * result;
}

template <ExtractionMode Mode = STRINGZ>
void extract_value(const char*& p, char* out, int out_len) {
    while (*p == ' ' || *p == '\n' || *p == '\t' || *p == '\r' || *p == '"') ++p;
    if constexpr (Mode == DATEZ) {
        for (int i = 0; i < out_len; ++p) {
            if (*p != '-' && *p != ':') {
                out[i] = *p;
                ++i;
            }
        }
    } else {
        const char* start = p;
        while (*p && *p != '"' && *p != '\n') ++p;
        --p;
        const char* end = p;
        int this_len = end - start;
        if constexpr (Mode == STRINGZ) {
            for (int i = 0; i < out_len; i++) {
                out[i] = (i < this_len) ? start[i] : '\0';
            }
        } else if (Mode == INTEGERZ) {
            int value = froststring_to_num<int>(start);
            *((int*) out) = value;
        } else {
            double value = froststring_to_num<double>(start);
            *((double*) out) = value;
        }
    }
}

template <char Sentinel = '\0'>
bool find_key(const char*& p, const char* key, int len) {
    while (*p) {
        while (*p != '"') {
            ++p;
            if constexpr (Sentinel != '\0') {
                if (*p == Sentinel) return false;
            }
        }
        ++p;
        if (std::strncmp(p, key, len) == 0) {
            p += len + 1;
            return true;
        }
        p += len + 1;
    }
    return false;
}

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

struct KeyInfo {
    const char* key;
    const int index;
    const int len;
    const ExtractionMode mode;
    const bool always_there;
};

constexpr KeyInfo keys[] = {
    {"parameterid", 8, 4, STRINGZ, true},
    {"stationid", 0, 8, STRINGZ, true},
    {"value", 0, 0, MILEPOST, true},
    {"elevation(masl/hs)", 12, 4, INTEGERZ, true},
    {"latitude", 16, 8, DECIMALZ, true},
    {"longitude", 24, 8, DECIMALZ, true},
    {"available", 0, 0, MILEPOST, true},
    {"from", 32, 16, DATEZ, true},
    {"to", 48, 16, DATEZ, false}
};

template <std::size_t I = 0>
void extract_all(const char*& p, char* data_as_bytes) {
    if constexpr (I < (sizeof(keys) / sizeof(keys[0]))) {
        constexpr auto& k = keys[I];
        bool is_key;
        if constexpr (k.always_there == true) {
            is_key = find_key(p, STR_AND_LEN(k.key));
        } else {
            is_key = find_key<'}'>(p, STR_AND_LEN(k.key));
        }
        p += 2;
        if constexpr (k.mode != MILEPOST) {
            if (is_key) {
                extract_value<k.mode>(p, data_as_bytes + k.index, k.len);
                std::cout << k.key << " = ";
                if constexpr (k.mode == INTEGERZ) {
                    std::cout << *reinterpret_cast<const int*>(data_as_bytes + k.index);
                } else if (k.mode == DECIMALZ) {
                    std::cout << *reinterpret_cast<const double*>(data_as_bytes + k.index);
                } else if (k.mode == DATEZ) {
                    std::cout.write(data_as_bytes + k.index, k.len); // Temporary
                } else {
                    std::cout.write(data_as_bytes + k.index, k.len);
                }
                std::cout << "\n";
            } else {
                if constexpr (k.mode == DATEZ) {
                    std::strcpy(data_as_bytes + k.index, "99990101T000000Z");
                } else {
                    for (int i = 0; i < k.len; ++i) {
                        *(data_as_bytes + k.index + i) = 0;
                    }
                }
                std::cout << k.key << " = ";
                std::cout.write(data_as_bytes + k.index, k.len); // Temporary
                std::cout << "\n";
            }
        } else {
            std::cout << "Milepost: " << k.key << "\n";
        }
        extract_all<I + 1>(p, data_as_bytes);
    }
}

void extract_from_json(const char* json, int header_count, StationProps* tseries) {
    std::cout << "There are " << header_count << " headers\n";
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

StationProps* parse_stations(const char* json_ptr, const size_t max_size, int* out_count) {
    int header_count = count_headers(json_ptr, max_size);
    StationProps* tseries = (StationProps*) malloc(header_count * sizeof(StationProps));
    extract_from_json(json_ptr, header_count, tseries);
    *out_count = header_count;
    return tseries;
}