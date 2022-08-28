#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include "flwrapper.hpp"
#include "qdtransition.hpp"
#include "qdcallbackbox.hpp"
#include "qdnodeeditarea.hpp"
#include "qdcondcheckerbox.hpp"

QD_NodeEditArea::QD_NodeEditArea(int argX, int argY, int argW, int argH, const char *argLabel)
    : QD_BaseEditArea(argX, argY, argW, argH, argLabel)
{
    {   new Fl_Box(0, 0, 20, 20); // dummy widget to hold corner-margin
    }

    {   m_title = new QD_InputLineButton(50, 50, 665, 20, "设置节点名称");
        m_title->input_align(FL_ALIGN_CENTER);
        m_title->wrap(false);
    }

    {   m_questLog = new QD_InputMultilineButton(50, 80, 665, 200, "设置节点日志");
        m_questLog->input_align(FL_ALIGN_LEFT);
        m_questLog->wrap(true);
    }

    {   m_enterTrigger = new QD_InputMultilineButton(50, 300, 665, 200, "设置节点进入逻辑");
        m_enterTrigger->input_align(FL_ALIGN_LEFT);
        m_enterTrigger->wrap(true);
    }

    {
        new QD_Transition(50, 550, 500, 140);
    }

    {
        new QD_CondCheckerBox(500, 600, 500, 140);
    }

    {   m_leaveTrigger = new QD_InputTextButton(1000, 400, 665, 200, "设置节点离开逻辑");
        m_leaveTrigger->label_align(FL_ALIGN_LEFT);
    }

    this->end();
    this->color(0x29b26900);
}

int QD_NodeEditArea::handle(int event)
{
    int result = QD_BaseEditArea::handle(event);
    if(event == FL_MOVE){
        m_currEdgeX = Fl::event_x();
        m_currEdgeY = Fl::event_y();

        if(m_currEdgeIn || m_currEdgeOut){
            redraw();
        }
    }

    if(!result){
        if(event == FL_PUSH){
            result = 1;
            m_title->edit(false);
            m_questLog->edit(false);
            m_enterTrigger->edit(false);
            m_leaveTrigger->edit(false);

            if(Fl::event_button() == FL_RIGHT_MOUSE){
                fl_wrapper::menu_item rclick_menu[]
                {

                    {"New Callback", 0, +[](Fl_Widget *, void * p)
                    {
                        static_cast<QD_NodeEditArea *>(p)->begin();
                        {
                            new QD_CallbackBox(500, 700, 500, 140);
                        }
                        static_cast<QD_NodeEditArea *>(p)->end();
                    }, this},

                    {"New CondChecker", 0, +[](Fl_Widget *, void *p)
                    {
                        static_cast<QD_NodeEditArea *>(p)->begin();
                        {
                            new QD_CondCheckerBox(500, 600, 500, 140);
                        }
                        static_cast<QD_NodeEditArea *>(p)->end();
                    }, this},

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
        }
    }
    return result;
}

void QD_NodeEditArea::draw()
{
    QD_BaseEditArea::draw();
    if(m_currEdgeIn || m_currEdgeOut){
        int  inX = 0;
        int  inY = 0;
        int outX = 0;
        int outY = 0;

        if(m_currEdgeIn){
            inX = m_currEdgeIn->x();
            inY = m_currEdgeIn->y() + m_currEdgeIn->h() / 2;
        }
        else{
            inX = m_currEdgeX;
            inY = m_currEdgeY;
        }

        if(m_currEdgeOut){
            outX = m_currEdgeOut->x() + m_currEdgeOut->w();
            outY = m_currEdgeOut->y() + m_currEdgeOut->h() / 2;
        }
        else{
            outX = m_currEdgeX;
            outY = m_currEdgeY;
        }

        fl_color(FL_MAGENTA);
        fl_line_style(FL_SOLID, 2);
        fl_begin_line();
        fl_line(inX, inY, outX, outY);
        fl_end_line();
    }
}
