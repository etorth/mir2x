/*
 * =====================================================================================
 *
 *       Filename: monitorbase.hpp
 *        Created: 04/21/2016 22:58:48
 *  Last Modified: 04/22/2016 01:21:28
 *
 *    Description: only react to message, no id etc.
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
#include <Theron/Theron.h>

class MonitorBase
{
    protected:
        ObjectPod   *m_ObjectPod;

    public:
        Monitor()
            : m_ObjectPod(nullptr)
        {
        }

        virtual ~Monitor()
        {
            delete m_ObjectPod;
        }

    public:
        virtual void Operate(const MessagePack &, Theron::Address) = 0;

        virtual bool Activate()
        {
            extern Theron::Framework *g_Framework;
            m_ObjectPod = new ObjectPod(*g_Framework,
                [this](const MessagePack &rstMPK, Theron::Address stFromAddr){
                    Operate(rstMPK, stFromAddr);
                });
        }
};
