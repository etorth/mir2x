#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "chatitem.hpp"
#include "labelboard.hpp"
#include "layoutboard.hpp"
#include "marginwrapper.hpp"
#include "shapeclipboard.hpp"

struct ChatItemContainer: public Widget
{
    constexpr static int BACKGROUND_MARGIN = 3;
    constexpr static int BACKGROUND_CORNER = 4;

    // use canvas to hold all chat item
    // then we can align canvas always to buttom when needed
    //
    // when scroll we can only move canvas inside this container
    // no need to move chat item one by one
    //
    // canvas height is flexible
    // ShapeClipBoard can achieve this on drawing, but prefer ShapeClipBoard when drawing primitives

    Widget canvas;

    LabelBoard nomsg; // show when there is no chat message
    LayoutBoard ops;  // block strangers, add friends, etc

    MarginWrapper nomsgWrapper;
    MarginWrapper opsWrapper;

    ChatItemContainer(
            Widget::VarDir,
            Widget::VarOff,
            Widget::VarOff,

            Widget::VarSize,
            Widget::VarSize,

            Widget * = nullptr,
            bool     = false);

    void clearChatItem();
    int  chatItemMaxWidth() const;
    void append(const SDChatMessage &, std::function<void(const ChatItem *)>);

    bool hasChatItem() const;
    const ChatItem *lastChatItem() const;
    const SDChatPeer &getChatPeer() const;
};
