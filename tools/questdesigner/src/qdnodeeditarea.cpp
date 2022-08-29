#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include "flwrapper.hpp"
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

    {   m_socketIn = new Fl_Button(50, 530, 30, 30, "@-1->");
    }

    {
        m_socketOuts = new Fl_Flex(1000, 200, 30, 30, Fl_Flex::VERTICAL);
        {
            new Fl_Button(0, 0, 0, 0, "@-1>");
        }
        m_socketOuts->end();
    }

    {   new QD_CondCheckerBox(500, 600, 500, 140);
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

                    {"New Socket Out", 0, +[](Fl_Widget *, void *p)
                    {
                        auto socketOuts = static_cast<Fl_Flex *>(p);
                        const auto socketHeight = socketOuts->h() / socketOuts->children();

                        socketOuts->begin();
                        {
                            new Fl_Button(0, 0, 0, 0, "@-1>");
                        }
                        socketOuts->end();

                        socketOuts->size(socketOuts->w(), socketHeight * socketOuts->children());
                    }, m_socketOuts},

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
    const auto fnDrawEdge = [this](Fl_Button *in, Fl_Button *out)
    {
        int  inX = 0;
        int  inY = 0;
        int outX = 0;
        int outY = 0;

        if(in){
            inX = in->x();
            inY = in->y() + in->h() / 2;
        }
        else{
            inX = m_currEdgeX;
            inY = m_currEdgeY;
        }

        if(out){
            outX = out->x() + out->w();
            outY = out->y() + out->h() / 2;
        }
        else{
            outX = m_currEdgeX;
            outY = m_currEdgeY;
        }

        fl_line(inX, inY, outX, outY);
    };

    fl_color(FL_MAGENTA);
    fl_line_style(FL_SOLID, 2);

    fl_begin_line();
    {
        if(m_currEdgeIn || m_currEdgeOut){
            fnDrawEdge(m_currEdgeIn, m_currEdgeOut);
        }

        for(auto [in, out]: m_edges){
            fnDrawEdge(in, out);
        }
    }
    fl_end_line();
}

void QD_NodeEditArea::setEdgeIn(Fl_Button *btn)
{
    m_currEdgeIn = btn;
    if(m_currEdgeIn && m_currEdgeOut){
        setEdgeOut(m_currEdgeOut);
    }
}

void QD_NodeEditArea::setEdgeOut(Fl_Button *btn)
{
    m_currEdgeOut = btn;
    if(m_currEdgeIn && m_currEdgeOut){
        removeEdge(m_currEdgeOut, false);
        m_edges.insert({m_currEdgeIn, m_currEdgeOut});
        m_currEdgeIn  = nullptr;
        m_currEdgeOut = nullptr;
    }
}

void QD_NodeEditArea::removeEdge(Fl_Button *edgeEnd, std::optional<bool> inOnly)
{
    if(edgeEnd){
        for(auto p = m_edges.begin(); p != m_edges.end();){
            if(inOnly.value_or(true) && std::get<0>(*p) == edgeEnd){
                p = m_edges.erase(p);
            }
            else if(!inOnly.value_or(false) && std::get<1>(*p) == edgeEnd){
                p = m_edges.erase(p);
            }
            else{
                ++p;
            }
        }
    }
}
