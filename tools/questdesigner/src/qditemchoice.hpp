#pragma once
#include <vector>
#include <FL/Fl.H>
#include <Fl/Fl_Choice.H>

class QD_ItemChoice: public Fl_Choice
{
    public:
        QD_ItemChoice(int, int, int, int, const char * = nullptr);

    private:
        std::vector<Fl_Menu_Item> m_menuItemList;
};
