#pragma once
#include <memory>
#include "serdesmsg.hpp"
#include "serverobject.hpp"

class Quest final: public ServerObject
{
    private:
        class LuaThreadRunner: public ServerObject::LuaThreadRunner
        {
            public:
                LuaThreadRunner(Quest *);

            public:
                Quest *getQuest() const
                {
                    return static_cast<Quest *>(getSO());
                }
        };

    private:
        const std::string m_scriptName;
        /* */ std::string m_scriptHash;

    private:
        const uint64_t m_mainScriptThreadKey = 1;
        /* */ uint64_t m_threadKey = m_mainScriptThreadKey + 1;

    private:
        std::unique_ptr<ServerObject::LuaThreadRunner> m_luaRunner;

    public:
        Quest(const SDInitQuest &);

    protected:
        corof::awaitable<> onActivate() override;

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
        corof::awaitable<> onActorMsg(const ActorMsgPack &) override;

    protected:
        corof::awaitable<> on_AM_REMOTECALL(const ActorMsgPack &);
};
