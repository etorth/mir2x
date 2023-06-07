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
        // one player can only have one state runner
        // one player runs multiple FSM state simultaneously doesn't make sense
        std::unordered_map<uint64_t, uint64_t> m_uidStateRunner;

    private:
        std::unique_ptr<ServerLuaCoroutineRunner> m_luaRunner;

    public:
        Quest(const SDInitQuest &);

    protected:
        void onActivate() override;

    public:
        std::string getQuestName() const
        {
            return std::get<1>(filesys::decompFileName(m_scriptName.c_str(), true));
        }

        std::string getQuestDBName() const
        {
            return SYS_QUEST_TBL_PREFIX + getQuestName();
        }

    protected:
        void operateAM(const ActorMsgPack &) override;

    protected:
        void on_AM_METRONOME      (const ActorMsgPack &);
        void on_AM_REMOTECALL     (const ActorMsgPack &);
        void on_AM_QUESTNOTIFY    (const ActorMsgPack &);
        void on_AM_RUNQUESTTRIGGER(const ActorMsgPack &);
};
