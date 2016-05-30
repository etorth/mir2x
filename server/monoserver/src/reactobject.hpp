/*
 * =====================================================================================
 *
 *       Filename: reactobject.hpp
 *        Created: 04/21/2016 23:02:31
 *  Last Modified: 05/29/2016 12:28:27
 *
 *    Description: object only react to message, with an object pod
 *                 atoms of an react object:
 *                      1. before Activate(), we can call its internal method by ``this"
 *                      2. after Activate(), we can only use ReactObject::Send()
 *
 *                 In other word, after Activate(), react object can only communicate
 *                 with react object or receiver
 *
 *                 This prevent me implement MonoServer as react object. For MonoServer
 *                 it needs to manager SessionHub. However SessionHub is not an actor, so
 *                 if MonoServer is an react object, we have to launch SessionHub before
 *                 calling of MonoServer::Activate(), but, before activation of MonoServer
 *                 we don't have the address of Mo MonoServer to pass to SessionHub!
 *
 *                 In my design, SessionHub create Session's with SID and pass it to the
 *                 MonoServer, then MonoServer check info of this connection from DB and
 *                 create player object, bind Session pointer to the player and send the
 *                 player to proper RegionMonitor via ServerMap object.
 *
 *                 Another thing is for g_MonoServer->AddLog(...), if make MonoServer as
 *                 a react object, we can't use it anymore
 *
 *                 So let's make MonoServer as a receriver instead.
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
#include <queue>
#include <Theron/Theron.h>

#include "delaycmd.hpp"
#include "messagepack.hpp"
#include "serverobject.hpp"

class ActorPod;
class ReactObject: public ServerObject
{
    protected:
        ActorPod *m_ActorPod;

        // the document of Theron says we can call GetAddress() outside of 
        // Actor, but is that safe?
        Theron::Address m_ThisAddress;

    protected:
        std::priority_queue<DelayCmd> m_DelayCmdQ;
        std::vector<std::tuple<std::string, std::function<void()>, bool>> m_TriggerV;

    public:
        ReactObject(uint8_t);
        ~ReactObject();

    protected:
        bool AccessCheck();

    public:
        virtual Theron::Address Activate();

        Theron::Address GetAddress()
        {
            return m_ThisAddress;
        }

    public:
        virtual void Operate(const MessagePack &, const Theron::Address &) = 0;

    private:
        void InnTrigger()
        {
            size_t nIndex = 0;
            while(nIndex < m_TriggerV.size()){
                if(true
                        && std::get<1>(m_TriggerV[nIndex])    // callable
                        && std::get<2>(m_TriggerV[nIndex])){  // active
                    std::get<1>(m_TriggerV[nIndex])();
                    nIndex++;
                    continue;
                }

                // ok current trigger handler should be deleted
                // we delete disabled & invalid handler everytime we found it

                std::swap(m_TriggerV[nIndex], m_TriggerV.back());
                m_TriggerV.pop_back();
            }
        }

    public:
        void Delay(uint32_t, const std::function<void()> &);


    public:
        // just try to match, and overwrite the matched slot
        void Install(const std::string &szTriggerName, const std::function<void()> &fnTriggerOp)
        {
            for(auto &rstEle: m_TriggerV){
                if(std::get<0>(rstEle) == szTriggerName){
                    std::get<1>(rstEle) = fnTriggerOp;
                    std::get<2>(rstEle) = true;
                    return;
                }
            }
            m_TriggerV.emplace_back(std::make_tuple(szTriggerName, fnTriggerOp, true));
        }

        // disabled or invalid handler won't count
        bool Installed(const std::string &szTriggerName)
        {
            for(auto &rstEle: m_TriggerV){
                if(true
                        && (std::get<0>(rstEle) == szTriggerName)   // name matching
                        && (std::get<1>(rstEle))                    // callable
                        && (std::get<2>(rstEle))){                  // still active
                    return true;
                }
            }
            return false;
        }

        // uninstall a trigger, parameters
        // szTriggerName: trigger to be uninstalled
        // bInside      : true if we only disable it and the pod will delete next time
        //              : false will make the trigger deleted immediately
        void Uninstall(const std::string &szTriggerName, bool bInside = true)
        {
            size_t nIndex = 0;
            while(nIndex < m_TriggerV.size()){
                if(true
                        && (std::get<0>(m_TriggerV[nIndex]) != szTriggerName) // not the one
                        && (std::get<1>(m_TriggerV[nIndex]))                  // callable
                        && (std::get<2>(m_TriggerV[nIndex]))){                // active
                    nIndex++;
                    continue;
                }

                // ok current trigger handler should be deleted
                // we delete disabled & invalid handler everytime we found it

                // 1. disable it always
                std::get<2>(m_TriggerV[nIndex]) = false;

                // 2. if inside we have to jump to next
                if(bInside){ nIndex++; continue; }

                // 3. otherwise we delete now
                std::swap(m_TriggerV[nIndex], m_TriggerV.back());
                m_TriggerV.pop_back();
            }
        }

        void Uninstall(bool bInside = true)
        {
            if(!bInside){ m_TriggerV.clear(); return; }

            for(auto &rstEle: m_TriggerV){
                Uninstall(std::get<0>(rstEle), true);
            }
        }
};
