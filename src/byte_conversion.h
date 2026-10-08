#ifndef BYTE_CONVERSION_H
#define BYTE_CONVERSION_H

#include <Arduino.h>
#include <cstdint>
#include <cstring>

namespace byte_conversion {

inline void int_2_bytes(int32_t val, uint8_t* bytes) {
    bytes[0] = (val >> 24) & 0xFF;
    bytes[1] = (val >> 16) & 0xFF;
    bytes[2] = (val >> 8) & 0xFF;
    bytes[3] = val & 0xFF;
}

inline int32_t bytes_2_int(const uint8_t* bytes) {
    return ((int32_t)bytes[0] << 24) |
           ((int32_t)bytes[1] << 16) |
           ((int32_t)bytes[2] << 8)  |
            (int32_t)bytes[3];
}

inline void float_2_bytes(float val, uint8_t* bytes) {
    uint32_t raw;
    std::memcpy(&raw, &val, sizeof(float));
    bytes[0] = (raw >> 24) & 0xFF;
    bytes[1] = (raw >> 16) & 0xFF;
    bytes[2] = (raw >> 8) & 0xFF;
    bytes[3] = raw & 0xFF;
}

inline float bytes_2_float(const uint8_t* bytes) {
    uint32_t raw = ((uint32_t)bytes[0] << 24) |
                   ((uint32_t)bytes[1] << 16) |
                   ((uint32_t)bytes[2] << 8)  |
                    (uint32_t)bytes[3];
    float val;
    std::memcpy(&val, &raw, sizeof(float));
    return val;
}

// Packs up to 32 booleans into up to 4 bytes
inline uint8_t bool_array_to_bytes(const bool* bool_arr, uint16_t count, uint8_t* out_bytes) {
    uint16_t limit = count < 32 ? count : 32;
    uint8_t byte_count = (limit + 7) / 8;

    memset(out_bytes, 0, byte_count);

    for (uint16_t i = 0; i < limit; i++) {
        if (bool_arr[i]) {
            uint8_t byte_idx = i / 8;
            uint8_t bit_idx  = i % 8;
            out_bytes[byte_idx] |= (1 << bit_idx);
        }
    }
    return byte_count;
}

// Unpacks up to 4 bytes back into up to 32 booleans
inline void bytes_to_bool_array(const uint8_t* in_bytes, bool* bool_arr, uint16_t count) {
    uint16_t limit = count < 32 ? count : 32;

    for (uint16_t i = 0; i < limit; i++) {
        uint8_t byte_idx = i / 8;
        uint8_t bit_idx  = i % 8;
        bool_arr[i] = (in_bytes[byte_idx] & (1 << bit_idx)) != 0;
    }
}

} // namespace byte_conversion

#endif // BYTE_CONVERSION_H
