#include "sysconst.hpp"
#include "pathf.hpp"
#include "serverredmoonevil.hpp"

corof::awaitable<> ServerRedMoonEvil::runAICoro()
{
    while(m_sdHealth.hp > 0){
        co_await corof::async_wait(2000);
    }

    goDie();
}
