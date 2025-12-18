#pragma once
#include "widget.hpp"
#include "imageboard.hpp"
#include "itemflex.hpp"
#include "tritexbutton.hpp"
#include "marginwrapper.hpp"
#include "gfxshapeboard.hpp"

class ProcessRun;
class MiniMapBoard: public Widget
{
    private:
        struct InitArgs final
        {
            ProcessRun *proc {};
            Widget::WADPair parent {};
        };

    private:
        bool m_alphaOn = false;
        bool m_extended = false;
        bool m_autoCenter = true;

    private:
        double m_zoomFactor = 1.0;

    private:
        int m_mapImage_dx = 0; // used if autoCenter is disabled
        int m_mapImage_dy = 0;

    private:
        bool m_dragStarted = false;

    private:
        ProcessRun *m_processRun;

    private:
        GfxShapeBoard m_bg;
        ImageBoard m_mapImage;

    private:
        GfxShapeBoard m_canvas;

    private:
        ImageBoard m_cornerUpLeft;
        ImageBoard m_cornerUpRight;
        ImageBoard m_cornerDownLeft;

    private:
        MarginWrapper m_zoomFactorBoard;
        TritexButton  m_buttonAlpha;
        TritexButton  m_buttonExtend;
        TritexButton  m_buttonAutoCenter;
        TritexButton  m_buttonConfig;

    private:
        ItemFlex m_buttonFlex;

    public:
        MiniMapBoard(MiniMapBoard::InitArgs);

    public:
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void flipAlpha();
        void flipExtended();
        void flipAutoCenter();

    public:
        SDL_Texture *getMiniMapTexture() const;

    private:
        void zoomOnCanvasAt(int, int, double);

    private:
        void normalizeMapImagePLoc();

    private:
        void drawCanvas(int, int);

    private:
        std::tuple<int, int>      onMapGLoc_from_onCanvasPLoc  (const std::tuple<int, int> &) const;
        std::tuple<int, int>   onCanvasPLoc_from_onMapGLoc     (const std::tuple<int, int> &) const;
        std::tuple<int, int> onMapImagePLoc_from_onMapGLoc     (const std::tuple<int, int> &) const;
        std::tuple<int, int> onMapImagePLoc_from_onCanvasPLoc  (const std::tuple<int, int> &) const;
        std::tuple<int, int>   onCanvasPLoc_from_onMapImagePLoc(const std::tuple<int, int> &) const;
};
