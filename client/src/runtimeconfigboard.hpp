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
        class SwitchIntegerButton: public TritexButton
        {
            private:
                std::pair<int, const int> m_valueState;

            private:
                std::function<void(int, int)> m_onSwitch;

            public:
                SwitchIntegerButton(dir8_t argDir, int argX, int argY, const uint32_t (& texIDList)[3], int initValue, int valueCount, std::function<void(int, int)> onSwitch, Widget *widgetPtr = nullptr, bool autoDelete = false)
                    : TritexButton
                      {
                          argDir,
                          argX,
                          argY,

                          texIDList,
                          {
                              SYS_U32NIL,
                              SYS_U32NIL,
                              0X01020000 + 105,
                          },

                          nullptr,
                          nullptr,
                          [this]()
                          {
                              setValue((getValue() + 1) % getValueCount(), true);
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

                    , m_valueState([initValue, valueCount]() -> std::pair<int, const int>
                      {
                          fflassert(initValue >= 0, initValue);
                          fflassert(valueCount > 0, valueCount);
                          fflassert(initValue < valueCount, initValue, valueCount);
                          return {initValue, valueCount};
                      }())

                    , m_onSwitch(std::move(onSwitch))
                {}

            public:
                int getValue() const
                {
                    return m_valueState.first;
                }

                int getValueCount() const
                {
                    return m_valueState.second;
                }

            public:
                virtual bool setValue(int, bool);
        };

        class SwitchNextButton: public SwitchIntegerButton
        {
            public:
                SwitchNextButton(dir8_t argDir, int argX, int argY, int initValue, int valueCount, std::function<void(int, int)> onSwitch, Widget *widgetPtr = nullptr, bool autoDelete = false)
                    : SwitchIntegerButton
                      {
                          argDir,
                          argX,
                          argY,

                          {
                              0X0000130,
                              0X0000131,
                              0X0000130,
                          },

                          initValue,
                          valueCount,
                          std::move(onSwitch),

                          widgetPtr,
                          autoDelete,
                      }
                {}
        };

        class OnOffButton: public SwitchIntegerButton
        {
            public:
                OnOffButton(dir8_t argDir, int argX, int argY, int initValue, int valueCount, std::function<void(int, int)> onSwitch, Widget *widgetPtr = nullptr, bool autoDelete = false)
                    : SwitchIntegerButton
                      {
                          argDir,
                          argX,
                          argY,

                          {
                              to_u32(initValue ? 0X00000110 : 0X00000120),
                              to_u32(initValue ? 0X00000111 : 0X00000121),
                              to_u32(initValue ? 0X00000111 : 0X00000121), // click triggers tex switch
                          },

                          initValue,
                          valueCount,
                          std::move(onSwitch),

                          widgetPtr,
                          autoDelete,
                      }
                {}

            public:
                bool setValue(int value, bool triggerSwitchCallback) override
                {
                    const auto valChanged = SwitchIntegerButton::setValue(value, triggerSwitchCallback);
                    setTexID(
                    {
                        to_u32(getValue() ? 0X00000110 : 0X00000120),
                        to_u32(getValue() ? 0X00000111 : 0X00000121),
                        to_u32(getValue() ? 0X00000111 : 0X00000121), // click triggers tex switch
                    });
                    return valChanged;
                }
        };

    private:
        TritexButton m_closeButton;

    private:
        OnOffButton m_musicSwitch;
        OnOffButton m_soundEffectSwitch;

    private:
        TexSlider m_musicSlider;
        TexSlider m_soundEffectSlider;

    private:
        const std::vector<std::tuple<std::vector<std::u8string>, int, std::function<void(int, int)>>> m_entryProtoList;

    private:
        std::vector<SwitchIntegerButton *> m_switchList;

    private:
        SDRuntimeConfig m_sdRuntimeConfig;

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

    private:
        void reportRuntimeConfig();

    public:
        const SDRuntimeConfig &getConfig() const
        {
            return m_sdRuntimeConfig;
        }

    public:
        void setConfig(SDRuntimeConfig);
};
