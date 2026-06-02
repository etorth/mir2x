#include "strf.hpp"
#include "mathf.hpp"
#include "xmlf.hpp"
#include "sdldevice.hpp"
#include "playersayboard.hpp"

extern SDLDevice *g_sdlDevice;

PlayerSayBoard::PlayerSayBoard(PlayerSayBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),
          .x = std::move(args.x),
          .y = std::move(args.y),

          .w = [this](const Widget *)
          {
              return empty() ? 0 : pw() + 2 * m_paddingX;
          },

          .h = [this](const Widget *)
          {
              int sumH = 0;
              for(const auto &line: m_lineList){
                  sumH += line.typeset->ph();
              }
              return empty() ? 0 : sumH + 2 * m_paddingY;
          },

          .parent = std::move(args.parent),
      }}

    , m_lineW(args.lineW)
    , m_font(args.font)
    , m_fontSize(args.fontSize)
    , m_fontStyle(args.fontStyle)
    , m_fontColor(std::move(args.fontColor))
    , m_showTime(args.showTime)
    , m_maxEntryCount(args.maxEntryCount)
{}

void PlayerSayBoard::addSay(const std::string &text)
{
    if(text.empty()){
        return;
    }

    while((m_maxEntryCount > 0) && (m_lineList.size() >= m_maxEntryCount)){
        m_lineList.pop_front();
    }

    auto &line = m_lineList.emplace_back();
    line.typeset = std::make_unique<XMLTypeset>(m_lineW, LALIGN_CENTER, false, m_font, m_fontSize, m_fontStyle, m_fontColor);
    line.typeset->loadXML(xmlf::toParString("%s", text.c_str()).c_str());
}

bool PlayerSayBoard::empty() const
{
    return m_lineList.empty();
}

int PlayerSayBoard::pw() const
{
    int maxW = 0;
    for(const auto &line: m_lineList){
        maxW = std::max<int>(maxW, line.typeset->pw());
    }
    return maxW;
}

void PlayerSayBoard::updateDefault(double)
{
    if(m_showTime > 0){
        while(!m_lineList.empty()){
            if(m_lineList.front().timer.diff_msec() >= m_showTime){
                m_lineList.pop_front();
            }
            else{
                break;
            }
        }
    }
}

void PlayerSayBoard::drawDefault(Widget::ROIMap m) const
{
    if(empty()){
        return;
    }

    if(!m.calibrate(this)){
        return;
    }

    {
        const SDLDeviceHelper::EnableRenderCropRectangle enableClip(m.x, m.y, m.ro->w, m.ro->h);
        const SDLDeviceHelper::EnableRenderBlendMode enableBlend(SDL_BLENDMODE_BLEND);
        g_sdlDevice->fillRectangle(colorf::RGBA(0X00, 0X00, 0X00, 0X80), m.x - m.ro->x, m.y - m.ro->y, w(), h(), 3);
        g_sdlDevice->drawRectangle(colorf::RGBA(0XFF, 0XFF, 0XFF, 0X60), m.x - m.ro->x, m.y - m.ro->y, w(), h(), 3);
    }

    int startY = m_paddingY;
    for(const auto &line: m_lineList){
        const auto p = line.typeset.get();

        int dstXCrop = m.x;
        int dstYCrop = m.y;
        int srcXCrop = m.ro->x;
        int srcYCrop = m.ro->y;
        int srcWCrop = m.ro->w;
        int srcHCrop = m.ro->h;

        if(!mathf::cropROI(
                    &srcXCrop, &srcYCrop,
                    &srcWCrop, &srcHCrop,
                    &dstXCrop, &dstYCrop,

                    w(),
                    h(),

                    m_paddingX, startY, p->pw(), p->ph(), 0, 0, -1, -1)){
            break;
        }

        p->draw({.x=dstXCrop, .y=dstYCrop, .ro{srcXCrop - m_paddingX, srcYCrop - startY, srcWCrop, srcHCrop}});
        startY += p->ph();
    }
}
