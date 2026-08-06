#pragma once
#include <string>
#include "widget.hpp"
#include "imageboard.hpp"
#include "textboard.hpp"
#include "tritexbutton.hpp"
#include "auctionitemlist.hpp"
#include "auctionitemdetailupper.hpp"
#include "auctionitemdetaillower.hpp"

class ProcessRun;
class AuctionBoard final: public Widget
{
    private:
        ProcessRun *m_runProc;
        bool m_pending = false;

    private:
        ImageBoard m_background;
        AuctionItemList m_itemList;

        TextBoard m_category;
        TextBoard m_itemCount;
        TextBoard m_itemRange;

        TextBoard m_columnItem;
        TextBoard m_columnSeller;
        TextBoard m_columnTime;
        TextBoard m_columnPrice;

        AuctionItemDetailUpper m_itemDetailUpper;
        AuctionItemDetailLower m_itemDetailLower;

    private:
        TritexButton m_buttonPrevious;
        TritexButton m_buttonNext;
        TritexButton m_buttonRefresh;
        TritexButton m_buttonItemSearch;
        TritexButton m_buttonSellerSearch;
        TritexButton m_buttonRegister;
        TritexButton m_buttonBuy;
        TritexButton m_buttonCancel;
        TritexButton m_buttonClose;

    public:
        AuctionBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setItemList(SDAuctionItemList);

    private:
        const SDAuctionItem *selectedItem() const;

    private:
        void refreshItemList();

    private:
        void confirmBuy();
        void buyItem(uint64_t, std::string);

    private:
        void confirmUnregister();
        void unregisterItem(uint64_t, std::string);
};
