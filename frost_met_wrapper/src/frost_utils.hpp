#pragma once

#include "frost_met_wrapper/types.hpp"

namespace frost_met_wrapper {
namespace internal {

enum ExtractionMode {
    MILEPOST = 0,
    STRINGZ = 1,
    INTEGERZ = 2,
    DECIMALZ = 3,
    DATEZ = 4
};

constexpr std::size_t strlen_constexpr(const char* str) {
    std::size_t len = 0;
    while (str[len] != '\0') ++len;
    return len;
}

#define STR_AND_LEN(_str_input) _str_input, strlen_constexpr(_str_input)

StationProps* parse_stations(const char* json_ptr, const size_t max_size, int* out_count);

size_t http_get_response(const std::string& url, char** out_json_ptr);

struct KeyInfo {
    const char* key;
    const int index;
    const int len;
    const ExtractionMode mode;
    const bool always_there;
};

#ifdef DEBUG
    #define LOG(_data_input) do { std::cout << _data_input; } while(0)
    #define LOGB(_data_ptr, _byte_len) do { std::cout.write((_data_ptr), (_byte_len)); } while(0)
#else
    #define LOG(_data_input) do {} while(0)
    #define LOGB(_data_ptr, _byte_len) do {} while(0)
#endif

}
}
