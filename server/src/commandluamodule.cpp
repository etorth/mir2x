#include "luaf.hpp"
#include "actorpool.hpp"
#include "serdesmsg.hpp"
#include "monoserver.hpp"
#include "syncdriver.hpp"
#include "serverluamodule.hpp"
#include "serverluaobject.hpp"

extern ActorPool *g_actorPool;
extern MonoServer *g_monoServer;

CommandLuaModule::CommandLuaModule(uint32_t cwid)
    : ServerLuaModule()
    , m_CWID(cwid)
{
    bindFunction("asyncEval", [sloCreated = false, this](std::string code, sol::this_state s) mutable
    {
        std::vector<sol::object> resList;
        try{
            if(!g_actorPool->running()){
                throw fflerror("actor pool is not running");
            }

            if(!sloCreated){
                auto p = new ServerLuaObject(CWID());
                p->activate();
                sloCreated = true;
            }

            if(!g_actorPool->checkUIDValid(uidf::getServerLuaObjectUID(CWID()))){
                throw fflerror("no SLO bind to current command window: %d", CWID());
            }

            switch(const auto rmpk = SyncDriver().forward(uidf::getServerLuaObjectUID(CWID()), {AM_REMOTECALL, cerealf::serialize(SDRemoteCall
            {
                .code = code,
                .args = luaf::luaNil(),
            })});

            rmpk.type()){
                case AM_SDBUFFER:
                    {
                        sol::state_view sv(s);
			auto sdRCR = rmpk.deserialize<SDRemoteCallResult>();

                        if(sdRCR.error.empty()){
                            for(auto & var: sdRCR.varList){
                                resList.emplace_back(luaf::buildLuaObj(sv, std::move(var)));
                            }
                        }
                        else{
                            fflassert(sdRCR.varList.empty(), sdRCR.error, sdRCR.varList);
                            for(const auto &line: sdRCR.error){
                                g_monoServer->addCWLogString(CWID(), 2, ">>>", line.c_str());
                            }
                        }
                        return sol::as_returns(resList);
                    }
                case AM_BADACTORPOD:
                    {
                        throw fflerror("can not reach SLO");
                    }
                default:
                    {
                        throw fflerror("invalid message type: %s", mpkName(rmpk.type()));
                    }
            }
        }
        catch(const std::exception &e){
            g_monoServer->addCWLogString(CWID(), 2, ">>>", e.what());
        }
        catch(...){
            g_monoServer->addCWLogString(CWID(), 2, ">>>", "unknown error");
        }

        return sol::as_returns(resList);
    });

    g_monoServer->regLuaExport(this, cwid);
}
