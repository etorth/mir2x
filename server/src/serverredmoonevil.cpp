#include "sysconst.hpp"
#include "pathfinder.hpp"
#include "serverredmoonevil.hpp"

corof::long_jmper ServerRedMoonEvil::updateCoroFunc()
{
    while(m_sdHealth.HP > 0){
        co_await corof::async_wait(2000);
    }

    goDie();
    co_return true;
}
