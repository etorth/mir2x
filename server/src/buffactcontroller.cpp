#include "fflerror.hpp"
#include "buff.hpp"
#include "buffact.hpp"
#include "buffactcontroller.hpp"

BaseBuffActController::BaseBuffActController(BaseBuff *argBuff, size_t argBuffActOff)
    : BaseBuffAct(argBuff, argBuffActOff)
{
    fflassert(getBAR().isController());
}

BaseBuffActController *BaseBuffActController::createController(BaseBuff *argBuff, size_t argBuffActOff)
{
    return new BaseBuffActController(argBuff, argBuffActOff);
}
