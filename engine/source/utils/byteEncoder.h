//------------------------------------------------------------------------------
// compact base64 encode decode
//------------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace ByteEncoder {


    struct Base64 {
        static inline const char* lut = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        // ... encode
        std::string encode(const unsigned char* data, uint32_t len) {
            std::string out;
            out.reserve(((len + 2) / 3) * 4);
            for (uint32_t i = 0; i < len; i += 3) {
                uint32_t val = (data[i] << 16) | (i + 1 < len ? data[i + 1] << 8 : 0) | (i + 2 < len ? data[i + 2] : 0);
                out.push_back(lut[(val >> 18) & 0x3F]);
                out.push_back(lut[(val >> 12) & 0x3F]);
                out.push_back(i + 1 < len ? lut[(val >> 6) & 0x3F] : '=');
                out.push_back(i + 2 < len ? lut[val & 0x3F] : '=');
            }
            return out;
        }

        // ... encode with string stream
        std::string encode(const std::stringstream& stream) {
            std::string buffer = stream.str(); // Inhalt als String kopieren
            return encode(reinterpret_cast<const unsigned char*>(buffer.data()),
                          static_cast<uint32_t>(buffer.size()));
        }

        // ... decode
        uint32_t decode(const std::string& input, unsigned char* output) {
            static const std::vector<int> rev_lut = [](){
                std::vector<int> v(256, -1);
                for (int i = 0; i < 64; i++) v[lut[i]] = i;
                return v;
            }();

            uint32_t val = 0, count = 0, bits = 0;
            for (unsigned char c : input) {
                if (rev_lut[c] == -1) break;
                val = (val << 6) | rev_lut[c];
                bits += 6;
                if (bits >= 8) {
                    output[count++] = (val >> (bits -= 8)) & 0xFF;
                }
            }
            return count;
        }

        // ... encode with string stream
        bool decode(const std::string& input, std::stringstream& output) {
            // validate len
            if (input.empty() || (input.length() % 4 != 0)) {
                return false;
            }
            std::vector<unsigned char> buffer(input.size());
            uint32_t actualLen = decode(input, buffer.data());

            if (actualLen > 0) {
                output.write(reinterpret_cast<const char*>(buffer.data()), actualLen);
                output.seekg(0);
            }
            return true;
        }


    }; //Base64



}; //namespace
