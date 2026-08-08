#pragma once
#include <array>
#include <cstdint>
#include <functional>
#include "protocoldef.hpp"
#include "sysconst.hpp"
#include "textbutton.hpp"
#include "tritexbutton.hpp"
#include "widget.hpp"

class Hero;
class ProcessRun;

class HeroStateBoard final: public Widget
{
    private:
        struct WearGrid
        {
            int x = 0;
            int y = 0;
            int w = SYS_INVGRIDPW;
            int h = SYS_INVGRIDPH;
            const char8_t *type = nullptr;
        };

    private:
        static constexpr int m_equipCharX = 103;
        static constexpr int m_equipCharY = 229;

    public:
        struct InitArgs final
        {
            ProcessRun *runProc = nullptr;
            std::function<void(uint64_t)> onTradeRequest {};
            Widget::WADPair parent {};
        };

    private:
        const std::array<WearGrid, WLG_END> m_gridList;
        TritexButton m_closeButton;

    private:
        TextButton m_tradeButton;
        TextButton m_friendButton;

    private:
        ProcessRun *m_processRun;
        uint64_t m_targetUID = 0;
        std::function<void(uint64_t)> m_onTradeRequest;

    public:
        explicit HeroStateBoard(HeroStateBoard::InitArgs);

    public:
        void updateDefault(double) override;

    protected:
        void drawDefault(Widget::ROIMap) const override;
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void inspect(uint64_t);
        void close();

    public:
        uint64_t targetUID() const
        {
            return m_targetUID;
        }

        Hero *target() const;

    private:
        void requestTrade();
        void drawItemHoverText(int) const;
};
