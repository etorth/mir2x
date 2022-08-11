#pragma once
#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/fl_draw.H>
#include "qdbaseeditarea.hpp"

class QD_QuestEditArea: public QD_BaseEditArea
{
    public:
        QD_QuestEditArea(int, int, int, int, const char * = nullptr);
};
