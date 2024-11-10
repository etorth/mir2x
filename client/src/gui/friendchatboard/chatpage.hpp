#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "menuboard.hpp"
#include "shapeclipboard.hpp"
#include "chatinputcontainer.hpp"
#include "chatitemcontainer.hpp"

struct ChatPage: public Widget
{
    // chat page is different, it uses the UIPage_MARGIN area
    // because we fill different color to chat area and input area
    //
    //         |<----- UIPage_MIN_WIDTH ------>|
    //       ->||<---- UIPage_MARGIN                     v
    //       - +---------------------------+             -
    //       ^ |+-------------------------+|           - -
    //       | || +------+                ||           ^ ^
    //       | || |******|                ||           | |
    //       | || +------+                ||           | + UIPage_MARGIN
    //       | ||                +------+ ||           |
    //  U    | ||                |******| ||           |
    //  I    | ||                +------+ ||           |
    //  P    | || +------------+          ||           +-- UIPage_MIN_HEIGHT - UIPage_MARGIN * 4 - INPUT_MARGIN * 2 - input.h() - 1
    //  a    | || |************|          ||           |
    //  g ---+ || |*****       |          ||           |
    //  e    | || +------------+          ||           |
    //  |    | ||                         ||           |
    //  H    | ||       chat area         ||         | |
    //  E    | ||                         ||         v v
    //  I    | |+-------------------------+|       | - -
    //  G    | +===========================+       v   UIPage_MARGIN * 2 + 1
    //  H    | |  +---------------------+  |       - -
    //  T    | | / +-------------------+ \ |     - -<- INPUT_MARGIN
    //       | ||  |*******************|  ||     ^ ^
    //       | ||  |****input area*****|  ||   | +---- input.h()
    //       | ||  |*******************|  || | v v
    //       | | \ +-------------------+ / | v - -
    //       v |  +---------------------+  | - -
    //       - +---------------------------+ - ^
    //       ->||<---- UIPage_MARGIN         ^ |
    //       -->| |<--  INPUT_CORNER         | +------  INPUT_MARGIN
    //       -->|  |<-  INPUT_MARGIN         +-------- UIPage_MARGIN
    //             |<--- input.w() --->|

    constexpr static int INPUT_CORNER = 8;
    constexpr static int INPUT_MARGIN = 8;

    constexpr static int INPUT_MIN_HEIGHT =  10;
    constexpr static int INPUT_MAX_HEIGHT = 200;

    SDChatPeer peer;
    ShapeClipBoard background;

    ChatInputContainer input;
    ChatItemContainer  chat;

    MenuBoard *menu = nullptr;

    ChatPage(dir8_t,

            int,
            int,

            Widget * = nullptr,
            bool     = false);

    bool processEventDefault(const SDL_Event &, bool) override;
};

