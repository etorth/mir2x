#pragma once
#include <algorithm>
#include <utility>
#include "widget.hpp"
#include "colorf.hpp"
#include "gfxcropboard.hpp"

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

            // external content widget — not owned by ScrollContainer
            Widget::VarGetter<Widget *> getter = nullptr;

            // enable/disable each axis independently
            //   - true  : axis is scrollable, bar shown when content overflows
            //   - false : axis is locked, bar force-hidden, scroll input on that axis rejected
            Widget::VarBool hScroll = true;
            Widget::VarBool vScroll = true;

            int barSize      = 12; // px thickness of each scrollbar strip
            int minThumbSize = 20; // px min thumb length
            int scrollStep   = 24; // px per wheel notch / arrow click

            Widget::VarDrawFunc bgDrawFunc = nullptr; // painted under the content, inside the viewport rect
            Widget::VarDrawFunc fgDrawFunc = nullptr; // painted over  the content, inside the viewport rect (below the scrollbars)

            Widget::VarU32 bgColor         = 0u; // 0 = don't fill
            Widget::VarU32 trackColor      = colorf::RGBA(232, 232, 232, 255);
            Widget::VarU32 thumbColor      = colorf::RGBA(176, 176, 176, 255);
            Widget::VarU32 thumbHoverColor = colorf::RGBA(144, 144, 144, 255);
            Widget::VarU32 thumbDragColor  = colorf::RGBA(112, 112, 112, 255);
            Widget::VarU32 arrowBoxColor   = colorf::RGBA(232, 232, 232, 255);
            Widget::VarU32 arrowColor      = colorf::RGBA( 96,  96,  96, 255);

            Widget::InstAttrs attrs {};
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
        Widget::VarU32 m_bgColor;
        Widget::VarU32 m_trackColor;
        Widget::VarU32 m_thumbColor;
        Widget::VarU32 m_thumbHoverColor;
        Widget::VarU32 m_thumbDragColor;
        Widget::VarU32 m_arrowBoxColor;
        Widget::VarU32 m_arrowColor;

    private:
        int m_scrollX = 0;
        int m_scrollY = 0;

    private:
        DragMode m_drag           = DRAG_NONE;
        int      m_dragMouseStart = 0;
        int      m_dragScrollStart = 0;

    public:
        explicit ScrollContainer(ScrollContainer::InitArgs);

    public:
        Widget *content()
        {
            return m_viewport->gfxWidget();
        }

        const Widget *content() const
        {
            return m_viewport->gfxWidget();
        }

    public:
        int scrollX() const { return m_scrollX; }
        int scrollY() const { return m_scrollY; }

        void scrollTo(int, int);
        void scrollBy(int dx, int dy) { scrollTo(m_scrollX + dx, m_scrollY + dy); }

    public:
        int contentW() const { return content() ? content()->w() : 0; }
        int contentH() const { return content() ? content()->h() : 0; }

    public:
        // effective: allowed AND content overflows on that axis
        bool hBarEnabled() const;
        bool vBarEnabled() const;

    public:
        int viewportW() const { return vBarEnabled() ? std::max<int>(0, w() - m_barSize) : w(); }
        int viewportH() const { return hBarEnabled() ? std::max<int>(0, h() - m_barSize) : h(); }

    public:
        int maxScrollX() const { return std::max<int>(0, contentW() - viewportW()); }
        int maxScrollY() const { return std::max<int>(0, contentH() - viewportH()); }

    public:
        void setHScroll(Widget::VarBool arg) { m_hScroll = std::move(arg); }
        void setVScroll(Widget::VarBool arg) { m_vScroll = std::move(arg); }

    public:
        void drawDefault(Widget::ROIMap) const override;
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    private:
        // scrollbar geometry in local coords (returns empty ROI when the bar is disabled)
        Widget::ROI vBarROI() const;      // full vertical strip (including arrow boxes)
        Widget::ROI hBarROI() const;
        Widget::ROI vUpArrowROI() const;
        Widget::ROI vDownArrowROI() const;
        Widget::ROI hLeftArrowROI() const;
        Widget::ROI hRightArrowROI() const;
        Widget::ROI vTrackROI() const;    // between the two arrow boxes
        Widget::ROI hTrackROI() const;
        Widget::ROI vThumbROI() const;    // proportional thumb within vTrackROI
        Widget::ROI hThumbROI() const;
};
