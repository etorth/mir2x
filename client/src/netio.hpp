#pragma once
#include <cstdint>
#include <concepts>
#include <functional>
#include <asio.hpp>
#include "servermsg.hpp"

class NetIO final
{
    private:
        asio::io_service        m_io;
        asio::ip::tcp::resolver m_resolver;
        asio::ip::tcp::socket   m_socket;

    private:
        uint8_t m_readSBuf[64];
        std::vector<uint8_t> m_readDBuf;

    private:
        std::vector<uint8_t> m_currSendBuf;
        std::vector<uint8_t> m_nextSendBuf;

    private:
        std::optional<uint64_t> m_respIDOpt;

    private:
        ServerMsg m_serverMsgBuf;
        const ServerMsg *m_serverMsg = nullptr;

    private:
        std::function<void(uint8_t, const uint8_t *, size_t, uint64_t)> m_msgHandler;

    public:
        NetIO()
            : m_io()
            , m_resolver(m_io)
            , m_socket  (m_io)
            , m_serverMsgBuf(SM_NONE_0)
        {}

    public:
        ~NetIO()
        {
            m_io.stop();
        }

    public:
        void start(const char *, const char *, std::function<void(uint8_t, const uint8_t *, size_t, uint64_t)>);

    public:
        void poll()
        {
            m_io.poll();
        }

        void stop()
        {
            m_io.post([this]()
            {
                shutdown();
            });
        }

    private:
        void doSendBuf();

    public:
        void send(uint8_t, const uint8_t *, size_t, uint64_t);

    private:
        void afterReadPacketHeadCode();

    private:
        void doReadPacketHeadCode();
        void doReadPacketResp();

        void doReadPacketBodySize();
        void doReadPacketBody(size_t, size_t);

    private:
        void shutdown()
        {
            m_io.stop();
        }

    private:
        void prepareServerMsg(uint8_t headCode)
        {
            m_serverMsgBuf.~ServerMsg();
            m_serverMsg = new (&m_serverMsgBuf) ServerMsg(headCode);
        }

    private:
        template<std::unsigned_integral T, std::invocable<T> F> void doReadVLInteger(size_t offset, F fnOp)
        {
            asio::async_read(m_socket, asio::buffer(m_readSBuf + offset, 1), [offset, fnOp, this](std::error_code ec, size_t)
            {
                if(ec){
                    throw fflerror("network error: %s", ec.message().c_str());
                }

                if(m_readSBuf[offset] & 0x80){
                    if(offset + 1 >= (sizeof(T) * 8 + 6) / 7){
                        throw fflerror("variant packet size uses more than %zu bytes", offset + 1);
                    }
                    else{
                        doReadVLInteger<T>(offset + 1, fnOp);
                    }
                }
                else{
                    fnOp(msgf::decodeLength<T>(m_readSBuf, offset + 1));
                }
            });
        }
};
