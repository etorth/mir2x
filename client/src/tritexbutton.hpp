#pragma once
#include <cstdint>
#include <utility>
#include <variant>
#include <optional>
#include <functional>
#include "sdldevice.hpp"
#include "buttonbase.hpp"
#include "imageboard.hpp"

class TritexButton: public ButtonBase
{
    public:
        class AniFrameOpt final
        {
            private:
                std::optional<std::pair<uint32_t, int>> m_frame;

            public:
                // helper constructors
                // then you can write code like this:
                //
                //     .texIDList
                //     {
                //         .texAniOpt {},       // no frame
                //         .texAniOpt {12},     // animation 12, frame 0
                //         .texAniOpt {12, 2},  // animation 12, frame 2
                //     }

                AniFrameOpt() = default;
                AniFrameOpt(uint32_t aniIndex                ): m_frame(std::make_pair(aniIndex, 0         )) {}
                AniFrameOpt(uint32_t aniIndex, int frameIndex): m_frame(std::make_pair(aniIndex, frameIndex)) {}

            public:
                operator bool () const noexcept
                {
                    return m_frame.has_value();
                }

            public:
                uint32_t animation() const { return m_frame.value().first ; } // throw if opt is empty
                int      frame    () const { return m_frame.value().second; } // throw if opt is empty
        };

    private:
        using TritexIDFunc = std::variant<std::nullptr_t,
                                          std::function<TritexButton::AniFrameOpt(                   )>,
                                          std::function<TritexButton::AniFrameOpt(                int)>,
                                          std::function<TritexButton::AniFrameOpt(const Widget *, int)>>;
    private:
        struct TritexIDList final
        {
            TritexButton::AniFrameOpt off  {};
            TritexButton::AniFrameOpt on   {};
            TritexButton::AniFrameOpt down {};

            decltype(auto) operator[](this auto && self, size_t index)
            {
                switch(index){
                    case  0: return self.off;
                    case  1: return self.on;
                    case  2: return self.down;
                    default: throw fflpanic("index out of range: {}", index);
                }
            }
        };

    private:
        struct InitArgs final
        {
            Widget::VarDir dir = DIR_UPLEFT;
            Widget::VarInt x = 0;
            Widget::VarInt y = 0;

            std::optional<uint32_t>    texAniOpt {}; // contains an animatinon
            TritexButton::TritexIDFunc texIDFunc {}; //  returns a  frame      which can be of different animations
            TritexButton::TritexIDList texIDList {}; // contains a  frame list which can be of different animations

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
        std::optional<uint32_t>    m_texAniOpt;
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
            m_texAniOpt.reset();
            m_texIDFunc = nullptr;
            m_texIDList = texIDList;
        }

        void setTexIDFunc(TritexButton::TritexIDFunc texIDFunc)
        {
            m_texAniOpt.reset();
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
