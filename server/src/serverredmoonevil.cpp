#include "sysconst.hpp"
#include "pathf.hpp"
#include "serverargparser.hpp"
#include "serverredmoonevil.hpp"

extern ServerArgParser *g_serverArgParser;
corof::awaitable<> ServerRedMoonEvil::runAICoro()
{
    while(m_sdHealth.hp > 0){
        if(g_serverArgParser->sharedConfig().forceMonsterRandomMove || hasPlayerNeighbor()){
            co_await asyncWait(1000);
        }
        else{
            m_idleWaitToken.emplace();
            co_await asyncWait(0, std::addressof(m_idleWaitToken.value())); // infinite wait till cancel
        }
    }

    goDie();
}
