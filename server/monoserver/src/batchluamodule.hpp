/*
 * =====================================================================================
 *
 *       Filename: batchluamodule.hpp
 *        Created: 12/19/2017 23:39:38
 *    Description: run non-interactively
 *                 won't support printLine() and command is static inside
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
#include "serverluamodule.hpp"

class BatchLuaModule: public ServerLuaModule
{
    private:
        std::string m_batchCmd;

    public:
        BatchLuaModule();
       ~BatchLuaModule() = default;

    public:
        bool LoadBatch(const char *szCmd)
        {
            if(szCmd){
                m_batchCmd = std::string(szCmd);
                return true;
            }
            return false;
        }

        bool Empty() const
        {
            return m_batchCmd.empty();
        }

    public:
        bool LoopOne();
};
