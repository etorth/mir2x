/*
 * =====================================================================================
 *
 *       Filename: transponder.hpp
 *        Created: 04/23/2016 10:51:19
 *  Last Modified: 05/02/2016 01:01:53
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

#include "messagepack.hpp"

class ActorPod;
class Transponder
{
    protected:
        ActorPod    *m_ActorPod;
        // the document of Theron says we can call GetAddress() outside of 
        // Actor, but is that safe?
        Theron::Address m_ThisAddress;

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

    public:
        // send with response operation registering
        bool Send(const MessagePack &, const Theron::Address &,
                const std::function<void(const MessagePack &, const Theron::Address &)> &);
        bool Send(const MessagePack &rstMSG, const Theron::Address &rstAddress)
        {
            std::function<void(const MessagePack &, const Theron::Address &)> fnNullOp;
            return Send(rstMSG, rstAddress, fnNullOp);
        }
};
