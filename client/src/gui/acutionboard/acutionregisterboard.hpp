#pragma once
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "imageboard.hpp"
#include "acutionregisternote.hpp"
#include "acutionregisteritem.hpp"
#include "acutionregisterprice.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class AcutionRegisterBoard final: public Widget
{
    private:
        ProcessRun *m_runProc;

    private:
        bool m_pending = false;
        bool m_dragging = false;

    private:
        ImageBoard m_background;
        AcutionRegisterNote m_note;
        AcutionRegisterItem m_item;
        AcutionRegisterPrice m_price;

        TritexButton m_buttonRegister;
        TritexButton m_buttonCancel;
        TritexButton m_buttonClose;

    public:
        AcutionRegisterBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void beginRegister();

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    private:
        void setPending(bool);
        void setPrice();
        void confirmRegister();
        void registerItem();
        void confirmCancel();

    private:
        void restoreGrabbedItem();
        void restoreOwnedItem();
        void closeRegister(bool);
};
