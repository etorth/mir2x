#include "totype.hpp"
#include "dbcomid.hpp"
#include "qdorderchoice.hpp"

QD_OrderChoice::QD_OrderChoice(int argX, int argY, int argW, int argH, const char *argLabel)
    : Fl_Choice(argX, argY, argW, argH, argLabel)
{
    menu(m_menuItemList);
}
