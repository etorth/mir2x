#pragma once
#include "widget.hpp"
#include "imageboard.hpp"
#include "textboard.hpp"
#include "tritexbutton.hpp"
#include "acutionitemlist.hpp"
#include "acutionitemdetailupper.hpp"
#include "acutionitemdetaillower.hpp"

class ProcessRun;
class AcutionBoard final: public Widget
{
    private:
        ProcessRun *m_runProc;

    private:
        ImageBoard m_background;
        AcutionItemList m_itemList;

        TextBoard m_category;
        TextBoard m_itemCount;
        TextBoard m_itemRange;

        TextBoard m_columnItem;
        TextBoard m_columnSeller;
        TextBoard m_columnTime;
        TextBoard m_columnPrice;

        AcutionItemDetailUpper m_itemDetailUpper;
        AcutionItemDetailLower m_itemDetailLower;

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
        AcutionBoard(ProcessRun *, Widget * = nullptr, bool = false);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setItemList(SDAcutionItemList);

    private:
        void updateSelectedItem();
};
