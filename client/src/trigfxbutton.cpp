#include "colorf.hpp"
#include "trigfxbutton.hpp"

TrigfxButton::TrigfxButton(Widget::VarDir argDir,
        Widget::VarOff argX,
        Widget::VarOff argY,

        std::array<const Widget *, 3> argGfxList,
        std::array<std::optional<uint32_t>, 3> argSeffIDList,

        std::function<void(Widget *      )> argOnOverIn,
        std::function<void(Widget *      )> argOnOverOut,
        std::function<void(Widget *, bool, int)> argOnClick,
        std::function<void(Widget *,      int)> argOnTrigger,

        int argOffXOnOver,
        int argOffYOnOver,
        int argOffXOnClick,
        int argOffYOnClick,

        bool argOnClickDone,
        bool argRadioMode,

        Widget *argParent,
        bool    argAutoDelete)

    : ButtonBase
      {
          std::move(argDir),
          std::move(argX),
          std::move(argY),
          0,
          0,

          std::move(argOnOverIn),
          std::move(argOnOverOut),
          std::move(argOnClick),
          std::move(argOnTrigger),

          argSeffIDList[0],
          argSeffIDList[1],
          argSeffIDList[2],

          argOffXOnOver,
          argOffYOnOver,
          argOffXOnClick,
          argOffYOnClick,

          argOnClickDone,
          argRadioMode,

          argParent,
          argAutoDelete,
      }

    , m_gfxList
      {
          argGfxList[0],
          argGfxList[1],
          argGfxList[2],
      }
{
    initButtonSize();
}

void TrigfxButton::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    const auto roiOpt = cropDrawROI(dstX, dstY, roi);
    if(!roiOpt.has_value()){
        return;
    }

    if(auto gfxPtr = m_gfxList[getState()]){
        const int offX = m_offset[getState()][0];
        const int offY = m_offset[getState()][1];
        // gfxPtr->drawEx(dstX + offX, dstY + offY, srcX, srcY, srcW, srcH);
        gfxPtr->drawEx(dstX + offX, dstY + offY, roiOpt.value());
    }
}

void TrigfxButton::initButtonSize()
{
    int maxW = 0;
    int maxH = 0;

    for(const auto &p: m_gfxList){
        maxW = std::max<int>(p->w(), maxW);
        maxH = std::max<int>(p->h(), maxH);
    }

    setW(maxW);
    setH(maxH);
}
