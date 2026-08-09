#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "widget.hpp"
#include "imageboard.hpp"
#include "inputline.hpp"
#include "textboard.hpp"
#include "tritexbutton.hpp"
#include "directtradeitemlist.hpp"

class ProcessRun;
class DirectTradeBoard final: public Widget
{
    public:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;

            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            ProcessRun *runProc = nullptr;
            Widget::WADPair parent {};
        };

    private:
        ProcessRun *m_runProc;

    private:
        bool m_dragging = false;

    private:
        bool m_lockPending = false;
        bool m_locked      = false;
        bool m_confirmed   = false;

    private:
        bool m_peerLocked    = false;
        bool m_peerConfirmed = false;

    private:
        size_t      m_peerGold = 0;
        uint64_t    m_peerUID  = 0;
        std::string m_peerNameStr;

    private:
        ImageBoard m_background;

    private:
        TextBoard m_peerName;
        TextBoard m_peerState;
        TextBoard m_peerGoldBoard;
        DirectTradeItemList m_peerItemList;

    private:
        TextBoard m_name;
        TextBoard m_state;
        InputLine m_goldInput;
        DirectTradeItemList m_itemList;

    private:
        TritexButton m_buttonTrade;
        TritexButton m_buttonClose;

    public:
        explicit DirectTradeBoard(DirectTradeBoard::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void    beginTrade(uint64_t, std::string);
        void    closeTrade(bool = true);
        void completeTrade();

    public:
        uint64_t peerUID() const
        {
            return m_peerUID;
        }

    public:
        uint32_t localGold() const;

        std::vector<SDItem> localItemList() const
        {
            return m_itemList.itemList();
        }

        bool localLocked() const
        {
            return m_locked;
        }

        bool peerLocked() const
        {
            return m_peerLocked;
        }

        void setLocalLocked(bool);
        void setPeerOffer(SDDirectTradeOffer);
        void applyLocalOfferAck(const SDDirectTradeOffer &);
        void rejectLocalOffer();

    private:
        std::optional<size_t> parsedLocalGold(bool = true) const;
        bool localOfferMatches(const SDDirectTradeOffer &) const;
        bool syncLocalOffer(bool);

        void onLocalItemClick(DirectTradeItemList::ClickEvent);
        void restoreLocalItems();
        void drawItemHoverText(const SDItem &) const;
};
