#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <array>
#include <unordered_map>

#include "frost_utils.hpp"

namespace frost_met_wrapper {
namespace internal {

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
    const char* start = p;
    while (*p && *p != '"' && *p != '\n') ++p;
    --p;
    const char* end = p;
    int this_len = end - start;
    if constexpr (Mode == STRINGZ || Mode == DATEZ) {
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

template <const auto& Keys, std::size_t I = 0> // Would be nice with a static assert for right keys type
void extract_all(const char*& p, char* data_as_bytes) { // Maybe this adds too much to the compiletime...
    if constexpr (I < Keys.size()) {
        constexpr auto& k = Keys[I];
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
                LOG(k.key << " = ");
                if constexpr (k.mode == INTEGERZ) { 
                    LOG(*reinterpret_cast<const int*>(data_as_bytes + k.index));
                } else if (k.mode == DECIMALZ) {
                    LOG(*reinterpret_cast<const double*>(data_as_bytes + k.index));
                } else if (k.mode == DATEZ) {
                    LOGB(data_as_bytes + k.index, k.len); // Temporary
                } else {
                    LOGB(data_as_bytes + k.index, k.len);
                }
                LOG("\n");
            } else {
                if constexpr (k.mode == DATEZ) {
                    std::strcpy(data_as_bytes + k.index, "9999-01-01T00:00:00Z");
                } else {
                    for (int i = 0; i < k.len; ++i) {
                        *(data_as_bytes + k.index + i) = 0;
                    }
                }
                LOG(k.key << " = ");
                LOGB(data_as_bytes + k.index, k.len); // Temporary
                LOG("\n");
            }
        } else {
            LOG("Milepost: " << k.key << "\n");
        }
        extract_all<Keys, I + 1>(p, data_as_bytes);
    }
}

}
}
