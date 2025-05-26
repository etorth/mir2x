#pragma once
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "labelboard.hpp"
#include "layoutboard.hpp"
#include "imageboard.hpp"
#include "shapeclipboard.hpp"
#include "friendchatboardconst.hpp"

struct ChatPreviewItem: public Widget
{
    constexpr static int HEIGHT = 50;

    constexpr static int ITEM_MARGIN = 5;
    constexpr static int GAP = 10;

    constexpr static int NAME_HEIGHT = 24;
    constexpr static int AVATAR_WIDTH = (HEIGHT - ITEM_MARGIN * 2) * 84 / 94; // original avatar size: 84 x 94

    // ITEM_MARGIN   GAP
    //   ->|  |<-   |<->|
    //     +------------------------------+ -                           -
    //     |                              | | ITEM_MARGIN               |
    //     |  +-+---+   +------+          | -             -             |
    //     |  |1|   |   | name |          |               | NAME_HEIGHT |
    //     |  +-+   |   +------+          |               -             | HEIGHT
    //     |  | IMG |   +--------------+  |                             |
    //     |  |     |   |latest message|  |                             |
    //     |  +-----+   +--------------+  |                             |
    //     |                              |                             |
    //     +------------------------------+                             -
    //
    //        |<--->|
    //      AVATAR_WIDTH
    //
    //     |<---------------------------->| UIPage_MIN_WIDTH - UIPage_MARGIN * 2
    //
    const SDChatPeerID cpid;

    ImageBoard avatar;
    LabelBoard name;

    LayoutBoard message;
    Widget      messageClip;

    ShapeClipBoard selected;

    ChatPreviewItem(
            Widget::VarDir,
            Widget::VarOff,
            Widget::VarOff,
            Widget::VarSize,

            const SDChatPeerID &,
            const char8_t *,

            Widget * = nullptr,
            bool     = false);

    bool processEventDefault(const SDL_Event &, bool) override;
};
