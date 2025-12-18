#include "menupage.hpp"
#include "sdldevice.hpp"
#include "tabheader.hpp"

extern SDLDevice *g_sdlDevice;

MenuPage::MenuPage(
        dir8_t argDir,
        int argX,
        int argY,

        Widget::VarSizeOpt argSeperatorW,
        int argGap,

        std::initializer_list<std::tuple<const char8_t *, Widget *, bool>> argTabList,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = argDir,

          .x = argX,
          .y = argY,
          .w = std::nullopt,
          .h = std::nullopt,

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_tabHeaderBg
      {{
          .drawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
          {
              if(m_selectedHeader){
                  g_sdlDevice->fillRectangle(colorf::RGBA(231, 231, 189, 100),
                          drawDstX + m_selectedHeader->dx(),
                          drawDstY + m_selectedHeader->dy(),

                          m_selectedHeader->w(),
                          m_selectedHeader->h());

                  g_sdlDevice->drawLine(colorf::RGBA(231, 231, 189, 100),
                          drawDstX,
                          drawDstY + m_selectedHeader->dy() + m_selectedHeader->h(),

                          drawDstX + self->w(),
                          drawDstY + m_selectedHeader->dy() + m_selectedHeader->h());
              }
          },

          .parent{this},
      }}
{
    TabHeader *lastHeader = nullptr;
    TabHeader *currHeader = nullptr;

    fflassert(argGap >= 0, argGap);
    fflassert(!std::empty(argTabList));

    for(auto &[tabName, tab, autoDelete]: argTabList){
        fflassert(str_haschar(tabName));
        fflassert(tab);

        addChild(tab, autoDelete);
        addChild(currHeader = new TabHeader
        {
            DIR_UPLEFT,
            lastHeader ? (lastHeader->dx() + lastHeader->w() + 10) : 0,
            0,

            tabName,
            [this, tab = tab](Widget *self, int)
            {
                if(m_selectedHeader){
                    std::any_cast<Widget *>(m_selectedHeader->data())->setShow(false);
                }

                tab->setShow(true);
                m_selectedHeader = self->parent();
            },

            tab,

        }, true);

        tab->setShow(lastHeader == nullptr);
        tab->moveAt(DIR_UPLEFT, 0, currHeader->dy() + currHeader->h() + argGap);

        lastHeader = currHeader;

        if(!m_selectedHeader){
            m_selectedHeader = currHeader;
        }
    }

    m_tabHeaderBg.setSize([argSeperatorW = std::move(argSeperatorW), this]
    {
        if(argSeperatorW.has_value()){
            return Widget::evalSize(argSeperatorW.value(), this, nullptr);
        }
        else{
            return maxChildCoverWExcept(&m_tabHeaderBg);
        }
    },

    [this]{ return maxChildCoverHExcept(&m_tabHeaderBg); });
}
