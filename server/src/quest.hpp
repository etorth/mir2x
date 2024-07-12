#pragma once
#include <memory>
#include "serdesmsg.hpp"
#include "serverobject.hpp"

class Quest final: public ServerObject
{
    private:
        class LuaThreadRunner: public ServerObject::LuaThreadRunner
        {
            private:
                using LuaThreadHandle = ServerObject::LuaThreadRunner::LuaThreadHandle;

            public:
                LuaThreadRunner(Quest *);

            protected:
                void closeQuestState(uint64_t, const char *, const void *);

            public:
                Quest *getQuest() const
                {
                    return static_cast<Quest *>(getSO());
                }
        };

    private:
        const std::string m_scriptName;

    private:
        const uint64_t m_mainScriptThreadKey = 1;
        /* */ uint64_t m_threadKey = m_mainScriptThreadKey + 1;

    private:
        // one player can only have one state runner
        // one player runs multiple FSM state simultaneously doesn't make sense
        std::unordered_map<std::string, std::unordered_map<uint64_t, uint64_t>> m_uidStateRunner;

    private:
        std::unique_ptr<ServerObject::LuaThreadRunner> m_luaRunner;

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

    public:
        void dumpQuestField(uint64_t, const std::string &) const;

    protected:
        void operateAM(const ActorMsgPack &) override;

    protected:
        void on_AM_METRONOME      (const ActorMsgPack &);
        void on_AM_REMOTECALL     (const ActorMsgPack &);
        void on_AM_RUNQUESTTRIGGER(const ActorMsgPack &);
};
