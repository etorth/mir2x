#include <cstdint>
#include <concepts>
#include <asio.hpp>
#include "msgf.hpp"
#include "fflerror.hpp"

namespace asiof
{
    template<std::unsigned_integral T> asio::awaitable<T> readVLInteger(asio::ip::tcp::socket &sock)
    {
        uint8_t buf[16];
        size_t offset = 0;

        for(size_t offset = 0; offset < (sizeof(T) * 8 + 6) / 7; ++offset){
            co_await asio::async_read(sock, asio::buffer(buf + offset, 1), asio::deferred);
            if(!(buf[offset] & 0x80)){
                co_return msgf::decodeLength<T>(buf, offset + 1);
            }
        }
        throw fflerror("variant packet size uses more than %zu bytes", offset);
    }
}
