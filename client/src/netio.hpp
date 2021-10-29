#pragma once
#include <cstdint>
#include <asio.hpp>

class NetIO final
{
    private:
        asio::io_service        m_io;
        asio::ip::tcp::resolver m_resolver;
        asio::ip::tcp::socket   m_socket;

    private:
        uint8_t m_readHeadCode = 0;
        uint8_t m_readLen[4] = {0, 0, 0, 0};

    private:
        std::vector<uint8_t> m_currSendBuf;
        std::vector<uint8_t> m_nextSendBuf;

    private:
        std::vector<uint8_t> m_readBuf;

    private:
        std::function<void(uint8_t, const uint8_t *, size_t)> m_msgHandler;

    public:
        NetIO()
            : m_io()
            , m_resolver(m_io)
            , m_socket  (m_io)
        {}

    public:
        ~NetIO()
        {
            m_io.stop();
        }

    public:
        void start(const char *, const char *, std::function<void(uint8_t, const uint8_t *, size_t)>);

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

    public:
        void send(uint8_t, const uint8_t *, size_t);

    private:
        void doReadHeadCode();
        void doReadBody(size_t, size_t);

    private:
        void doSendBuf();

    private:
        void shutdown()
        {
            m_io.stop();
        }
};
