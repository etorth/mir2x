#pragma once
#include <cstdint>
#include <optional>
#include <functional>
#include "widget.hpp"
#include "imageboard.hpp"
#include "labelboard.hpp"
#include "chatitemref.hpp"
#include "shapeclipboard.hpp"
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
    constexpr static int ITEM_SPACE = 5;  // space between two items
    constexpr static int NAME_HEIGHT = 20;

    constexpr static int TRIANGLE_WIDTH  = 4;
    constexpr static int TRIANGLE_HEIGHT = 6;

    constexpr static int MAX_WIDTH = UIPage_MIN_WIDTH - UIPage_MARGIN * 2 - ChatItem::TRIANGLE_WIDTH - ChatItem::GAP - ChatItem::AVATAR_WIDTH;

    constexpr static int MESSAGE_MARGIN = 5;
    constexpr static int MESSAGE_CORNER = 3;

    constexpr static int MESSAGE_MIN_WIDTH  = 10; // handling small size message
    constexpr static int MESSAGE_MIN_HEIGHT = 10;

    constexpr static int REF_GAP = 10;

    bool pending = true;
    double accuTime = 0.0;

    const bool showName;
    const bool avatarLeft;
    const std::optional<uint32_t> bgColor;

    ImageBoard avatar;
    LabelBoard name;

    LayoutBoard    message;
    ShapeClipBoard background;

    ChatItemRef * const msgref = nullptr;

    ChatItem(Widget::VarDir,
            Widget::VarOff,
            Widget::VarOff,

            bool,

            const char8_t *,
            const char8_t *,
            const char8_t *,

            std::function<SDL_Texture *(const ImageBoard *)>,

            bool,
            bool,
            std::optional<uint32_t>,

            Widget * = nullptr,
            bool     = false);

    void update(double) override;
    bool processEventDefault(const SDL_Event &, bool) override;
};
