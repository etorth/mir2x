#pragma once
#include <vector>
#include <cstdint>
#include <cstddef>
#include <initializer_list>

class Rawbuf final
{
    private:
        std::vector<uint8_t> m_data;

    public:
        Rawbuf(std::initializer_list<uint8_t>);

    public:
        ~Rawbuf() = default;

    public:
        const uint8_t *data() const
        {
            return (size() == 0) ? nullptr : m_data.data();
        }

    public:
        size_t size() const
        {
            return m_data.size();
        }

    public:
        static std::vector<uint8_t> buildBuf(const char *);

    public:
        static void buildBinFile(const char *, const char *);
        static void buildHexFile(const char *, const char *, size_t);
};
