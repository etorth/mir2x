#include "totype.hpp"
#include "dbcomid.hpp"
#include "qditemchoice.hpp"

QD_ItemChoice::QD_ItemChoice(int argX, int argY, int argW, int argH, const char *argLabel)
    : Fl_Choice(argX, argY, argW, argH, argLabel)
{
    for(uint32_t i = 1; i < DBCOM_ITEMENDID(); ++i){
        m_menuItemList.push_back(Fl_Menu_Item{to_cstr(DBCOM_ITEMRECORD(i).name), 0, 0, 0, 0, (uchar)(FL_NORMAL_LABEL), 0, 14, 0});
    }

    m_menuItemList.push_back(Fl_Menu_Item{});
    menu(m_menuItemList.data());
}
