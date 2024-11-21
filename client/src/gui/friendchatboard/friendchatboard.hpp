#pragma once
#include <array>
#include <string>
#include <optional>
#include <unordered_map>
#include "serdesmsg.hpp"
#include "widget.hpp"
#include "texslider.hpp"
#include "labelboard.hpp"
#include "imageboard.hpp"
#include "tritexbutton.hpp"
#include "gfxcropdupboard.hpp"
#include "friendchatboardconst.hpp"

class ProcessRun;
class FriendChatBoard: public Widget
{
    private:
        friend struct ChatItem;
        friend struct ChatItemContainer;
        friend struct ChatInputContainer;
        friend struct ChatPreviewItem;
        friend struct SearchPage;

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
        struct FriendMessage
        {
            SDChatPeerID cpid;
            size_t unread = 0;
            std::vector<SDChatMessage> list;
        };

    private:
        ProcessRun *m_processRun;

    private:
        std::optional<int> m_dragIndex;

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
        std::unordered_map<uint64_t, SDChatMessage> m_cachedChatMessageList;

    private:
        std::unordered_map<uint64_t, SDChatMessage> m_localMessageList;
        std::list<FriendMessage> m_friendMessageList;

    public:
        FriendChatBoard(
                Widget::VarOff,
                Widget::VarOff,

                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEventDefault(const SDL_Event &, bool) override;

    public:
        const SDChatPeer *findChatPeer      (const SDChatPeerID &) const;
        const SDChatPeer *findFriendChatPeer(const SDChatPeerID &) const;

    private:
        void queryChatMessage(uint64_t, std::function<void(const SDChatMessage *, bool)>);
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
        void requestAddFriend      (const SDChatPeer &, bool);
        void requestAcceptAddFriend(const SDChatPeer &);
        void requestRejectAddFriend(const SDChatPeer &);
        void requestBlockPlayer    (const SDChatPeer &);

    public:
        void onAddFriendAccepted(const SDChatPeer &);
        void onAddFriendRejected(const SDChatPeer &);

    private:
        std::optional<int> getEdgeDragIndex(int, int) const;
};
