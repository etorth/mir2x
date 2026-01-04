#pragma once
#include <cstdint>
#include <optional>
#include <functional>
#include "widget.hpp"
#include "imageboard.hpp"
#include "labelboard.hpp"
#include "chatitemref.hpp"
#include "gfxshapeboard.hpp"
#include "friendchatboardconst.hpp"

//            WIDTH
// |<----------------------->|
//       GAP
//     ->| |<-
// +-----+     +--------+      -
// |     |     |  name  |      | NAME_HEIGHT
// |     |     +--------+      -
// |     |     /-------------\             <----+
// | IMG |     | ........... |                  |
// |     |     | ........... |                  |
// |     |    /  ........... |                  |
// |     |  <    ........... |                  |
// |     |    \  ........... |                  |
// |     |  ^  | ........... |                  | background includes messsage round-corner background box and the triangle area
// +-----+  |  | ........... |                  |
//          |  | ........... |                  |
//          |  \-------------/<- MESSAGE_CORNER |  -
//          |            ->| |<-                |  ^
//          |             MESSAGE_MARGIN        |  |
//          +-----------------------------------+  +-- REF_GAP
//                                                 |
//             /---------\                         -
//             | msg ref |
//             \---------/
//
//
//            -->|  |<-- TRIANGLE_WIDTH
//                2 +                + 2                    -
//      -----+     /|                |\     +-----          ^
//           |    / |                | \    |               |
//    avatar | 1 +  |                |  + 1 | avatar        | TRIANGLE_HEIGHT
//           |    \ |                | /    |               |
//      -----+     \|                |/     +-----          v
//                3 +                + 3                    -
//           |<->| GAP                  |<->| GAP
//               ^                      ^
//               |                      |
//               +-- background startX  +-- background endX

struct ChatItem: public Widget
{
    constexpr static int AVATAR_WIDTH  = 35;
    constexpr static int AVATAR_HEIGHT = AVATAR_WIDTH * 94 / 84;

    constexpr static int GAP = 5;
    constexpr static int NAME_HEIGHT = 20;

    constexpr static int TRIANGLE_WIDTH  = 4;
    constexpr static int TRIANGLE_HEIGHT = 6;

    constexpr static int MESSAGE_MARGIN = 5;
    constexpr static int MESSAGE_CORNER = 3;

    constexpr static int MESSAGE_MIN_WIDTH  = 10; // handling small size message
    constexpr static int MESSAGE_MIN_HEIGHT = 10;

    constexpr static int REF_GAP = 10;

    struct InitArgs
    {
        Widget::VarDir dir = DIR_UPLEFT;

        Widget::VarInt x = 0;
        Widget::VarInt y = 0;

        int  maxWidth = 100;
        bool pending  = true;

        std::optional<uint64_t>    msgID = std::nullopt;
        std::optional<uint64_t> msgRefID = std::nullopt;

        const char8_t *name       = nullptr;
        const char8_t *message    = nullptr;
        const char8_t *messageRef = nullptr;

        Widget::VarTexLoadFunc texLoadFunc = nullptr;
        Widget::VarU32Opt      bgColor     = std::nullopt;

        bool showName   = true;
        bool avatarLeft = true;

        Widget::WADPair parent {};
    };

    bool pending = true;
    double accuTime = 0.0;

    // nullopt when message id is pending
    // also used to support fake chat message that has no message id
    std::optional<uint64_t> msgID = std::nullopt;

    const bool showName;
    const bool avatarLeft;
    const Widget::VarU32Opt bgColor;

    ImageBoard avatar;
    LabelBoard name;

    LayoutBoard message;
    GfxShapeBoard background;

    ChatItemRef * const msgref = nullptr;

    ChatItem(ChatItem::InitArgs);

    void setMaxWidth(int);
    void updateDefault(double) override;
    bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;
};
