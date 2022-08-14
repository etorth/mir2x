#include <cstdlib>
#include "flwrapper.hpp"
#include "qdquesteditarea.hpp"

QD_QuestEditArea::QD_QuestEditArea(int argX, int argY, int argW, int argH, const char *argLabel)
    : QD_BaseEditArea(argX, argY, argW, argH, argLabel)
{
    color(0x5585cd00);
}

int QD_QuestEditArea::handle(int event)
{
    int result = QD_BaseEditArea::handle(event);
    if((event == FL_PUSH) && (Fl::event_button() == FL_RIGHT_MOUSE)){
        fl_wrapper::menu_item rclick_menu[]
        {
            {"New Node", 0, +[](Fl_Widget *, void *)
            {

            }},

            {"Exit", 0, +[](Fl_Widget *, void *)
            {
            }},

            {},
        };

        if(const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, 0); m){
            m->do_callback(this, m->user_data());
            redraw();
        }
    }
    return result;
}
