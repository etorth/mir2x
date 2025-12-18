#include "sysconst.hpp"
#include "servermonstertree.hpp"

corof::awaitable<> ServerMonsterTree::runAICoro()
{
    while(!m_sdHealth.dead()){
        co_await asyncIdleWait(1000);
    }
}
