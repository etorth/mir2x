#pragma once
#include <array>
#include <string>
#include <optional>
#include <unordered_map>
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "menuboard.hpp"
#include "chatitemref.hpp"
#include "shapecropboard.hpp"
#include "chatinputcontainer.hpp"
#include "chatitemcontainer.hpp"

struct ChatPage: public Widget
{
    // chat page is different, it uses the UIPage_MARGIN area
    // because we fill different color to chat area and input area
    //
    //         |<--- UIPage_MIN_WIDTH ---->|
    //       ->||<-- UIPage_MARGIN                       v
    //       - +---------------------------+             -
    //       ^ |+-------------------------+|           - -
    //       | || +------+                ||           ^ ^
    //       | || |******|                ||           | |
    //       | || +------+                ||           | + UIPage_MARGIN
    //       | ||                +------+ ||           |
    //  U    | ||                |******| ||           |
    //  I    | ||                +------+ ||           |
    //  P    | || +------------+          ||           +-- UIPage_MIN_HEIGHT
    //  a    | || |************|          ||           |         - UIPage_MARGIN * 2            // top & bottom margin
    //  g ---+ || |*****       |          ||           |         -    SEP_MARGIN * 2 - 1        // middle area between chat area and input area
    //  e    | || +------------+          ||           |         -  INPUT_MARGIN * 2            //
    //  |    | ||                         ||           |         - input.h()                    //
    //  H    | ||       chat area         ||         | |         - (showref() ? (chatref->h() + CHATREF_GAP) : 0)
    //  E    | ||                         ||         v v
    //  I    | |+-------------------------+|       | - -
    //  G    | +===========================+       v   SEP_MARGIN * 2 + 1
    //  H    | |  +---------------------+  |       - -
    //  T    | | / +-------------------+ \ |     - -<- INPUT_MARGIN
    //       | ||  |*******************|  ||     ^ ^
    //       | ||  |****input area*****|  ||   | +---- input.h()
    //       | ||  |*******************|  || | v v
    //       | | \ +-------------------+ / | v - -                +--- showref() ? chatref->h() : 0
    //       | |  +---------------------+  | - -                  |
    //       | |                           |   ^                  v
    //       | |+-------------------------+| - |                  -
    //       | ||        ChatRef      (x) || ^ +-- INPUT_MARGIN
    //       v |+-------------------------+| |                    -
    //       - +---------------------------+ +---- showref() ? CHATREF_GAP : 0
    //       ->||<---- UIPage_MARGIN
    //       -->| |<--  INPUT_CORNER
    //       -->|  |<-  INPUT_MARGIN
    //             |<--- input.w() --->|

    constexpr static int SEP_MARGIN = 2;

    constexpr static int INPUT_CORNER = 8;
    constexpr static int INPUT_MARGIN = 8;

    constexpr static int INPUT_MIN_HEIGHT =  10;
    constexpr static int INPUT_MAX_HEIGHT = 200;

    constexpr static int CHATREF_GAP = 5;

    SDChatPeer peer;
    ShapeCropBoard background;

    ChatItemRef *chatref = nullptr;

    ChatInputContainer input;
    ChatItemContainer  chat;

    MenuBoard *menu = nullptr;

    ChatPage(
            Widget::VarDir,
            Widget::VarInt,
            Widget::VarInt,
            Widget::VarSizeOpt,
            Widget::VarSizeOpt,

            Widget * = nullptr,
            bool     = false);

    bool showref() const;
    bool showmenu() const;

    std::optional<uint64_t> refopt() const;

    void  enableChatRef(uint64_t, std::string);
    void disableChatRef();

    static ChatItemRef *createChatItemRef(uint64_t, std::string, Widget *, bool);

    void afterResizeDefault() override;
    bool processEventDefault(const SDL_Event &, bool, int, int, const Widget::ROIOpt &) override;
};
