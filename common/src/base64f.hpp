// from: https://github.com/tobiaslocker/base64
#pragma once
#include <algorithm>
#include <string>
#include <string_view>

namespace base64f
{
    namespace _details
    {
        inline constexpr std::string_view base64_chars =
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            "abcdefghijklmnopqrstuvwxyz"
            "0123456789+/";
    }

    inline std::string encode(const void *s, size_t size)
    {
        std::string result;

        int offset = 0;
        int counter = 0;
        uint32_t bit_stream = 0;

        result.reserve(((size + 2) / 3) * 4);
        for(size_t i = 0; i < size; ++i){
            const unsigned int c = static_cast<const uint8_t *>(s)[i];

            offset = 16 - (counter % 3) * 8;
            bit_stream += (c << offset);

            if(offset == 16){
                result.push_back(_details::base64_chars.at((bit_stream >> 18) & 0x3f));
            }

            if(offset == 8){
                result.push_back(_details::base64_chars.at((bit_stream >> 12) & 0x3f));
            }

            if(offset == 0 && counter != 3){
                result.push_back(_details::base64_chars.at((bit_stream >> 6) & 0x3f));
                result.push_back(_details::base64_chars.at((bit_stream >> 0) & 0x3f));
                bit_stream = 0;
            }
            counter++;
        }

        if(offset == 16){
            result.push_back(_details::base64_chars.at((bit_stream >> 12) & 0x3f));
            result.push_back('=');
            result.push_back('=');
        }

        if(offset == 8){
            result.push_back(_details::base64_chars.at((bit_stream >> 6) & 0x3f));
            result.push_back('=');
        }

        return result;
    }

    inline std::string decode(const void *data, size_t size)
    {
        if(size % 4 != 0){
            throw std::runtime_error("invalid base64 string length: " + std::to_string(size));
        }

        std::string result;

        int counter = 0;
        uint32_t bit_stream = 0;

        result.reserve((size / 4) * 3);
        for(size_t i = 0; i < size; ++i){
            const auto c = static_cast<const char *>(data)[i];
            if(const auto num_val = _details::base64_chars.find(c); num_val != std::string_view::npos){
                const int offset = 18 - (counter % 4) * 6;
                bit_stream += (num_val << offset);

                if(offset == 12){
                    result.push_back(static_cast<char>((bit_stream >> 16) & 0xff));
                }

                if(offset == 6){
                    result.push_back(static_cast<char>((bit_stream >> 8) & 0xff));
                }

                if(offset == 0 && counter != 4){
                    result.push_back(static_cast<char>(bit_stream & 0xff));
                    bit_stream = 0;
                }
            }

            else if (c != '='){
                throw std::runtime_error("invalid character in base64 string: " + std::to_string(static_cast<unsigned char>(c)));
            }

            counter++;
        }
        return result;
    }

    template<typename C> std::string encode(const C &data)
    {
        static_assert(sizeof(typename C::value_type) == 1);
        return encode(data.data(), data.size());
    }

    template<typename C> std::string decode(const C &data)
    {
        static_assert(sizeof(typename C::value_type) == 1);
        return decode(data.data(), data.size());
    }
}
