#pragma once
#include <tuple>
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
                std::tuple<int &, const int, const int> m_valueState;

            private:
                std::function<void(int)> m_onSwitch;

            public:
                SwitchIntegerButton(dir8_t argDir, int argX, int argY, const uint32_t (& texIDList)[3], int &valueRef, int valueOffset, int valueCount, std::function<void(int)> onSwitch, Widget *widgetPtr = nullptr, bool autoDelete = false)
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
                              std::get<0>(m_valueState) = ((getValue() - getValueOffset() + 1) % getValueCount()) + getValueOffset();
                              triggerCallback();

                              for(auto parentPtr = parent(); parentPtr; parentPtr = parentPtr->parent()){
                                  if(auto configBoardPtr = dynamic_cast<RuntimeConfigBoard *>(parentPtr)){
                                      configBoardPtr->reportRuntimeConfig();
                                      break;
                                  }
                              }
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

                    , m_valueState([&valueRef, valueOffset, valueCount]()
                      {
                          fflassert(valueOffset >= 0, valueOffset);
                          fflassert(valueCount > 0, valueCount);

                          fflassert(valueRef >= valueOffset, valueRef, valueOffset);
                          fflassert(valueRef < valueCount, valueRef, valueCount);

                          return std::tuple<int &, const int, const int>(valueRef, valueOffset, valueCount);
                      }())

                    , m_onSwitch(std::move(onSwitch))
                {}

            public:
                int getValue() const
                {
                    return std::get<0>(m_valueState);
                }

                int getValueOffset() const
                {
                    return std::get<1>(m_valueState);
                }

                int getValueCount() const
                {
                    return std::get<2>(m_valueState);
                }

            public:
                void triggerCallback()
                {
                    if(m_onSwitch){
                        m_onSwitch(getValue());
                    }
                }
        };

        class SwitchNextButton: public SwitchIntegerButton
        {
            public:
                SwitchNextButton(dir8_t argDir, int argX, int argY, int &valueRef, int valueOffset, int valueCount, std::function<void(int)> onSwitch, Widget *widgetPtr = nullptr, bool autoDelete = false)
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

                          valueRef,
                          valueOffset,
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
                OnOffButton(dir8_t argDir, int argX, int argY, int &valueRef, int valueOffset, int valueCount, std::function<void(int)> onSwitch, Widget *widgetPtr = nullptr, bool autoDelete = false)
                    : SwitchIntegerButton
                      {
                          argDir,
                          argX,
                          argY,

                          {
                              to_u32(valueRef ? 0X00000110 : 0X00000120),
                              to_u32(valueRef ? 0X00000111 : 0X00000121),
                              to_u32(valueRef ? 0X00000111 : 0X00000121), // click triggers tex switch
                          },

                          valueRef,
                          valueOffset,
                          valueCount,

                          [onSwitch = std::move(onSwitch), this](int state)
                          {
                              setTexID(
                              {
                                  to_u32(getValue() ? 0X00000110 : 0X00000120),
                                  to_u32(getValue() ? 0X00000111 : 0X00000121),
                                  to_u32(getValue() ? 0X00000111 : 0X00000121), // click triggers tex switch
                              });

                              if(onSwitch){
                                  onSwitch(state);
                              }
                          },

                          widgetPtr,
                          autoDelete,
                      }
                {}
        };

    private:
        TritexButton m_closeButton;

    private:
        SDRuntimeConfig m_sdRuntimeConfig;

    private:
        OnOffButton m_musicSwitch;
        OnOffButton m_soundEffectSwitch;

    private:
        TexSlider m_musicSlider;
        TexSlider m_soundEffectSlider;

    private:
        const std::vector<std::tuple<std::vector<std::u8string>, std::reference_wrapper<int>, int, std::function<void(int)>>> m_entryProtoList;

    private:
        std::vector<SwitchIntegerButton *> m_switchList;

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
