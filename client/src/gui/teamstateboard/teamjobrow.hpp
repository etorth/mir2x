#pragma once
#include <optional>
#include <functional>
#include "widget.hpp"
#include "textbutton.hpp"
#include "checklabel.hpp"
#include "serdesmsg.hpp"

class TeamJobRow: public Widget // [x] 允许战士加入 [设置]
{
    public:
        struct InitArgs final
        {
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            int w = 0;
            int h = 0;

            size_t jobIndex = 0;
            std::optional<SDTeamJobReq> *requirement = nullptr;

            std::function<void()> onSetup = nullptr;

            Widget::VarBool editable = true;
            Widget::WADPair parent {};
        };

    private:
        std::optional<SDTeamJobReq> * const m_requirement;

    private:
        CheckLabel m_allow;
        TextButton m_setup;

    public:
        explicit TeamJobRow(TeamJobRow::InitArgs);
};
