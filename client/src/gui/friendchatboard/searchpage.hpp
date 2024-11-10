#include <string>
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "layoutboard.hpp"
#include "searchinputline.hpp"
#include "friendchatboardconst.hpp"

struct SearchPage: public Widget
{
    //                  -->| |<-- CLEAR_GAP
    // |<----------WIDTH----------->|
    // +-------------------+ +------+
    // |      INPUT        | | 清空 |
    // +-------------------+ +------+
    // | auto completion item       |
    // +----------------------------+
    // | auto completion item       |
    // +----------------------------+

    constexpr static int WIDTH  = UIPage_MIN_WIDTH  - UIPage_MARGIN * 2;
    constexpr static int HEIGHT = UIPage_MIN_HEIGHT - UIPage_MARGIN * 2;

    constexpr static int CLEAR_GAP = 10;

    SearchInputLine input;
    LayoutBoard clear;

    Widget autocompletes;
    Widget candidates;

    SearchPage(Widget::VarDir,

            Widget::VarOff,
            Widget::VarOff,

            Widget * = nullptr,
            bool     = false);

    void appendFriendItem(const SDChatPeer &);
    void appendAutoCompletionItem(bool, const SDChatPeer &, const std::string &);
};
