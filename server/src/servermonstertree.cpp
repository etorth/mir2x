#include "sysconst.hpp"
#include "servermonstertree.hpp"

corof::awaitable<> ServerMonsterTree::runAICoro()
{
    while(m_sdHealth.hp > 0){
        co_await asyncIdleWait(1000);
    }
    goDie();
}
