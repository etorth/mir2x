#include "magiciconbutton.hpp"
#include "skillboard.hpp"
#include "processrun.hpp"
#include "textboard.hpp"

MagicIconButton::MagicIconButton(MagicIconButton::InitArgs args)
    : Widget
      {{
          .parent = std::move(args.parent),
      }}

    , m_magicID(fflcheck(args.magicID, true
                && DBCOM_MAGICRECORD(args.magicID)
                && SkillBoard::getMagicIconGfx(args.magicID)))

    , m_config    (fflcheck(args.config))
    , m_processRun(fflcheck(args.proc  ))

    , m_icon
      {{
          .texIDList
          {
              .off  = SkillBoard::getMagicIconGfx(m_magicID)->magicIcon,
              .on   = SkillBoard::getMagicIconGfx(m_magicID)->magicIcon,
              .down = SkillBoard::getMagicIconGfx(m_magicID)->magicIcon,
          },

          .onClickDone = false,
          .parent{this},
      }}
{
    moveTo(SkillBoard::getMagicIconGfx(m_magicID)->x * 60 + 12,
           SkillBoard::getMagicIconGfx(m_magicID)->y * 65 + 13);

    // leave some pixels to draw level label
    // since level can change during run, can't get the exact size here

    setSize(m_icon.w() + 8,
            m_icon.h() + 8);
}

void MagicIconButton::drawDefault(Widget::ROIMap m) const
{
    if(!m.calibrate(this)){
        return;
    }

    if(const auto levelOpt = m_config->getMagicLevel(magicID()); levelOpt.has_value()){
        Widget::drawDefault(m);

        const TextBoard magicLevel
        {{
            .textFunc = std::to_string(levelOpt.value()),
            .font
            {
                .id = 3,
                .size = 12,
                .color = colorf::YELLOW_A255,
            },
        }};
        drawAsChild(&magicLevel, DIR_UPLEFT, m_icon.w() - 2, m_icon.h() - 1, m);

        if(const auto keyOpt = m_config->getMagicKey(magicID()); keyOpt.has_value()){
            const TextShadowBoard magicKey
            {{
                .shadowX = 2,
                .shadowY = 2,

                .textFunc = [keyOpt]{ return str_printf("%c", std::toupper(keyOpt.value())); },
                .font
                {
                    .id = 3,
                    .size = 20,
                    .color = colorf::RGBA(0XFF, 0X80, 0X00, 0XE0),
                },
                .shadowColor = colorf::RGBA(0X00, 0X00, 0X00, 0XE0),
            }};
            drawAsChild(&magicKey, DIR_UPLEFT, 2, 2, m);
        }
    }
}

bool MagicIconButton::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    const auto result = m_icon.processEventParent(event, valid, m);
    if(event.type == SDL_KEYDOWN && cursorOn()){
        if(const auto key = SDLDeviceHelper::getKeyChar(event, false); (key >= '0' && key <= '9') || (key >= 'a' && key <= 'z')){
            if(m_config->hasMagicID(magicID())){
                if(SkillBoard::getMagicIconGfx(magicID())->passive){
                    m_processRun->addCBLog(CBLOG_SYS, u8"无法为被动技能设置快捷键：%s", to_cstr(DBCOM_MAGICRECORD(magicID()).name));
                }
                else{
                    m_config->setMagicKey(magicID(), key);
                    m_processRun->requestSetMagicKey(magicID(), key);
                }
            }
            return consumeFocus(true);
        }
    }
    return result;
}
