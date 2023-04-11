#include "quest.hpp"
#include "filesys.hpp"

Quest::Quest(const SDInitQuest &initQuest)
    : ServerObject(uidf::getQuestUID(initQuest.questID))
    , m_scriptName(initQuest.fullScriptName)
{}

void Quest::onActivate()
{
    ServerObject::onActivate();
    m_luaRunner = std::make_unique<ServerLuaCoroutineRunner>(m_actorPod);

    m_luaRunner->bindFunction("getQuestName", [this]() -> std::string
    {
        return std::get<1>(filesys::decompFileName(m_scriptName.c_str(), true));
    });

    m_luaRunner->bindFunctionCoop<int, bool>("_RSVD_NAME_modifyQuestTriggerType", [this](int triggerType, bool enable, LuaCoopResumer onOK, LuaCoopResumer onError)
    {
        fflassert(triggerType >= SYS_ON_BEGIN, triggerType);
        fflassert(triggerType <  SYS_ON_END  , triggerType);

        AMModifyQuestTriggerType amMQTT;
        std::memset(&amMQTT, 0, sizeof(amMQTT));

        amMQTT.type = triggerType;
        amMQTT.enable = enable;

        m_actorPod->forward(uidf::getServiceCoreUID(), {AM_MODIFYQUESTTRIGGERTYPE, amMQTT}, [onOK, onError, this](const ActorMsgPack &rmpk)
        {
            // expected an reply
            // this makes sure when modifyQuestTriggerType() returns, the trigger has already been enabled/disabled

            switch(rmpk.type()){
                case AM_OK:
                    {
                        onOK(true);
                        break;
                    }
                default:
                    {
                        onError();
                        break;
                    }
            }
        });
    });

    m_luaRunner->bindFunctionCoop<std::string>("_RSVD_NAME_loadMap", [this](std::string mapName, LuaCoopResumer onOK, LuaCoopResumer onError)
    {
        fflassert(str_haschar(mapName));

        AMLoadMap amLM;
        std::memset(&amLM, 0, sizeof(AMLoadMap));

        amLM.mapID = DBCOM_MAPID(to_u8cstr(mapName));
        amLM.activateMap = true;

        m_actorPod->forward(uidf::getServiceCoreUID(), {AM_LOADMAP, amLM}, [mapID = amLM.mapID, onOK = std::move(onOK), onError = std::move(onError), this](const ActorMsgPack &mpk)
        {
            switch(mpk.type()){
                case AM_LOADMAPOK:
                    {
                        const auto amLMOK = mpk.conv<AMLoadMapOK>();
                        onOK(amLMOK.uid);
                        break;
                    }
                default:
                    {
                        onError();
                        break;
                    }
            }
        });
    });

    m_luaRunner->pfrCheck(m_luaRunner->execRawString(BEGIN_LUAINC(char)
#include "quest.lua"
    END_LUAINC()));

    // define all functions needed for the quest
    // but don't execute them here since they may require coroutine environment
    m_luaRunner->pfrCheck(m_luaRunner->execFile(m_scriptName.c_str()));

    m_luaRunner->spawn(m_mainScriptThreadKey, str_printf(
        R"#( do                           )#""\n"
        R"#(     getTLSTable().uid = %llu )#""\n"
        R"#(     return main()            )#""\n"
        R"#( end                          )#""\n", to_llu(UID())));
}

void Quest::operateAM(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_METRONOME:
            {
                on_AM_METRONOME(mpk);
                break;
            }
        case AM_RUNQUESTTRIGGER:
            {
                on_AM_RUNQUESTTRIGGER(mpk);
                break;
            }
        default:
            {
                throw fflerror("unsupported message: %s", mpkName(mpk.type()));
            }
    }
}
