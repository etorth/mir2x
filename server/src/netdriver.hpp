#pragma once
#include <asio.hpp>
#include <atomic>
#include <thread>
#include <cstdint>
#include "uidf.hpp"
#include "channel.hpp"
#include "fflerror.hpp"
#include "sysconst.hpp"
#include "monoserver.hpp"
#include "cachequeue.hpp"
#include "dispatcher.hpp"
#include "actormsgpack.hpp"

class Channel;
class NetDriver final
{
    private:
        friend class Channel;

    private:
        unsigned int m_port = 0;

    private:
        asio::io_service        *m_io       = nullptr;
        asio::ip::tcp::endpoint *m_endPoint = nullptr;
        asio::ip::tcp::acceptor *m_acceptor = nullptr;

    private:
        std::thread m_thread;

    private:
        std::queue<uint32_t> m_channIDQ;
        std::vector<std::shared_ptr<Channel>> m_channList;

    public:
        NetDriver();

    public:
        ~NetDriver();

    private:
        void recycle(uint32_t channID)
        {
            m_channIDQ.push(channID);
        }

    public:
        bool isNetThread() const;

    public:
        void launch(uint32_t);

    public:
        void post(uint32_t channID, uint8_t headCode, const void *buf, size_t bufLen)
        {
            fflassert(to_uz(channID) > 0);
            fflassert(to_uz(channID) < m_channList.size());
            fflassert(ServerMsg(headCode).checkData(buf, bufLen));
            m_channList.at(channID)->post(headCode, buf, bufLen);
        }

        void bindPlayer(uint32_t channID, uint64_t uid)
        {
            fflassert(to_uz(channID) > 0);
            fflassert(to_uz(channID) < m_channList.size());

            fflassert(uidf::isPlayer(uid));
            m_channList.at(channID)->bindPlayer(uid);
        }

        void shutdown(uint32_t channID, bool force)
        {
            fflassert(to_uz(channID) > 0);
            fflassert(to_uz(channID) < m_channList.size());

            m_channList.at(channID)->shutdown(force);
            m_channList.at(channID).reset();
        }

    private:
        void acceptNewConnection();

    private:
        void release()
        {
            if(m_io){
                m_io->stop();
            }

            if(m_thread.joinable()){
                m_thread.join();
            }

            delete m_acceptor;
            delete m_endPoint;
            delete m_io;

            m_acceptor = nullptr;
            m_endPoint = nullptr;
            m_io       = nullptr;
        }
};
