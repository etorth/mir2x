/*
 * =====================================================================================
 *
 *       Filename: eventtask.hpp
 *        Created: 04/03/2016 21:12:59
 *  Last Modified: 04/03/2016 21:34:04
 *
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

#include "task.hpp"

class EventTask: public Task
{
    uint32_t m_ID;

    public:
        EventTask(uint32_t nDelayMS, const std::function<void()> &fnOp)
            : Task(nDelayMS, fnOp)
            , m_ID(0)
        {
        }
};
