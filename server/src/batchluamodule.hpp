#pragma once
#include "serverluamodule.hpp"

class BatchLuaModule: public ServerLuaModule
{
    private:
        std::string m_batchCmd;

    public:
        BatchLuaModule();

    public:
        ~BatchLuaModule() override = default;

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
