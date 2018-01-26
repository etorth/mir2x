/*
 * =====================================================================================
 *
 *       Filename: channel.hpp
 *        Created: 10/04/2017 12:36:13
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#pragma once
#include <atomic>
#include <memory>
#include "session.hpp"

class Channel final
{
    private:
        enum ChannStateType: int
        {
            CHANNSTATE_NONE   = -2,
            CHANNSTATE_MODIFY = -1,

            CHANNSTATE_IDLE   =  0,
            CHANNSTATE_INUSE1 =  1,
            CHANNSTATE_INUSE2 =  2,
            CHANNSTATE_INUSE3 =  3,
        };

    private:
        std::atomic<int> m_State;

    private:
        std::shared_ptr<Session> m_Session;

    public:
        Channel()
            : m_State(CHANNSTATE_NONE)
            , m_Session()
        {}

    public:
        bool Launch(const Theron::Address &rstAddr)
        {
            condcheck(m_Session);
            return m_Session->Launch(rstAddr);
        }

        void Bind(const Theron::Address &rstAddr)
        {
            condcheck(m_Session);
            return m_Session->Bind(rstAddr);
        }

    public:
        void ChannBuild(uint32_t nSessionID, asio::ip::tcp::socket stSocket)
        {
            while(true){
                switch(auto nCurrState = m_State.exchange(CHANNSTATE_MODIFY)){
                    case CHANNSTATE_NONE:
                        {
                            // OK safe to connect now
                            // then the session pointer should be empty

                            condcheck(!m_Session);
                            m_Session = std::make_shared<Session>(nSessionID, std::move(stSocket));

                            m_State.store(CHANNSTATE_IDLE);
                            return;
                        }
                    case CHANNSTATE_MODIFY:
                        {
                            // shouldn't do anything
                            // someone is modifying (or querying) the channel
                            break;
                        }
                    default:
                        {
                            // in use now
                            // we are trying to destroy an working session

                            // current I support this
                            // but not sure whether there is bug

                            condcheck(nCurrState >= CHANNSTATE_IDLE);
                            m_State.store(nCurrState);

                            ChannRelease();
                            break;
                        }
                }
            }
        }

    public:
        void ChannRelease()
        {
            while(true){
                switch(auto nCurrState = m_State.exchange(CHANNSTATE_MODIFY)){
                    case CHANNSTATE_NONE:
                        {
                            condcheck(!m_Session);
                            m_State.store(CHANNSTATE_NONE);
                            return;
                        }
                    case CHANNSTATE_MODIFY:
                        {
                            // don't do anything
                            // someone is modifying (or querying) the channel
                            break;
                        }
                    case CHANNSTATE_IDLE:
                        {
                            // channl is ready but no in use
                            // the session pointer should be initialized

                            // previous bug here
                            // idle state means no one is posting to the queue
                            // but the session may be working on already existing requests

                            condcheck(m_Session);
                            m_Session.reset();

                            m_State.store(CHANNSTATE_NONE);
                            return;
                        }
                    default:
                        {
                            // shouldn't do anything
                            // channel is in use and need to wait till all done

                            condcheck(m_Session);
                            condcheck(nCurrState > CHANNSTATE_IDLE);

                            m_State.store(nCurrState);
                            break;
                        }
                }

                // if we reach here means we need to poll
                // then yield current thread
                std::this_thread::yield();
            }
        }

    public:
        template<typename... Args> bool Send(uint8_t nHC, Args&&... args)
        {
            while(true){
                switch(auto nCurrState = m_State.exchange(CHANNSTATE_MODIFY)){
                    case CHANNSTATE_NONE:
                        {
                            condcheck(!m_Session);
                            m_State.store(CHANNSTATE_NONE);
                            return false;
                        }
                    case CHANNSTATE_MODIFY:
                        {
                            // wait till it's done
                            // someone is modifying (or querying) the channel
                            break;
                        }
                    default:
                        {
                            condcheck(m_Session);
                            condcheck(nCurrState >= CHANNSTATE_IDLE);

                            // don't hold the channel too long
                            // mark it as in-using and put it back to ready state
                            m_State.store(nCurrState + 1);

                            // do the send job
                            // this allows multiple user to send
                            // but the channel won't be released during any sending
                            auto bSendDone = m_Session->Send(nHC, std::forward<Args>(args)...);

                            // need to decrement the user count
                            // we know at least one sender is doning its work
                            // so any request to release current session won't take place

                            while(true){
                                switch(auto nUserCount = m_State.exchange(CHANNSTATE_MODIFY)){
                                    case CHANNSTATE_MODIFY:
                                        {
                                            // should wait till it's done
                                            // someone is querying the state
                                            break;
                                        }
                                    default:
                                        {
                                            condcheck(nUserCount > CHANNSTATE_IDLE);
                                            m_State.store(nUserCount - 1);

                                            return bSendDone;
                                        }
                                }

                                // yield current thread
                                // polling when reset the sender's number
                                std::this_thread::yield();
                            }

                            // to make the compiler happy
                            return false;
                        }
                }

                // if we reach here means we need to poll
                // then yield current thread
                std::this_thread::yield();
            }
            return false;
        }
};
