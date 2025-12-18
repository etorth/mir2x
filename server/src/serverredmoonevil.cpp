#include "sysconst.hpp"
#include "pathf.hpp"
#include "serverredmoonevil.hpp"

corof::awaitable<> ServerRedMoonEvil::runAICoro()
{
    while(!m_sdHealth.dead()){
        co_await asyncIdleWait(1000);
    }
}
