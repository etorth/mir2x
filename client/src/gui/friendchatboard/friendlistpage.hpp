#pragma once
#include <functional>
#include <utility>
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "frienditem.hpp"

struct FriendListPage: public Widget
{
    Widget canvas;
    FriendListPage(Widget::VarDir,
            Widget::VarInt,
            Widget::VarInt,

            Widget::VarOptSize,
            Widget::VarOptSize,

            Widget * = nullptr,
            bool     = false);

    void append(const SDChatPeer &, std::function<void(FriendItem *)> = nullptr, std::pair<Widget *, bool> = {});
};
