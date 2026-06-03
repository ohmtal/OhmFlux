//------------------------------------------------------------------------------
// compact base64 encode decode
//------------------------------------------------------------------------------
#pragma once

// #include <string>
// #include <vector>
// #include <cstdint>

namespace ByteEncoder {
    struct Base64 {
        static inline const char* lut = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        // ... encode
        std::string encode(const unsigned char* data, uint32_t len);

        // ... encode with string stream
        std::string encode(const std::stringstream& stream);

        // ... decode
        uint32_t decode(const std::string& input, unsigned char* output);

        // ... encode with string stream
        bool decode(const std::string& input, std::stringstream& output);
    }; //Base64



}; //namespace
