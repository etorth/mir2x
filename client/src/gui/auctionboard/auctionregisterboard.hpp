#pragma once
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "imageboard.hpp"
#include "auctionregisternote.hpp"
#include "auctionregisterbox.hpp"
#include "auctionregisterprice.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class AuctionRegisterBoard final: public Widget
{
    private:
        ProcessRun *m_runProc;

    private:
        bool m_pending = false;
        bool m_dragging = false;

    private:
        ImageBoard m_background;
        AuctionRegisterNote m_note;
        AuctionRegisterBox m_box;
        AuctionRegisterPrice m_price;

        TritexButton m_buttonRegister;
        TritexButton m_buttonCancel;
        TritexButton m_buttonClose;

    public:
        AuctionRegisterBoard(ProcessRun *, Widget * = nullptr, bool = false);

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
        void restoreItemInBox();
        void restoreItemInGrab();

    private:
        void closeRegister(bool);
};
