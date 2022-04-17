#pragma once
#include <string>
#include "mathf.hpp"
#include "widget.hpp"
#include "labelboard.hpp"
#include "texslider.hpp"
#include "tritexbutton.hpp"

class ProcessRun;
class RuntimeConfigBoard: public Widget
{
    private:
        class OnOffTexButton: public TritexButton
        {
            private:
                bool m_value = false;

            private:
                const std::function<void(bool)> m_onSwitch;

            public:
                OnOffTexButton(int argX, int argY, bool initValue, std::function<void(bool)> onSwitch, Widget *widgetPtr = nullptr, bool autoDelete = false)
                    : TritexButton
                      {
                          DIR_UPLEFT,
                          argX,
                          argY,

                          {
                              to_u32(initValue ? 0X00000110 : 0X00000120),
                              to_u32(initValue ? 0X00000111 : 0X00000121),
                              to_u32(initValue ? 0X00000111 : 0X00000121), // click triggers tex switch
                          },

                          {
                              SYS_U32NIL,
                              SYS_U32NIL,
                              0X01020000 + 105,
                          },

                          nullptr,
                          nullptr,
                          [this]()
                          {
                              setValue(!getValue(), true);
                          },

                          0,
                          0,
                          0,
                          0,

                          false,
                          false,
                          widgetPtr,
                          autoDelete,
                      }

                    , m_value(initValue)
                    , m_onSwitch(std::move(onSwitch))
                {}

            public:
                bool getValue() const
                {
                    return m_value;
                }

                void setValue(bool value, bool triggerSwitchCallback)
                {
                    if(value == getValue()){
                        return;
                    }

                    m_value = value;
                    setTexID(
                    {
                        to_u32(getValue() ? 0X00000110 : 0X00000120),
                        to_u32(getValue() ? 0X00000111 : 0X00000121),
                        to_u32(getValue() ? 0X00000111 : 0X00000121), // click triggers tex switch
                    });

                    if(triggerSwitchCallback && m_onSwitch){
                        m_onSwitch(getValue());
                    }
                }
        };

    private:
        TritexButton m_closeButton;

    private:
        OnOffTexButton m_musicSwitch;
        OnOffTexButton m_soundEffectSwitch;

    private:
        TexSlider m_musicSlider;
        TexSlider m_soundEffectSlider;

    private:
        std::vector<std::tuple<const char8_t *, OnOffTexButton *>> m_switchList;

    private:
        ProcessRun *m_processRun;

    public:
        RuntimeConfigBoard(int, int, ProcessRun *, Widget * = nullptr, bool = false);

    public:
        void drawEx(int, int, int, int, int, int) const override;

    public:
        bool processEvent(const SDL_Event &, bool) override;

    protected:
        void drawEntryTitle(const char8_t *, int, int) const;

    protected:
        static std::tuple<int, int, int, int> getEntryPLoc(size_t);

    public:
        std::optional<float> getMusicVolume() const
        {
            if(m_musicSwitch.getValue()){
                return mathf::bound<float>(m_musicSlider.getValue(), 0.0f, 1.0f);
            }
            return {};
        }

        std::optional<float> getSoundEffectVolume() const
        {
            if(m_soundEffectSwitch.getValue()){
                return mathf::bound<float>(m_soundEffectSlider.getValue(), 0.0f, 1.0f);
            }
            return {};
        }
};
