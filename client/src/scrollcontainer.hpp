#pragma once
#include <algorithm>
#include <utility>
#include "widget.hpp"
#include "colorf.hpp"
#include "gfxcropboard.hpp"

// vertical scroll bar (analogous for horizontal):
//
// ┌──────────┐  ← vBarROI top
// │ up arrow │    vUpArrowROI   (barSize × barSize square at top)
// ├──────────┤
// │          │
// │   track  │    vTrackROI     (the middle segment between arrows)
// │  ┌────┐  │
// │  │thmb│  │    vThumbROI     (sits inside the track, sized proportionally)
// │  └────┘  │
// │          │
// ├──────────┤
// │ down arw │    vDownArrowROI (barSize × barSize square at bottom)
// └──────────┘  ← vBarROI bottom

class ScrollContainer: public Widget
{
    public:
        struct InitArgs final
        {
            Widget::VarDir  dir = DIR_UPLEFT;
            Widget::VarInt  x   = 0;
            Widget::VarInt  y   = 0;
            Widget::VarSize w   = 0;
            Widget::VarSize h   = 0;

            Widget::VarGetter<Widget *> getter = nullptr;

            Widget::VarBool hScroll = true;
            Widget::VarBool vScroll = true;

            int      barSize = 12; // scrollbar thickness
            int minThumbSize = 20; // minimal thumb length
            int   scrollStep = 24;

            Widget::VarDrawFunc bgDrawFunc = nullptr; // painted under the content, inside the viewport rect
            Widget::VarDrawFunc fgDrawFunc = nullptr; // painted over  the content, inside the viewport rect (below the scrollbars)

            Widget::VarU32 trackColor = colorf::RGBA(232, 232, 232, 255);

            Widget::VarU32 thumbColor      = colorf::RGBA(176, 176, 176, 255);
            Widget::VarU32 thumbHoverColor = colorf::RGBA(144, 144, 144, 255);
            Widget::VarU32 thumbDragColor  = colorf::RGBA(112, 112, 112, 255);

            Widget::VarU32 arrowColor    = colorf::RGBA( 96,  96,  96, 255);
            Widget::VarU32 arrowBoxColor = colorf::RGBA(232, 232, 232, 255);

            Widget::InstAttrs attrs  {};
            Widget::WADPair   parent {};
        };

    private:
        enum DragMode : uint8_t
        {
            DRAG_NONE    = 0,
            DRAG_V_THUMB = 1,
            DRAG_H_THUMB = 2,
        };

    private:
        GfxCropBoard *m_viewport; // owned via childList, drawn/handled through the standard tree

    private:
        Widget::VarBool m_hScroll;
        Widget::VarBool m_vScroll;

    private:
        int m_barSize;
        int m_minThumbSize;
        int m_scrollStep;

    private:
        Widget::VarU32 m_trackColor;

    private:
        Widget::VarU32 m_thumbColor;
        Widget::VarU32 m_thumbHoverColor;
        Widget::VarU32 m_thumbDragColor;

    private:
        Widget::VarU32 m_arrowColor;
        Widget::VarU32 m_arrowBoxColor;

    private:
        int m_scrollX = 0;
        int m_scrollY = 0;

    private:
        DragMode m_drag = DRAG_NONE;
        int m_dragMouseStart = 0;
        int m_dragScrollStart = 0;

    public:
        explicit ScrollContainer(ScrollContainer::InitArgs);

    public:
        auto contained(this auto && self)
        {
            return self.m_viewport->gfxWidget();
        }

    public:
        int scrollX() const { return m_scrollX; }
        int scrollY() const { return m_scrollY; }

        void scrollTo(int, int);
        void scrollBy(int dx, int dy) { scrollTo(m_scrollX + dx, m_scrollY + dy); }

    public:
        bool hBarEnabled() const;
        bool vBarEnabled() const;

    public:
        int viewportW() const { return vBarEnabled() ? std::max<int>(0, w() - m_barSize) : w(); }
        int viewportH() const { return hBarEnabled() ? std::max<int>(0, h() - m_barSize) : h(); }

    public:
        int maxScrollX() const { return std::max<int>(0, (contained() ? contained()->w() : 0) - viewportW()); }
        int maxScrollY() const { return std::max<int>(0, (contained() ? contained()->h() : 0) - viewportH()); }

    public:
        void setHScroll(Widget::VarBool arg) { m_hScroll = std::move(arg); }
        void setVScroll(Widget::VarBool arg) { m_vScroll = std::move(arg); }

    public:
        void drawDefault(Widget::ROIMap) const override;
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    private:
        Widget::ROI vBarROI() const;
        Widget::ROI hBarROI() const;

        Widget::ROI    vUpArrowROI() const;
        Widget::ROI  vDownArrowROI() const;

        Widget::ROI  hLeftArrowROI() const;
        Widget::ROI hRightArrowROI() const;

        Widget::ROI vTrackROI() const;
        Widget::ROI hTrackROI() const;
        Widget::ROI vThumbROI() const;
        Widget::ROI hThumbROI() const;
};
