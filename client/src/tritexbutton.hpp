#pragma once
#include <cstdint>
#include <functional>
#include "sdldevice.hpp"
#include "buttonbase.hpp"
#include "imageboard.hpp"

class TritexButton: public ButtonBase
{
    private:
        struct TritexIDList final
        {
            std::optional<uint32_t> off  = std::nullopt;
            std::optional<uint32_t> on   = std::nullopt;
            std::optional<uint32_t> down = std::nullopt;

            decltype(auto) operator[](this auto && self, size_t index)
            {
                switch(index){
                    case  0: return self.off;
                    case  1: return self.on;
                    case  2: return self.down;
                    default: throw fflerror("index out of range: %zu", index);
                }
            }
        };

    private:
        using TritexIDFunc = std::variant<std::nullptr_t,
                                          std::function<std::optional<uint32_t>(                   )>,
                                          std::function<std::optional<uint32_t>(                int)>,
                                          std::function<std::optional<uint32_t>(const Widget *, int)>>;

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            TritexButton::TritexIDFunc texIDFunc {};
            TritexButton::TritexIDList texIDList {};

            Button::SeffIDList seff {};

            Button::OverCBFunc onOverIn  = nullptr;
            Button::OverCBFunc onOverOut = nullptr;

            Button::ClickCBFunc onClick = nullptr;
            Button::TriggerCBFunc onTrigger = nullptr;

            int offXOnOver = 0;
            int offYOnOver = 0;

            int offXOnClick = 0;
            int offYOnClick = 0;

            bool onClickDone = true;
            bool radioMode   = false;

            Widget::VarU32Opt modColor = std::nullopt;

            Widget::InstAttrs attrs {};
            Widget::WADPair  parent {};
        };

    private:
        TritexButton::TritexIDFunc m_texIDFunc;
        TritexButton::TritexIDList m_texIDList;

    private:
        const Widget::VarU32 m_modColor;

    private:
        double m_accuBlinkTime = 0.0;
        std::optional<std::tuple<unsigned, unsigned, unsigned>> m_blinkTime = {}; // {off, on} in ms

    private:
        ImageBoard m_img;

    public:
        TritexButton(TritexButton::InitArgs);

    public:
        void drawDefault(Widget::ROIMap) const override;

    public:
        void setTexIDList(const TritexButton::TritexIDList &texIDList)
        {
            m_texIDFunc = nullptr;
            m_texIDList = texIDList;
        }

        void setTexIDFunc(TritexButton::TritexIDFunc texIDFunc)
        {
            m_texIDFunc = std::move(texIDFunc);
        }

        void setBlinkTime(unsigned offTime, unsigned onTime, unsigned activeTotalTime = 0)
        {
            m_blinkTime = std::make_tuple(offTime, onTime, activeTotalTime);
            m_accuBlinkTime = 0.0;
        }

        void stopBlink()
        {
            m_blinkTime.reset();
            m_accuBlinkTime = 0.0;
        }

    public:
        void updateDefault(double fUpdateTime) override
        {
            if(m_blinkTime.has_value()){
                m_accuBlinkTime += fUpdateTime;
                if(const auto activeTotalTime = std::get<2>(m_blinkTime.value()); activeTotalTime > 0 && m_accuBlinkTime > activeTotalTime){
                    m_blinkTime.reset();
                }
            }
        }

    private:
        SDL_Texture *evalGfxTexture     (std::optional<int> = std::nullopt) const;
        SDL_Texture *evalGfxTextureValid(                                 ) const; // search the first valid Texture

    public:
        int w() const override { return SDLDeviceHelper::getTextureWidth (evalGfxTextureValid(), 0); }
        int h() const override { return SDLDeviceHelper::getTextureHeight(evalGfxTextureValid(), 0); }
};
