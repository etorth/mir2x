#include <cstdlib>
#include "qdquesteditarea.hpp"

QD_QuestEditArea::QD_QuestEditArea(int argX, int argY, int argW, int argH, const char *argLabel)
    : QD_BaseEditArea(argX, argY, argW, argH, argLabel)
{
    color(0x567b9100);
}
