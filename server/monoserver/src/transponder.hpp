/*
 * =====================================================================================
 *
 *       Filename: transponder.hpp
 *        Created: 04/23/2016 10:51:19
 *  Last Modified: 05/27/2016 00:34:51
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
            auto pTrigger = m_TriggerV.begin();
            while(pTrigger != m_TriggerV.end()){
                // this trigger is disabled
                if(!std::get<2>(*pTrigger)){
                    std::swap(*pTrigger, m_TriggerV.back());
                    m_TriggerV.pop_back();
                    pTrigger = m_TriggerV.begin();
                    continue;
                }

                if(std::get<1>(*pTrigger)){
                    std::get<1>(*pTrigger)();
                }
            }
        }

    public:
        void Delay(uint32_t, const std::function<void()> &);


    public:
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

        bool Installed(const std::string &szTriggerName)
        {
            for(auto &rstEle: m_TriggerV){
                if(std::get<0>(rstEle) == szTriggerName){ return true; }
            }
            return false;
        }

        // uninstall a trigger, parameters
        // szTriggerName: trigger to be uninstalled
        // bInside      : true if we only disable it and the pod will delete next time
        //              : false will make the trigger deleted immediately
        void Uninstall(const std::string &szTriggerName, bool bInside = true)
        {
            if(m_TriggerV.empty()){ return; }

            // if the last one is already disabled
            if(!std::get<2>(m_TriggerV.back())){ m_TriggerV.pop_back(); }

            // now handle current one
            // there won't be duplicated names, so if found we directly break
            bool bFind = false;
            for(auto &rstEle: m_TriggerV){
                if(std::get<0>(rstEle) == szTriggerName){
                    // 1. disable it
                    std::get<2>(rstEle) = false;
                    // 2. swap it to the end
                    std::swap(rstEle, m_TriggerV.back());
                    // 3. we find it
                    bFind = true;
                    // 4. no duplicates, directly break
                    break;
                }
            }

            if(bFind && !bInside){ m_TriggerV.pop_back(); }
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
