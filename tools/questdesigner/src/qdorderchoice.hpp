#pragma once
#include <vector>
#include <FL/Fl.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Menu_Item.H>

class QD_OrderChoice: public Fl_Choice
{
    private:
        constexpr static Fl_Menu_Item m_menuItemList []
        {
            {    "大于", 0,  0, 0, 0, (uchar)(FL_NORMAL_LABEL), 0, 14, 0},
            {    "小于", 0,  0, 0, 0, (uchar)(FL_NORMAL_LABEL), 0, 14, 0},
            {    "等于", 0,  0, 0, 0, (uchar)(FL_NORMAL_LABEL), 0, 14, 0},
            {  "不等于", 0,  0, 0, 0, (uchar)(FL_NORMAL_LABEL), 0, 14, 0},
            {"大于等于", 0,  0, 0, 0, (uchar)(FL_NORMAL_LABEL), 0, 14, 0},
            {"小于等于", 0,  0, 0, 0, (uchar)(FL_NORMAL_LABEL), 0, 14, 0},

            {0,0,0,0,0,0,0,0,0},
        };

    public:
        QD_OrderChoice(int argX, int argY, int argW, int argH, const char *argLabel = nullptr)
            : Fl_Choice(argX, argY, argW, argH, argLabel)
        {
            menu(m_menuItemList);
        }
};
