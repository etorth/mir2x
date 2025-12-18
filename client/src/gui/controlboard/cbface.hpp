#pragma once
#include <cstdint>
#include <optional>
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "imageboard.hpp"
#include "gfxcropboard.hpp"
#include "gfxshapeboard.hpp"

class ProcessRun;
class CBFace: public Widget
{
    private:
        constexpr static int BAR_HEIGHT = 3;

    private:
        ProcessRun *m_processRun;

    private:
        double m_accuTime = 0;

    private:
        ImageBoard m_faceFull;

    private:
        GfxCropBoard m_face;
        GfxShapeBoard m_hpBar;

    private:
        GfxShapeBoard m_drawBuffIDList;

    public:
        CBFace( Widget::VarDir,
                Widget::VarInt,
                Widget::VarInt,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    private:
        double getHPRatio() const;
        uint32_t getFaceTexID() const;
        const std::optional<SDBuffIDList> &getSDBuffIDListOpt() const;

    private:
        void drawBuffIDList(int, int, int, int) const;

    private:
        void updateDefault(double fUpdateTime) override
        {
            m_accuTime += fUpdateTime;
        }
};
