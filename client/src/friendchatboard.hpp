#pragma once
#include <array>
#include <unordered_map>
#include "widget.hpp"
#include "serdesmsg.hpp"
#include "texslider.hpp"
#include "inputline.hpp"
#include "labelboard.hpp"
#include "layoutboard.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"
#include "shapeclipboard.hpp"
#include "gfxcropdupboard.hpp"

class ProcessRun;
class FriendChatBoard: public Widget
{
    constexpr static int UIPage_WIDTH  = 400; // the area excludes border area, margin included
    constexpr static int UIPage_HEIGHT = 400;
    constexpr static int UIPage_MARGIN =   4;

    private:
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
            //   |<------------------------->| UIPage_WIDTH - UIPage_MARGIN * 2

            constexpr static int HEIGHT = 40;
            constexpr static int ITEM_MARGIN = 3;
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
            bool processEvent(const SDL_Event &, bool) override;
        };

        struct FriendListPage: public Widget
        {
            Widget canvas;
            FriendListPage(Widget::VarDir,
                    Widget::VarOffset,
                    Widget::VarOffset,

                    Widget * = nullptr,
                    bool     = false);

            void append(const SDChatPeer &, std::function<void(FriendItem *)> = nullptr, std::pair<Widget *, bool> = {});
        };

        struct SearchInputLine: public Widget
        {
            // o: (0,0)
            // x: (3,3) : fixed by gfx resource border
            //
            //   o==========================+ -
            //   | x+---+ +---------------+ | ^
            //   | ||O、| |xxxxxxx        | | | HEIGHT
            //   | ++---+ +---------------+ | v
            //   +==========================+ -
            //
            //   |<------- WIDTH --------->|
            //      |<->|
            //   ICON_WIDTH
            //
            //   ->||<- ICON_MARGIN
            //
            //       -->| |<-- GAP

            constexpr static int WIDTH = UIPage_WIDTH - UIPage_MARGIN * 2 - 60;
            constexpr static int HEIGHT = 30;

            constexpr static int ICON_WIDTH = 20;
            constexpr static int ICON_MARGIN = 5;
            constexpr static int GAP = 5;

            ImageBoard image;
            GfxCropDupBoard inputbg;

            ImageBoard icon;
            InputLine  input;
            LabelBoard hint;

            SearchInputLine(Widget::VarDir,

                    Widget::VarOffset,
                    Widget::VarOffset,

                    Widget * = nullptr,
                    bool     = false);
        };

        struct SearchAutoCompletionItem: public Widget
        {
            // o: (0,0)
            // x: (3,3)
            //
            //   o--------------------------+ -
            //   | x+---+ +---------------+ | ^
            //   | ||O、| |label          | | | HEIGHT
            //   | ++---+ +---------------+ | v
            //   +--------------------------+ -
            //   |<-------- WIDTH --------->|
            //      |<->|
            //   ICON_WIDTH
            //
            //   ->||<- ICON_MARGIN
            //
            //       -->| |<-- GAP

            constexpr static int WIDTH = UIPage_WIDTH - UIPage_MARGIN * 2;
            constexpr static int HEIGHT = 30;

            constexpr static int ICON_WIDTH = 20;
            constexpr static int ICON_MARGIN = 5;
            constexpr static int GAP = 5;

            const bool byID; // when clicked, fill input automatically by ID if true, or name if false
            const SDChatPeer candidate;

            ShapeClipBoard background;

            ImageBoard icon;
            LabelBoard label;

            SearchAutoCompletionItem(Widget::VarDir,

                    Widget::VarOffset,
                    Widget::VarOffset,

                    bool,
                    SDChatPeer,

                    const char * = nullptr,

                    Widget * = nullptr,
                    bool     = false);

            bool processEvent(const SDL_Event &, bool) override;
        };

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

            constexpr static int WIDTH  = UIPage_WIDTH  - UIPage_MARGIN * 2;
            constexpr static int HEIGHT = UIPage_HEIGHT - UIPage_MARGIN * 2;

            constexpr static int CLEAR_GAP = 10;

            SearchInputLine input;
            LayoutBoard clear;

            Widget autocompletes;
            Widget candidates;

            SearchPage(Widget::VarDir,

                    Widget::VarOffset,
                    Widget::VarOffset,

                    Widget * = nullptr,
                    bool     = false);

            void appendFriendItem(const SDChatPeer &);
            void appendAutoCompletionItem(bool, const SDChatPeer &, const std::string &);
        };

        struct ChatItem: public Widget
        {
            //          WIDTH
            // |<------------------->|
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
            //          |  \-------------/<- MESSAGE_CORNER |
            //          |            ->| |<-                |
            //          |             MESSAGE_MARGIN        |
            //          +-----------------------------------+
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
            //           |<---->|                |<---->|
            //           ^  GAP                     GAP ^
            //           |                              |
            //           +-- startX of background       +-- endX of background

            constexpr static int AVATAR_WIDTH  = 35;
            constexpr static int AVATAR_HEIGHT = AVATAR_WIDTH * 94 / 84;

            constexpr static int GAP = 5;
            constexpr static int ITEM_SPACE = 5;  // space between two items
            constexpr static int NAME_HEIGHT = 20;

            constexpr static int TRIANGLE_WIDTH  = 4;
            constexpr static int TRIANGLE_HEIGHT = 6;

            constexpr static int MAX_WIDTH = UIPage_WIDTH - UIPage_MARGIN * 2 - ChatItem::TRIANGLE_WIDTH - ChatItem::GAP - ChatItem::AVATAR_WIDTH;

            constexpr static int MESSAGE_MARGIN = 5;
            constexpr static int MESSAGE_CORNER = 3;

            constexpr static int MESSAGE_MIN_WIDTH  = 10; // handling small size message
            constexpr static int MESSAGE_MIN_HEIGHT = 10;

            bool pending = true;
            double accuTime = 0.0;

            const bool showName;
            const bool avatarLeft;
            const std::optional<uint32_t> bgColor;

            ImageBoard avatar;
            LabelBoard name;

            LayoutBoard    message;
            ShapeClipBoard background;

            ChatItem(dir8_t,
                    int,
                    int,

                    bool,

                    const char8_t *,
                    const char8_t *,

                    std::function<SDL_Texture *(const ImageBoard *)>,

                    bool,
                    bool,
                    std::optional<uint32_t>,

                    Widget * = nullptr,
                    bool     = false);

            void update(double) override;
        };

        struct ChatItemContainer: public Widget
        {
            struct BackgroundWrapper: public Widget
            {
                Widget * const gfxWidget;
                ShapeClipBoard background;

                BackgroundWrapper(dir8_t,
                        int, // x
                        int, // y
                        int, // margin
                        int, // corner

                        Widget *, // holding widget
                                  // widget should have been initialized

                        Widget * = nullptr,
                        bool     = false);

                bool processEvent(const SDL_Event &event, bool valid) override
                {
                    return gfxWidget->processEvent(event, valid);
                }
            };

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

            BackgroundWrapper nomsgWrapper;
            BackgroundWrapper opsWrapper;

            ChatItemContainer(dir8_t,

                    int,
                    int,

                    Widget::VarSize,

                    Widget * = nullptr,
                    bool     = false);


            void clearChatItem();
            void append(const SDChatMessage &, std::function<void(const ChatItem *)>);

            bool hasChatItem() const;
            const ChatItem *lastChatItem() const;
            const SDChatPeer &getChatPeer() const;
        };

        struct ChatInputContainer: public Widget
        {
            LayoutBoard layout;
            ChatInputContainer(dir8_t,

                    int,
                    int,

                    Widget * = nullptr,
                    bool     = false);
        };

        struct ChatPage: public Widget
        {
            // chat page is different, it uses the UIPage_MARGIN area
            // because we fill different color to chat area and input area
            //
            //         |<----- UIPage_WIDTH ------>|
            //       ->||<---- UIPage_MARGIN                     v
            //       - +---------------------------+             -
            //       ^ |+-------------------------+|           - -
            //       | || +------+                ||           ^ ^
            //       | || |******|                ||           | |
            //       | || +------+                ||           | + UIPage_MARGIN
            //       | ||                +------+ ||           |
            //  U    | ||                |******| ||           |
            //  I    | ||                +------+ ||           |
            //  P    | || +------------+          ||           +-- UIPage_HEIGHT - UIPage_MARGIN * 4 - INPUT_MARGIN * 2 - input.h() - 1
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

            ChatPage(dir8_t,

                    int,
                    int,

                    Widget * = nullptr,
                    bool     = false);

            bool processEvent(const SDL_Event &, bool) override;
        };

        struct ChatPreviewItem: public Widget
        {
            constexpr static int WIDTH  = UIPage_WIDTH - UIPage_MARGIN * 2;
            constexpr static int HEIGHT = 50;

            constexpr static int GAP = 10;
            constexpr static int NAME_HEIGHT = 30;
            constexpr static int AVATAR_WIDTH = HEIGHT * 84 / 94; // original avatar size: 84 x 94

            //        GAP
            //       |<->|
            // +-+---+  +------+          -             -
            // |1|   |  | name |          | NAME_HEIGHT ^
            // +-+   |  +------+          -             | HEIGHT
            // | IMG |  +--------------+                |
            // |     |  |latest message|                v
            // +-----+  +--------------+                -
            //
            // |<--->|
            // AVATAR_WIDTH
            //
            // |<--------------------->|
            //           WIDTH

            const SDChatPeerID cpid;

            ImageBoard  avatar;
            LabelBoard  name;
            LayoutBoard message;

            ShapeClipBoard preview;
            ShapeClipBoard selected;

            ChatPreviewItem(dir8_t,
                    int,
                    int,

                    const SDChatPeerID &,
                    const char8_t *,

                    Widget * = nullptr,
                    bool     = false);

            bool processEvent(const SDL_Event &, bool) override;
        };

        struct ChatPreviewPage: public Widget
        {
            Widget canvas;
            ChatPreviewPage(Widget::VarDir,

                    Widget::VarOffset,
                    Widget::VarOffset,

                    Widget * = nullptr,
                    bool     = false);

            void updateChatPreview(const SDChatPeerID &, const std::string &);
        };

    private:
        struct PageControl: public Widget
        {
            PageControl(dir8_t,

                    int,
                    int,

                    int,

                    std::initializer_list<std::pair<Widget *, bool>>,

                    Widget * = nullptr,
                    bool     = false);
        };

    public:
        enum UIPageType: int
        {
            UIPage_CHAT = 0,
            UIPage_CHATPREVIEW,
            UIPage_FRIENDLIST,
            UIPage_FRIENDSEARCH,
            UIPage_CREATEGROUP,
            UIPage_END,
        };

    private:
        struct UIPage
        {
            LabelBoard * const title   = nullptr;
            Widget     * const control = nullptr;
            TexSlider  * const slider  = nullptr;
            Widget     * const page    = nullptr;

            std::function<void(int, UIPage *)> enter = nullptr;
            std::function<void(int, UIPage *)> exit  = nullptr;
        };

    private:
        static constexpr int UIPage_BORDER[4]
        {
            54,
            10,
            13,
            38,
        };

    private:
        struct FriendMessage
        {
            SDChatPeerID cpid;
            size_t unread = 0;
            std::vector<SDChatMessage> list;
        };

    private:
        ProcessRun *m_processRun;

    private:
        ImageBoard m_frame;
        GfxCropDupBoard m_frameCropDup;

    private:
        ImageBoard m_background;
        GfxCropDupBoard m_backgroundCropDup;

    private:
        TritexButton m_close;

    private:
        int m_uiLastPage = UIPage_CHATPREVIEW;
        int m_uiPage     = UIPage_CHATPREVIEW;
        std::array<FriendChatBoard::UIPage, UIPage_END> m_uiPageList; // {buttons, page}

    private:
        SDFriendList m_sdFriendList;
        std::list<SDChatPeer> m_cachedChatPeerList;

    private:
        std::unordered_map<uint64_t, SDChatMessage> m_localMessageList;
        std::list<FriendMessage> m_friendMessageList;

    public:
        FriendChatBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    public:
        const SDChatPeer *findChatPeer      (const SDChatPeerID &) const;
        const SDChatPeer *findFriendChatPeer(const SDChatPeerID &) const;

    private:
        void queryChatPeer(const SDChatPeerID &, std::function<void(const SDChatPeer *, bool /* async */)>);

    public:
        void addMessage(std::optional<uint64_t>, const SDChatMessage &);
        void addMessagePending(uint64_t, const SDChatMessage &);

    public:
        void finishMessagePending(size_t, const SDChatMessageDBSeq &);

    public:
        void setFriendList(const SDFriendList &);

    public:
        void setChatPeer(const SDChatPeer &, bool);
        void setUIPage(int);

    public:
        void loadChatPage();

    public:
        static       FriendChatBoard *getParentBoard(      Widget *);
        static const FriendChatBoard *getParentBoard(const Widget *);

    public:
        void addGroup(const SDChatPeer &);
        void addFriendListChatPeer(const SDChatPeerID &);

    public:
        void requestAddFriend(const SDChatPeer &);
};
