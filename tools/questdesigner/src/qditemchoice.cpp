#include "totype.hpp"
#include "dbcomid.hpp"
#include "qditemchoice.hpp"

QD_ItemChoice::QD_ItemChoice(int argX, int argY, int argW, int argH, const char *argLabel)
    : Fl_Choice(argX, argY, argW, argH, argLabel)
{
    std::vector<std::u8string_view> sortedType;
    std::unordered_map<std::u8string_view, std::vector<uint32_t>> typedItemList;

    for(uint32_t i = 1; i < DBCOM_ITEMENDID(); ++i){
        if(const auto &ir = DBCOM_ITEMRECORD(i)){
            if(!typedItemList.count(ir.type)){
                sortedType.push_back(ir.type);
            }
            typedItemList[ir.type].push_back(i);
        }
    }

    for(const auto &type: sortedType){
        m_menuItemList.push_back(Fl_Menu_Item(to_cstr(type), 0, 0, 0, FL_SUBMENU, (uchar)(FL_NORMAL_LABEL), 0, 14, 0));
        for(uint32_t id: typedItemList.at(type)){
            m_menuItemList.push_back(Fl_Menu_Item(to_cstr(DBCOM_ITEMRECORD(id).name), 0, 0, 0, 0, (uchar)(FL_NORMAL_LABEL), 0, 14, 0));
        }
        m_menuItemList.push_back(Fl_Menu_Item());
    }

    m_menuItemList.push_back(Fl_Menu_Item());
    menu(m_menuItemList.data());
}
