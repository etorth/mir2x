#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "skillpage.hpp"
#include "skillboard.hpp"
#include "magiciconbutton.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

SkillPage::SkillPage(SkillPage::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),

          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = SkillBoard::getPageRectange().w,

          .parent = std::move(args.parent),
      }}

    , m_config    (fflcheck(args.config))
    , m_processRun(fflcheck(args.proc  ))

    , m_bg
      {{
          .texLoadFunc = [texID = args.pageTexID]{ return g_progUseDB->retrieve(texID); },
          .parent{this},
      }}
{
    setH([this]
    {
        const auto low  = std::min<int>(m_bg.h(), SkillBoard::getPageRectange().h);
        const auto high = std::max<int>(m_bg.h(), SkillBoard::getPageRectange().h);

        int maxButtonReachY = 0;
        for(const auto button: m_magicIconButtonList){
            maxButtonReachY = std::max<int>(maxButtonReachY, button->dy() + button->h());
        }

        return std::clamp<int>(maxButtonReachY + 10, low, high); // give 10 pixels of bottom margin
    });
}

void SkillPage::addIcon(uint32_t argMagicID)
{
    for(auto button: m_magicIconButtonList){
        if(button->magicID() == argMagicID){
            return;
        }
    }

    fflassert(DBCOM_MAGICRECORD(argMagicID));
    m_magicIconButtonList.push_back(new MagicIconButton
    {{
        .magicID = argMagicID,

        .config = m_config,
        .proc   = m_processRun,

        .parent
        {
            .widget = this,
            .autoDelete = true,
        },
    }});
}
