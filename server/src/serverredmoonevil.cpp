#include "sysconst.hpp"
#include "pathf.hpp"
#include "serverredmoonevil.hpp"

corof::awaitable<> ServerRedMoonEvil::runAICoro()
{
    while(m_sdHealth.hp > 0){
        co_await asyncIdleWait(1000);
    }
    goDie();
}
