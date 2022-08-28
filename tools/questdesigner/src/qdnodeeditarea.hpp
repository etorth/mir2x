#pragma once
#include <set>
#include <FL/Fl.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Multiline_Input.H>
#include "qdbaseeditarea.hpp"
#include "qdinputlinebutton.hpp"
#include "qdinputtextbutton.hpp"
#include "qdinputmultilinebutton.hpp"

class QD_NodeEditArea: public QD_BaseEditArea
{
    private:
        QD_InputLineButton *m_title = nullptr;
        QD_InputMultilineButton *m_questLog = nullptr;

    private:
        QD_InputMultilineButton *m_enterTrigger = nullptr;

    private:
        QD_InputTextButton *m_leaveTrigger = nullptr;

    private:
        int m_currEdgeX = 0;
        int m_currEdgeY = 0;

        Fl_Button *m_currEdgeIn  = nullptr;
        Fl_Button *m_currEdgeOut = nullptr;

    private:
        std::set<std::tuple<Fl_Button *, Fl_Button *>> m_edges;

    public:
        QD_NodeEditArea(int, int, int, int, const char * = nullptr);

    public:
        int handle(int) override;

    public:
        void setEdgeIn (Fl_Button *);
        void setEdgeOut(Fl_Button *);

    public:
        Fl_Button *getEdgeIn()
        {
            return m_currEdgeIn;
        }

        Fl_Button *getEdgeOut()
        {
            return m_currEdgeOut;
        }

    public:
        void draw() override;
};
