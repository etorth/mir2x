#include "colorf.hpp"
#include "trigfxbutton.hpp"

TrigfxButton::TrigfxButton(TrigfxButton::InitArgs args)
    : ButtonBase
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = 0, // use override w() and h()
          .h = 0, // ...

          .onOverIn  = std::move(args.onOverIn),
          .onOverOut = std::move(args.onOverOut),

          .onClick = std::move(args.onClick),
          .onTrigger = std::move(args.onTrigger),

          .seff = args.seff,

          .offXOnOver = args.offXOnOver,
          .offYOnOver = args.offYOnOver,

          .offXOnClick = args.offXOnClick,
          .offYOnClick = args.offYOnClick,

          .onClickDone = args.onClickDone,
          .radioMode   = args.radioMode,

          .attrs  = std::move(args.attrs),
          .parent = std::move(args.parent),
      }}

    , m_gfxFunc(std::move(args.gfxFunc))
    , m_gfxList(args.gfxList)
{}

void TrigfxButton::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(auto gfxPtr = evalGfxWidget()){
        drawAsChild(gfxPtr, DIR_UPLEFT, m_offset[getState()][0], m_offset[getState()][1], m);
    }
}

const Widget *TrigfxButton::evalGfxWidget(std::optional<int> stateOpt) const
{
    const auto state = stateOpt.value_or(getState());
    return std::visit(VarDispatcher
    {
        [state, this](const std::function<const Widget *(                   )> &f){ return f ? f(           ) : m_gfxList.at(state); },
        [state, this](const std::function<const Widget *(                int)> &f){ return f ? f(      state) : m_gfxList.at(state); },
        [state, this](const std::function<const Widget *(const Widget *, int)> &f){ return f ? f(this, state) : m_gfxList.at(state); },
        [state, this](const                                               auto & ){ return                      m_gfxList.at(state); },
    },
    m_gfxFunc);
}

const Widget *TrigfxButton::evalGfxWidgetValid() const
{
    for(const auto s = getState() - BEVENT_BEGIN; int i: {0, 1, 2}){
        if(const auto gfxPtr = evalGfxWidget(BEVENT_BEGIN + ((s + i) % 3))){
            return gfxPtr;
        }
    }
    return nullptr;
}
