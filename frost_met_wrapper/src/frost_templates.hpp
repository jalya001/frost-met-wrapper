#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <cmath>
#include <array>

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
    if constexpr (Mode == DATEZ) {
        for (int i = 0; i < out_len; ++p) { // Needs to skip .000 straight to the Z if there is a .
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

template <const auto& Keys, std::size_t I = 0>
void extract_all(const char*& p, char* data_as_bytes) {
    static_assert(
        std::is_same_v<std::remove_cvref_t<decltype(Keys)>,
        std::array<KeyInfo, std::remove_cvref_t<decltype(Keys)>.size()>>,
        "Keys must be a std::array<KeyInfo, N>, because idk how to template discrete N for that"
    );
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
        extract_all<Keys, I + 1>(p, data_as_bytes);
    }
}

}
}
