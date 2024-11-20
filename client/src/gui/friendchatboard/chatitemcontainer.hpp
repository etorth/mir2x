#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "itemflex.hpp"
#include "chatitem.hpp"
#include "labelboard.hpp"
#include "layoutboard.hpp"
#include "margincontainer.hpp"

struct ChatItemContainer: public Widget
{
    constexpr static int ITEM_SPACE = 5;
    constexpr static int BACKGROUND_MARGIN = 3;
    constexpr static int BACKGROUND_CORNER = 4;

    // use canvas to hold all chat item
    // then we can align canvas always to buttom when needed
    //
    // when scroll we can only move canvas inside this container
    // no need to move chat item one by one

    ItemFlex canvas;

    LabelBoard nomsg; // show when there is no chat message
    LayoutBoard ops;  // all kinds of ops, including block strangers, add friends, etc

    MarginContainer nomsgBox;
    MarginContainer   opsBox;

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
    const SDChatPeer &getChatPeer() const;
};
