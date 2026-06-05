#include "xmlf.hpp"
#include "sdldevice.hpp"
#include "messagestackboard.hpp"

extern SDLDevice *g_sdlDevice;

MessageStackBoard::MessageStackBoard(MessageStackBoard::InitArgs args)
    : Widget
      {{
          .dir = std::move(args.dir),
          .x = std::move(args.x),
          .y = std::move(args.y),
          .w = std::nullopt,
          .h = std::nullopt,
          .parent = std::move(args.parent),
      }}

    , m_itemFlex
      {{
          .v = true,
          .align = args.align,
          .itemSpace = std::move(args.itemSpace),
          .parent{this},
      }}

    , m_width(args.width)
    , m_corner(args.corner)
    , m_font(std::move(args.font))
    , m_showTime(args.showTime)
    , m_entryLimit(args.entryLimit)
    , m_margin(std::move(args.margin))
    , m_bgColor(std::move(args.bgColor))
    , m_borderColor(std::move(args.borderColor))
{}

void MessageStackBoard::addMessage(const std::u8string &text)
{
    const auto xml = xmlf::toParString("%s", text.empty() ? "" : reinterpret_cast<const char *>(text.c_str()));
    addXMLMessage(to_u8str(xml));
}

void MessageStackBoard::addXMLMessage(const std::u8string &xml)
{
    while((m_entryLimit > 0) && (m_messageList.size() >= m_entryLimit)){
        removeFrontMessage();
    }

    auto message = std::make_unique<Message>();
    message->typeset = std::make_unique<XMLTypeset>(m_width, LALIGN_LEFT, false, m_font.id, m_font.size, m_font.style, m_font.color);
    message->typeset->loadXML(reinterpret_cast<const char *>(xml.c_str()));

    auto messagePtr = message.get();
    auto contentWidget = new Widget
    {{
        .w = [messagePtr](const Widget *)
        {
            return messagePtr->typeset->pw();
        },

        .h = [messagePtr](const Widget *)
        {
            return messagePtr->typeset->ph();
        },

        .attrs
        {
            .inst
            {
                .moveOnFocus = false,
                .draw = [messagePtr](const Widget *self, Widget::ROIMap m)
                {
                    if(!m.calibrate(self)){
                        return;
                    }
                    messagePtr->typeset->draw(m);
                },
            },
        },
    }};

    message->wrapper.reset(new MarginWrapper
    {{
        .wrapped
        {
            .widget = contentWidget,
            .autoDelete = true,
        },
        .margin = m_margin,
        .bgDrawFunc = [this](const Widget *self, int drawDstX, int drawDstY)
        {
            const SDLDeviceHelper::EnableRenderBlendMode enableBlend(SDL_BLENDMODE_BLEND);
            g_sdlDevice->fillRectangle(Widget::evalU32(m_bgColor, this), drawDstX, drawDstY, self->w(), self->h(), m_corner);

            if(colorf::A(Widget::evalU32(m_borderColor, this))){
                g_sdlDevice->drawRectangle(Widget::evalU32(m_borderColor, this), drawDstX, drawDstY, self->w(), self->h(), m_corner);
            }
        },
    }});

    m_itemFlex.addItem(message->wrapper.get(), false);
    m_messageList.push_back(std::move(message));
}

void MessageStackBoard::clear()
{
    while(!m_messageList.empty()){
        removeFrontMessage();
    }
}

bool MessageStackBoard::empty() const
{
    return m_messageList.empty();
}

void MessageStackBoard::setFont(uint8_t font)
{
    m_font.id = font;
}

void MessageStackBoard::setFontSize(uint8_t fontSize)
{
    m_font.size = fontSize;
}

void MessageStackBoard::setFontStyle(uint8_t fontStyle)
{
    m_font.style = fontStyle;
}

void MessageStackBoard::setFontColor(Widget::VarU32 fontColor)
{
    m_font.color = std::move(fontColor);
}

void MessageStackBoard::updateDefault(double ms)
{
    if(m_showTime > 0){
        while(!m_messageList.empty()){
            if(m_messageList.front()->timer.diff_msec() >= m_showTime){
                removeFrontMessage();
            }
            else{
                break;
            }
        }
    }

    Widget::updateDefault(ms);
}

void MessageStackBoard::removeFrontMessage()
{
    fflassert(!m_messageList.empty());
    m_itemFlex.removeItem(m_messageList.front()->wrapper->id(), false);
    m_itemFlex.purge();
    m_messageList.pop_front();
}
