#include "fflerror.hpp"
#include "actormsg.hpp"
#include "dispatcher.hpp"
#include "server.hpp"
#include "delaydriver.hpp"

extern Server *g_server;
static thread_local bool t_delayThreadFlag = false; // use bool since only has 1 net thread

bool DelayDriver::isDelayThread()
{
    return t_delayThreadFlag;
}

DelayDriver::DelayDriver()
    : m_worker([this]()
      {
          t_delayThreadFlag = true;

          std::vector<std::pair<uint64_t, uint64_t>>   timeoutArgs;
          std::vector<std::pair<uint64_t, uint64_t>> cancelledArgs;

          while(true){
              timeoutArgs  .clear();
              cancelledArgs.clear();
              {
                  std::unique_lock lock(m_mutex);

                  if(m_timers.c.empty()){
                      m_cond.wait(lock, [this]()
                      {
                          return m_stopRequested || !m_timers.c.empty() || !m_cancelledTimerArgs.empty();
                      });
                  }
                  else{
                      m_cond.wait_until(lock, m_timers.c.begin()->first, [this]()
                      {
                          return m_stopRequested || !m_cancelledTimerArgs.empty();
                      });
                  }

                  if(m_stopRequested){
                      break;
                  }

                  for(const auto now = clock_type::now(); !m_timers.c.empty() && m_timers.c.begin()->first <= now;){
                      timeoutArgs.push_back(m_timers.c.begin()->second.cbArg);
                      m_timerIds.erase(m_timers.c.begin()->second.seqID);
                      m_timers.erase(m_timers.c.begin());
                  }

                  std::swap(cancelledArgs, m_cancelledTimerArgs);
              }

              for(const auto &cbArg: timeoutArgs){
                  onTimerCallback(cbArg, true);
              }

              for(const auto &cbArg: cancelledArgs){
                  onTimerCallback(cbArg, false);
              }
          }

          // pool has been stopped
          // outside can not add/cancel timers

          for(const auto &cbArg: m_cancelledTimerArgs){
              onTimerCallback(cbArg, false);
          }

          for(const auto &timer: m_timers.c){
              onTimerCallback(timer.second.cbArg, false);
          }
      })
{}

DelayDriver::~DelayDriver()
{
    try{
        forceStop();
    }
    catch(const std::exception &e){
        g_server->addFatal("DelayDriver::dtor: %s", e.what());
    }
    catch(...){
        g_server->addFatal("DelayDriver::dtor: unknown exception");
    }
}

uint64_t DelayDriver::addTimer(const std::pair<uint64_t, uint64_t> &cbArg, uint64_t msec)
{
    uint64_t seqID;
    bool needReschedule;

    const auto expireAt = clock_type::now() + std::chrono::milliseconds(msec);
    {
        const std::unique_lock lock(m_mutex);
        if(m_stopRequested){
            return 0;
        }

        if(m_seqID % 2){
            seqID = m_seqID = (m_seqID + 2);
        }
        else{
            seqID = m_seqID = (m_seqID + 1);
        }

        needReschedule = m_timers.c.empty() || (expireAt < m_timers.c.begin()->first);
        timer_map::iterator iter;

        if(m_timers.has_node()){
            iter = m_timers.node_insert([expireAt, seqID, &cbArg](auto &node)
            {
                node.key() = expireAt;
                node.mapped() = TimerEntry{seqID, cbArg};
            }).first;
        }
        else{
            iter = m_timers.alloc_insert([expireAt, seqID, &cbArg](auto &c)
            {
                return c.emplace(expireAt, TimerEntry{seqID, cbArg});
            });
        }

        if(m_timerIds.has_node()){
            m_timerIds.node_insert([seqID, &iter](auto &node)
            {
                node.key() = seqID;
                node.mapped() = iter;
            });
        }
        else{
            m_timerIds.alloc_insert([seqID, &iter](auto &c)
            {
                c.emplace(seqID, iter);
            });
        }
    }

    if(needReschedule){
        m_cond.notify_one();
    }
    return seqID;
}

uint64_t DelayDriver::addWaiter(const std::pair<uint64_t, uint64_t> &cbArg)
{
    uint64_t seqID;
    {
        const std::unique_lock lock(m_mutex);
        if(m_stopRequested){
            return 0;
        }

        if(m_seqID % 2){
            seqID = m_seqID = (m_seqID + 1);
        }
        else{
            seqID = m_seqID = (m_seqID + 2);
        }

        if(m_waiters.has_node()){
            m_waiters.node_insert([seqID, &cbArg](auto &node)
            {
                node.key() = seqID;
                node.mapped() = cbArg;
            });
        }
        else{
            m_waiters.alloc_insert([seqID, &cbArg](auto &c)
            {
                c.emplace(seqID, cbArg);
            });
        }
    }
    return seqID;
}

void DelayDriver::requestStop()
{
    fflassert(!isDelayThread());
    forceStop();
}

void DelayDriver::onTimerCallback(const std::pair<uint64_t, uint64_t> &cbArg, bool timeout)
{
    Dispatcher().post(cbArg, timeout ? AM_TIMEOUT : AM_CANCEL);
}

void DelayDriver::forceStop()
{
    {
        const std::unique_lock lock(m_mutex);
        if(m_stopRequested){
            return;
        }
        else{
            m_stopRequested = true;
        }
    }
    m_cond.notify_all();

    if(m_worker.joinable()){
        m_worker.join();
    }
}

std::optional<bool> DelayDriver::cancelTimer(uint64_t id, bool triggerInPlace)
{
    bool found = false;
    std::pair<uint64_t, uint64_t> cbArg;
    {
        const std::unique_lock lock(m_mutex);
        if(m_stopRequested){
            return std::nullopt;
        }

        if(auto p = m_timerIds.c.find(id); p != m_timerIds.c.end()){
            found = true;
            cbArg = p->second->second.cbArg;

            m_timers.erase(p->second);
            m_timerIds.erase(p);

            if(!triggerInPlace){
                m_cancelledTimerArgs.push_back(cbArg);
            }
        }
    }

    if(found){
        if(triggerInPlace){
            onTimerCallback(cbArg, false);
        }
        else{
            m_cond.notify_one();
        }
    }
    return found;
}

std::optional<bool> DelayDriver::cancelWaiter(uint64_t id, bool triggerInPlace)
{
    bool found = false;
    std::pair<uint64_t, uint64_t> cbArg;
    {
        const std::unique_lock lock(m_mutex);
        if(m_stopRequested){
            return std::nullopt;
        }

        if(auto p = m_waiters.c.find(id); p != m_waiters.c.end()){
            found = true;
            cbArg = p->second;

            m_waiters.erase(p);
            if(!triggerInPlace){
                m_cancelledTimerArgs.push_back(cbArg);
            }
        }
    }

    if(found){
        if(triggerInPlace){
            onTimerCallback(cbArg, false);
        }
        else{
            m_cond.notify_one();
        }
    }
    return found;
}
