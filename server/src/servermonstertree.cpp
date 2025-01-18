#include "sysconst.hpp"
#include "servermonstertree.hpp"

corof::eval_poller<> ServerMonsterTree::updateCoroFunc()
{
    while(m_sdHealth.hp > 0){
        co_await corof::async_wait(2000);
    }

    goDie();
}
