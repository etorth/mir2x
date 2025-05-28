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

                  if(m_timers.empty()){
                      m_cond.wait(lock, [this]()
                      {
                          return m_stopRequested || !m_timers.empty() || !m_cancelledTimerArgs.empty();
                      });
                  }
                  else{
                      m_cond.wait_until(lock, m_timers.begin()->first, [this]()
                      {
                          return m_stopRequested || !m_cancelledTimerArgs.empty();
                      });
                  }

                  if(m_stopRequested){
                      break;
                  }

                  for(const auto now = clock_type::now(); !m_timers.empty() && m_timers.begin()->first <= now;){
                      timeoutArgs.push_back(m_timers.begin()->second.cbArg);
                      m_timerNodes.push_back(std::move(m_timers.extract(m_timers.begin())));
                      m_timerIdNodes.push_back(std::move(m_timerIds.extract(m_timerNodes.back().mapped().seqID)));
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

          for(const auto &timer: m_timers){
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

        needReschedule = m_timers.empty() || (expireAt < m_timers.begin()->first);

        timer_map::iterator iter;
        if(m_timerNodes.empty()){
            iter = m_timers.emplace(expireAt, TimerEntry{seqID, cbArg});
        }
        else{
            m_timerNodes.back().key() = expireAt;
            m_timerNodes.back().mapped() = TimerEntry{seqID, cbArg};

            iter = m_timers.insert(std::move(m_timerNodes.back()));
            m_timerNodes.pop_back();
        }

        if(m_timerIdNodes.empty()){
            m_timerIds.emplace(seqID, iter);
        }
        else{
            m_timerIdNodes.back().key() = seqID;
            m_timerIdNodes.back().mapped() = iter;

            m_timerIds.insert(std::move(m_timerIdNodes.back()));
            m_timerIdNodes.pop_back();
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

        waiter_map::iterator iter;
        if(m_waiterNodes.empty()){
            iter = m_waiters.emplace(seqID, cbArg).first;
        }
        else{
            m_waiterNodes.back().key() = seqID;
            m_waiterNodes.back().mapped() = cbArg;

            iter = m_waiters.insert(std::move(m_waiterNodes.back())).position;
            m_waiterNodes.pop_back();
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

        if(auto p = m_timerIds.find(id); p != m_timerIds.end()){
            found = true;
            cbArg = p->second->second.cbArg;

            m_timerNodes  .push_back(std::move(m_timers  .extract(p->second)));
            m_timerIdNodes.push_back(std::move(m_timerIds.extract(p)));

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

        if(auto p = m_waiters.find(id); p != m_waiters.end()){
            found = true;
            cbArg = p->second;

            m_waiterNodes.push_back(std::move(m_waiters.extract(p)));
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
