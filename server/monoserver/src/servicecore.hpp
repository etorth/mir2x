/*
 * =====================================================================================
 *
 *       Filename: servicecore.hpp
 *        Created: 04/22/2016 17:59:06
 *  Last Modified: 04/22/2016 22:44:09
 *
 *    Description: split monoserver into actor-code and non-actor code
 *                 put all actor code in this class
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
#include <system_erro>

#include "objectpod.hpp"

class ServiceCore
{
    protected:
        ObjectPod *m_ObjectPod;

    protected:
        static m_Count {0};

    public:
        ServiceCore()
            : m_ObjectPod(nullptr)
        {
            m_Count++;
            if(m_Count > 1){
                extern Log *g_Log;
                g_Log->AddLog(LOGTYPE_WARNING, "one service core please");
                throw std::error_code();
            }
        }

        virtual ~ServiceCore()
        {
            delete m_ObjectPod;
            m_Count--;
        }

        Theron::Address Activate()
        {
            auto fnOperate = [this](const MessagePack &rstMPK, const Theron::Address &rstAddr){
                this->Operate(rstMPK, rstAddr);
            };

            m_ObjectPod = new ObjectPod(fnOperate);

            return m_ObjectPod->GetAddress();
        }

        Theron::Address GetAddress()
        {
            if(!m_ObjectPod){
                return Theron::Address::Null();
            }

            return m_ObjectPod->GetAddress();
        }

    public:
        void Operate(const MessagePack &, const Theron::Address &);
};
