/*
 * =====================================================================================
 *
 *       Filename: dispatcher.hpp
 *        Created: 01/26/2018 15:36:14
 *    Description:
 *                 anonymous message sender
 *                 actor who receives the message will uid zero
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
#include "messagebuf.hpp"

class Dispatcher
{
    public:
        Dispatcher() = default;

    public:
        virtual ~Dispatcher() = default;

    public:
        bool Forward(const MessageBuf &, const Theron::Address &, uint32_t = 0);
};
