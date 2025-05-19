#include <cinttypes>
#include <chrono>
#include "fflerror.hpp"
#include "server.hpp"
#include "actormsg.hpp"
#include "delaydriver.hpp"
#include "dispatcher.hpp"

extern Server *g_server;
static thread_local bool t_delayThreadFlag = false; // use bool since only has 1 net thread

DelayDriver::DelayDriver()
    : m_context(std::make_unique<asio::io_context>(1))
    , m_holder(*m_context, std::chrono::steady_clock::time_point::max())
{
    fflassert(!isDelayThread());
    m_holder.async_wait([this](std::error_code ec)
    {
        if(ec){
            if(ec == asio::error::operation_aborted){

            }
            else{
                throw std::system_error(ec);
            }
        }
        else{
            throw fflerror("unexpected timeout");
        }
    });

    try{
        m_thread = std::thread([this]()
        {
            t_delayThreadFlag = true;
            m_context->run();
            g_server->addLog(LOGTYPE_INFO, "Delay driver thread has exited");
        });
    }
    catch(const std::exception &e){
        throw fflerror("failed to launch delay driver: %s", e.what());
    }
    catch(...){
        throw fflerror("failed to launch delay driver: unknown error");
    }
}

DelayDriver::~DelayDriver()
{
    try{
        m_holder.cancel();
        if(m_context){
            m_context->stop();
        }

        if(m_thread.joinable()){
            m_thread.join();
        }
    }
    catch(const std::exception &e){
        g_server->addLog(LOGTYPE_WARNING, "Failed when release delay driver: %s", e.what());
    }
    catch(...){
        g_server->addLog(LOGTYPE_WARNING, "Failed when release delay driver: unknown error");
    }
}

bool DelayDriver::isDelayThread()
{
    return t_delayThreadFlag;
}

uint64_t DelayDriver::add(const std::pair<uint64_t, uint64_t> &fromAddr, uint64_t tick)
{
    uint64_t seq;
    timer_map::iterator iter;
    {
        const std::lock_guard lockGuard(m_lock);
        seq = m_seqID++;

        if(m_nodeList.empty()){
            iter = m_timerList.emplace(seq, asio::steady_timer(*m_context)).first;
        }
        else{
            timer_node node = std::move(m_nodeList.back());
            m_nodeList.pop_back();

            node.key() = seq;
            iter = m_timerList.insert(std::move(node)).position;
        }
    }

    iter->second.expires_after(std::chrono::milliseconds(tick));
    iter->second.async_wait([fromAddr, seq, this](std::error_code ec)
    {
        if(ec){
            if(ec == asio::error::operation_aborted){
                Dispatcher().post(fromAddr, AM_CANCEL);
                recycleTimer(seq);
            }
            else{
                throw std::system_error(ec);
            }
        }
        else{
            Dispatcher().post(fromAddr, AM_TIMEOUT);
            recycleTimer(seq);
        }
    });
    return seq;
}

bool DelayDriver::remove(uint64_t seq)
{
    if(seq){
        const std::lock_guard lockGuard(m_lock);
        if(auto p = m_timerList.find(seq); p != m_timerList.end()){
            p->second.cancel();
            return true;
        }
    }
    return false;
}

void DelayDriver::recycleTimer(uint64_t seq)
{
    const std::lock_guard lockGuard(m_lock);
    if(auto p = m_timerList.find(seq); p != m_timerList.end()){
        m_nodeList.push_back(m_timerList.extract(p));
    }
}
