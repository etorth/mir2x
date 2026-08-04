#pragma once
#include "widget.hpp"
#include "imageboard.hpp"
#include "textbutton.hpp"
#include "tritexbutton.hpp"
#include "acutionitemlist.hpp"

class ProcessRun;
class AcutionBoard final: public Widget
{
    private:
        constexpr static size_t m_pageSize = AcutionItemList::m_rowCount;

    private:
        ProcessRun *m_runProc;

    private:
        size_t m_page = 0;

    private:
        ImageBoard m_background;
        AcutionItemList m_itemList;
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
        void setItemList(SDAcutionItemList);

    private:
        size_t pageCount() const;
        void setPage(size_t);
        void updatePageButtonState();

    private:
        void drawSelectedItem(Widget::ROIMap) const;
};
