#include "sysconst.hpp"
#include "pathf.hpp"
#include "serverredmoonevil.hpp"

corof::eval_poller ServerRedMoonEvil::updateCoroFunc()
{
    while(m_sdHealth.hp > 0){
        co_await corof::async_wait(2000);
    }

    goDie();
    co_return true;
}
