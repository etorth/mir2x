#include "quest.hpp"

Quest::Quest(const SDInitQuest &initQuest)
    : ServerObject(uidf::getQuestUID(initQuest.questID))
    , m_scriptName(initQuest.fullScriptName)
{}

void Quest::onActivate()
{
    ServerObject::onActivate();
    m_luaRunner = std::make_unique<ServerLuaCoroutineRunner>(m_actorPod, [this](ServerLuaModule *luaModule)
    {
        luaModule->bindFunction("_RSVD_NAME_loadMapCoop", [this](std::string mapName, sol::function onOK, sol::function onError, uint64_t runnerSeqID, sol::this_state s)
        {
            fflassert(str_haschar(mapName));
            fflassert(runnerSeqID > 0, runnerSeqID);

            AMLoadMap amLM;
            std::memset(&amLM, 0, sizeof(AMLoadMap));

            amLM.mapID = DBCOM_MAPID(to_u8cstr(mapName));
            amLM.activateMap = true;

            const CallDoneFlag doneFlag;
            m_actorPod->forward(uidf::getServiceCoreUID(), {AM_LOADMAP, amLM}, [doneFlag, mapID = amLM.mapID, onOK, onError, runnerSeqID, s, this](const ActorMsgPack &mpk)
            {
                switch(mpk.type()){
                    case AM_LOADMAPOK:
                        {
                            sol::state_view sv(s);
                            const auto amLMOK = mpk.conv<AMLoadMapOK>();

                            onOK(sol::object(sv, sol::in_place_type<uint64_t>, amLMOK.uid));
                            if(doneFlag){
                                m_luaRunner->resume(runnerSeqID);
                            }
                            break;
                        }
                    default:
                        {
                            onError();
                            if(doneFlag){
                                m_luaRunner->resume(runnerSeqID);
                            }
                            break;
                        }
                }
            });
        });

        luaModule->pfrCheck(luaModule->execRawString(BEGIN_LUAINC(char)
#include "quest.lua"
        END_LUAINC()));
    });

    m_luaRunner->spawn(m_mainScriptThreadKey, {}, str_printf(
        R"#( do                           )#""\n"
        R"#(     getTLSTable().uid = %llu )#""\n"
        R"#(     return dofile('%s')      )#""\n"
        R"#( end                          )#""\n", to_llu(UID()), m_scriptName.c_str()).c_str());
}

void Quest::operateAM(const ActorMsgPack &mpk)
{
    switch(mpk.type()){
        case AM_METRONOME:
            {
                on_AM_METRONOME(mpk);
                break;
            }
        default:
            {
                throw fflerror("unsupported message: %s", mpkName(mpk.type()));
            }
    }
}
