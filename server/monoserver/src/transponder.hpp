/*
 * =====================================================================================
 *
 *       Filename: transponder.hpp
 *        Created: 04/23/2016 10:51:19
 *  Last Modified: 05/28/2016 00:06:50
 *
 *    Description: base of actor model in mir2x, Theron::Actor acitvated at create
 *                 time so no way to control it, instead Transponder can 
 *                      1. react to message by callback
 *                      2. activate when needed
 *                      3. delete it to disable it
 *
 *                 it's not an object necessarily, in monoserver, an ``object" means
 *                 it has (UID(), AddTime()), but an transponder may not consist of
 *                 these attributes.
 *
 *                 Transponder : with an actor pod, override Operate() for operation
 *                 ReactObject : is an ``object" which has an acotr pod, and override
 *                               Operate() for operation
 *                 ActiveObject: is an ReactObject
 *
 *                 ReactObject is not an transponder, it's an ServerObject, because
 *                 I am trying to avoid MI.
 *
 *
 *                 Trigger can be install/uninstall before / after Activate(), good
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
#include <vector>
#include <utility>
#include <functional>

#include <Theron/Theron.h>

#include "delaycmd.hpp"
#include "messagepack.hpp"

class ActorPod;
class Transponder
{
    protected:
        ActorPod    *m_ActorPod;

        // the document of Theron says we can call GetAddress() outside of 
        // Actor, but is that safe?
        Theron::Address m_ThisAddress;


    protected:
        // TODO & TBD
        // use trigger here since most of the time we are traversing
        // rather than install/uninstall trigger
        std::priority_queue<DelayCmd> m_DelayCmdQ;
        std::vector<std::tuple<std::string, std::function<void()>, bool>> m_TriggerV;

    public:
        Transponder();
        virtual ~Transponder();

    public:
        virtual void Operate(const MessagePack &, const Theron::Address &) = 0;

    public:
        virtual Theron::Address Activate();

        Theron::Address GetAddress()
        {
            return m_ThisAddress;
        }

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

                // 2. if we are not calling this funciton inside the trigger
                //    we can directly remove it, otherwise we can only mark it
                if(!bInside){
                    std::swap(m_TriggerV[nIndex], m_TriggerV.back());
                    m_TriggerV.pop_back();
                }
            }
        }

        void Uninstall(bool bInside = true)
        {
            if(!bInside){ m_TriggerV.clear(); return; }

            for(auto &rstEle: m_TriggerV){
                Uninstall(std::get<0>(rstEle), true);
            }
        }

    public:
        // // send with response operation registering
        // bool Send(const MessagePack &, const Theron::Address &,
        //         const std::function<void(const MessagePack &, const Theron::Address &)> &);
        // bool Send(const MessagePack &rstMSG, const Theron::Address &rstAddress)
        // {
        //     std::function<void(const MessagePack &, const Theron::Address &)> fnNullOp;
        //     return Send(rstMSG, rstAddress, fnNullOp);
        // }
};
