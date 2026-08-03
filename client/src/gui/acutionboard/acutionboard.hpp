#pragma once
#include <optional>
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class AcutionBoard final: public Widget
{
    private:
        constexpr static size_t m_pageSize = 13;

        constexpr static int m_tableX = 10;
        constexpr static int m_tableY = 84;
        constexpr static int m_tableW = 460;
        constexpr static int m_rowH = 19;

    private:
        ProcessRun *m_runProc;

    private:
        SDAcutionItemList m_sdAcutionItemList;
        size_t m_page = 0;
        std::optional<size_t> m_selected;
        double m_elapsedMS = 0.0;

    private:
        ImageBoard m_background;

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
        void updateDefault(double) override;
        void drawDefault(Widget::ROIMap) const override;
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void setItemList(SDAcutionItemList);

    private:
        size_t pageCount() const;
        void setPage(size_t);
        void updatePageButtonState();

    private:
        size_t currentTimeLeft(const SDAcutionItem &) const;
        static std::string formatTimeLeft(size_t);
        static uint32_t priceColor(size_t);

    private:
        void drawSelectedItem(Widget::ROIMap) const;
};
