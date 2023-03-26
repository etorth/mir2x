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
        std::unique_ptr<ServerLuaCoroutineRunner> m_luaRunner;

    public:
        Quest(const SDInitQuest &);

    protected:
        void onActivate() override;

    protected:
        void operateAM(const ActorMsgPack &) override;

    protected:
        void on_AM_METRONOME(const ActorMsgPack &);
};
