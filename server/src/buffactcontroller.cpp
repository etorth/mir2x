#include "buffactcontroller.hpp"

BaseBuffActController::BaseBuffActController(uint32_t argBuffID, uint32_t argBuffActID)
    : BaseBuffAct(argBuffID, argBuffActID)
{}

BaseBuffActController *BaseBuffActController::createController(uint32_t argBuffID, uint32_t argBuffActID)
{
    return new BaseBuffActController(argBuffID, argBuffActID);
}
