#pragma once
#include <functional>
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "labelboard.hpp"
#include "layoutboard.hpp"
#include "imageboard.hpp"
#include "shapeclipboard.hpp"

struct FriendItem: public Widget
{
    //   ITEM_MARGIN                    | ITRM_MARGIN
    // ->| |<-                          v
    //   +---------------------------+ - -
    //   | +-----+                   | ^ -
    //   | |     | +------+ +------+ | | ^
    //   | | IMG | | NAME | | FUNC | | | HEIGHT
    //   | |     | +------+ +------+ | |
    //   | +-----+                   | v
    //   +---------------------------+ -
    //         ->| |<-          -->| |<-- FUNC_MARGIN
    //           GAP
    //   |<------------------------->| UIPage_MIN_WIDTH - UIPage_MARGIN * 2

    constexpr static int HEIGHT = 40;
    constexpr static int ITEM_MARGIN = 5;
    constexpr static int AVATAR_WIDTH = (HEIGHT - ITEM_MARGIN * 2) * 84 / 94;

    constexpr static int GAP = 5;
    constexpr static int FUNC_MARGIN = 5;

    SDChatPeerID cpid;

    uint64_t funcWidgetID;
    std::function<void(FriendItem *)> onClick;

    ShapeClipBoard hovered;

    ImageBoard avatar;
    LabelBoard name;

    FriendItem(dir8_t,
            int,
            int,

            const SDChatPeerID &,

            const char8_t *,
            std::function<SDL_Texture *(const ImageBoard *)>,

            std::function<void(FriendItem *)> = nullptr,
            std::pair<Widget *, bool> argFuncWidget = {},

            Widget * = nullptr,
            bool     = false);

    void setFuncWidget(Widget *, bool);
    bool processEventDefault(const SDL_Event &, bool) override;
};

