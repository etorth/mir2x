/*
 * =====================================================================================
 *
 *       Filename: eventtask.hpp
 *        Created: 04/03/2016 22:55:21
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
#include "task.hpp"

class EventTask: public Task
{
    public:
        friend class EventTaskHub;

    protected:
		uint32_t m_eventID;

	protected:
		EventTask(uint32_t nDelayMS, const std::function<void()>& fnOp)
            : Task(nDelayMS, fnOp)
            , m_eventID(0)
        {}

        virtual ~EventTask() = default;

	public:
		void ID(uint32_t nID)
        {
			m_eventID = nID;
		}

		uint32_t ID() const
        {
			return m_eventID;
		}

        std::chrono::system_clock::time_point Cycle() const
        {
            return m_expiration;
        }
};
