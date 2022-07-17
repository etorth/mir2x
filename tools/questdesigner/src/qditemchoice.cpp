#include "totype.hpp"
#include "dbcomid.hpp"
#include "qditemchoice.hpp"

QD_ItemChoice::QD_ItemChoice(int argX, int argY, int argW, int argH, const char *argLabel)
    : Fl_Choice(argX, argY, argW, argH, argLabel)
{
    std::vector<Fl_Menu_Item> menuItems;
    menuItems.reserve(DBCOM_ITEMENDID());

    menuItems.push_back(Fl_Menu_Item{"", 0, 0, 0, 0, (uchar)(FL_NORMAL_LABEL), 0, 14, 0});
    for(uint32_t i = 1; i < DBCOM_ITEMENDID(); ++i){
        menuItems.push_back(Fl_Menu_Item{to_cstr(DBCOM_ITEMRECORD(i).name), 0, 0, 0, 0, (uchar)(FL_NORMAL_LABEL), 0, 14, 0});
    }

    menuItems.push_back(Fl_Menu_Item{});
    menu(menuItems.data());
}
