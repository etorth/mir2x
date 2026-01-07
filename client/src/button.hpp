#pragma once
#include <cstddef>
#include <cstdint>
#include <variant>
#include <optional>
#include <functional>

class Widget;
namespace Button
{
    using    OverCBFunc = std::variant<std::nullptr_t, std::function<void(         )>, std::function<void(Widget *           )>>;
    using   ClickCBFunc = std::variant<std::nullptr_t, std::function<void(bool, int)>, std::function<void(Widget *, bool, int)>>;
    using TriggerCBFunc = std::variant<std::nullptr_t, std::function<void(      int)>, std::function<void(Widget *,       int)>>;

    void evalOverCBFunc   (const Button::OverCBFunc    &, Widget *           );
    void evalClickCBFunc  (const Button::ClickCBFunc   &, Widget *, bool, int);
    void evalTriggerCBFunc(const Button::TriggerCBFunc &, Widget *,       int);

    struct SeffIDList
    {
        std::optional<uint32_t> onOverIn  = {};
        std::optional<uint32_t> onOverOut = {};
        std::optional<uint32_t> onClick   = 0X01020000 + 105;
    };
}
