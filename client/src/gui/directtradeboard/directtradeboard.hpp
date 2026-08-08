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

// Client-side direct-trade state machine.
//
// The local offer is displayed on the right. Items placed there are temporarily
// removed from InvPack and restored if the trade closes without completion.
// Every edit sends a compact offer request; only a matching server echo can
// finish the local lock transition. Peer items and all peer state always come
// from canonical server offers.
//
// The trade button has two phases:
//   1. submit and lock the local items/gold;
//   2. after both offers are locked, confirm the exchange.
// Both players must confirm before the server performs the atomic exchange.
// SM_INVENTORY/SM_GOLD carry the final state, then SM_COMPLETEDIRECTTRADE closes
// the board without restoring the provisional local offer.
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

        bool m_dragging = false;
        bool m_localLocked = false;
        bool m_localLockPending = false;
        bool m_peerLocked = false;
        bool m_localConfirmed = false;
        bool m_peerConfirmed = false;

        uint32_t m_peerGold = 0;
        uint64_t m_peerUID = 0;
        std::string m_peerName;

    private:
        ImageBoard m_background;
        DirectTradeItemList m_localItemList;
        DirectTradeItemList m_peerItemList;

        TextBoard m_localName;
        TextBoard m_localState;
        TextBoard m_peerState;
        TextBoard m_peerNameBoard;

        InputLine m_localGoldInput;
        TextBoard m_peerGoldBoard;

        TritexButton m_buttonTrade;
        TritexButton m_buttonClose;

    public:
        explicit DirectTradeBoard(DirectTradeBoard::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;
        bool processEventDefault(const SDL_Event &, bool, Widget::ROIMap) override;

    public:
        void begin(uint64_t, std::string);
        void close(bool notify = true);
        void complete();

        uint64_t peerUID() const
        {
            return m_peerUID;
        }

    public:
        uint32_t localGold() const;

        std::vector<SDItem> localItemList() const
        {
            return m_localItemList.itemList();
        }

        bool localLocked() const
        {
            return m_localLocked;
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
        std::optional<uint32_t> parsedLocalGold() const;
        bool localOfferMatches(const SDDirectTradeOffer &) const;
        bool syncLocalOffer(bool);

        void onLocalItemClick(DirectTradeItemList::ClickEvent);
        void restoreLocalItems();
        void drawItemHoverText(const SDItem &) const;
};
