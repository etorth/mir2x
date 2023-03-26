#pragma once
#include "luamodule.hpp"

class ServerLuaModule: public LuaModule
{
    public:
        ServerLuaModule();

    public:
        ~ServerLuaModule() override = default;

    protected:
       void addLogString(int, const char8_t *) override;
};

class CallDoneFlag final
{
    private:
        std::shared_ptr<bool> m_flag;

    public:
        CallDoneFlag()
            : m_flag(std::make_shared<bool>(false))
        {}

        CallDoneFlag(const CallDoneFlag &arg)
            : m_flag(arg.m_flag)
        {}

    public:
        ~CallDoneFlag()
        {
            *m_flag = true;
        }

    public:
        CallDoneFlag(CallDoneFlag &&) = delete;

    public:
        CallDoneFlag & operator = (const CallDoneFlag & ) = delete;
        CallDoneFlag & operator = (      CallDoneFlag &&) = delete;

    public:
        operator bool () const
        {
            return *m_flag;
        }
};
