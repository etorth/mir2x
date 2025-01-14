#pragma once
#include <string>
#include <cstdint>
#include <cstddef>

namespace aesf
{
    class AES
    {
        private:
#define AES_storage_alignment 16
#define AES_storage_size     192
            alignas(AES_storage_alignment) std::byte storage[AES_storage_size];

        public:
            AES(const char *, uint64_t, uint64_t);

        public:
            void encrypt(void *, size_t);
            void decrypt(void *, size_t);

        public:
            template<typename T> void encrypt(T &t) { encrypt(std::data(t), std::size(t)); }
            template<typename T> void decrypt(T &t) { decrypt(std::data(t), std::size(t)); }
    };
}
