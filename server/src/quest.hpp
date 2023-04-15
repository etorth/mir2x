#pragma once
#include <memory>
#include "serdesmsg.hpp"
#include "serverobject.hpp"
#include "serverluacoroutinerunner.hpp"

class Quest final: public ServerObject
{
    private:
        const std::string m_scriptName;

    private:
        const uint64_t m_mainScriptThreadKey = 1;
        /* */ uint64_t m_threadKey = m_mainScriptThreadKey + 1;

    private:
        std::unique_ptr<ServerLuaCoroutineRunner> m_luaRunner;

    public:
        Quest(const SDInitQuest &);

    protected:
        void onActivate() override;

    protected:
        void operateAM(const ActorMsgPack &) override;

    protected:
        void on_AM_METRONOME      (const ActorMsgPack &);
        void on_AM_REMOTECALL     (const ActorMsgPack &);
        void on_AM_QUESTNOTIFY    (const ActorMsgPack &);
        void on_AM_RUNQUESTTRIGGER(const ActorMsgPack &);
};
