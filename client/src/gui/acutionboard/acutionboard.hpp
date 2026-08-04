#pragma once
#include "widget.hpp"
#include "imageboard.hpp"
#include "textboard.hpp"
#include "textbutton.hpp"
#include "tritexbutton.hpp"
#include "acutionitemlist.hpp"

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

        TextBoard m_sellerNoteTitle;
        TextBoard m_itemDescriptionTitle;
        TextBoard m_itemAttributeTitle;

        TextButton m_buttonContactSeller;

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
        void drawDefault(Widget::ROIMap) const override;
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setItemList(SDAcutionItemList sdAcutionItemList)
        {
            m_itemList.setItemList(std::move(sdAcutionItemList));
        }

    private:
        void drawSelectedItem(Widget::ROIMap) const;
};
