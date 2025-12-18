#pragma once
#include <cstdint>
#include <functional>
#include "widget.hpp"
#include "textboard.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"
#include "gfxcropboard.hpp"

class ProcessRun;
class ControlBoard;
class CBLeft: public Widget
{
    private:
        friend class ControlBoard;

    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard   m_bgFull;
        GfxCropBoard m_bg;

        ImageBoard   m_hpFull;
        GfxCropBoard m_hp;

        ImageBoard   m_mpFull;
        GfxCropBoard m_mp;

        ImageBoard   m_levelBarFull;
        GfxCropBoard m_levelBar;

        ImageBoard   m_inventoryBarFull;
        GfxCropBoard m_inventoryBar;

    private:
        TritexButton m_buttonQuickAccess;

    private:
        TritexButton m_buttonClose;
        TritexButton m_buttonMinize;

    private:
        TextBoard    m_mapGLocFull;
        GfxCropBoard m_mapGLoc;

    private:
        const int m_mapGLocMaxWidth   = 120;
        const int m_mapGLocPixelSpeed =  20;

    private:
        double m_mapGLocAccuTime = 0.0;

    public:
        CBLeft(
                Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                ProcessRun *,
                Widget * = nullptr,
                bool     = false);

    private:
        std::string getMapGLocStr() const;

    protected:
        void updateDefault(double fUpdateTime) override
        {
            m_mapGLocAccuTime += fUpdateTime;
            Widget::updateDefault(fUpdateTime);
        }
};
