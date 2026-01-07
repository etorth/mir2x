#include <any>
#include <memory>
#include "luaf.hpp"
#include "client.hpp"
#include "imeboard.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "radioselector.hpp"
#include "soundeffectdb.hpp"
#include "processrun.hpp"
#include "inventoryboard.hpp"
#include "fontselector.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

RuntimeConfigBoard::RuntimeConfigBoard(int argX, int argY, int argW, int argH, ProcessRun *proc, Widget *argParent, bool argAutoDelete)
    : Widget
      {{
           .x = argX,
           .y = argY,
           .w = argW,
           .h = argH,

           .parent
           {
               .widget = argParent,
               .autoDelete = argAutoDelete,
           }
       }}

    , m_frameBoard
      {{
          .w = argW,
          .h = argH,
          .parent{this},
      }}

    , m_leftMenuBackground
      {{
          .x = 30,
          .y = 30,
          .w = 80,
          .h = argH - 30 * 2,

          .drawFunc = [](const Widget *widgetPtr, int drawDstX, int drawDstY)
          {
              g_sdlDevice->fillRectangle(                             colorf::A_SHF(128), drawDstX, drawDstY, widgetPtr->w(), widgetPtr->h(), 10);
              g_sdlDevice->drawRectangle(colorf::RGB(231, 231, 189) + colorf::A_SHF(100), drawDstX, drawDstY, widgetPtr->w(), widgetPtr->h(), 10);
          },

          .parent{this},
      }}

    , m_leftMenu
      {{
          .dir = DIR_NONE,

          .x = m_leftMenuBackground.dx() + m_leftMenuBackground.w() / 2,
          .y = m_leftMenuBackground.dy() + m_leftMenuBackground.h() / 2,

          .lineWidth = to_dround(m_leftMenuBackground.w() * 0.7),
          .onClickText = [this](const std::unordered_map<std::string, std::string> &attrList, int event)
          {
              if(event == BEVENT_RELEASE){
                  if(const auto id = LayoutBoard::findAttrValue(attrList, "id", nullptr)){
                      for(const auto &[label, page]: std::initializer_list<std::tuple<const char8_t *, Widget *>>
                      {
                          {u8"系统", &m_pageSystem},
                          {u8"社交", &m_pageSocial},
                          {u8"游戏", &m_pageGameConfig},
                      }){
                          page->setShow(to_u8sv(id) == label);
                      }
                  }
              }
          },

          .parent{this},
      }}

    , m_pageSystem_resolution
      {{
          .label
          {
              .text = u8"分辨率",
              .w = 40,
          },

          .title
          {
              .text = u8"???",
              .w = 80,
              .h = 24,
          },

          .itemList
          {
              {{new LabelBoard{{.label = u8"800×600" , .attrs{.data = std::pair<int, int>( 800, 600)}}}, true}},
              {{new LabelBoard{{.label = u8"960×600" , .attrs{.data = std::pair<int, int>( 960, 600)}}}, true}},
              {{new LabelBoard{{.label = u8"1024×768", .attrs{.data = std::pair<int, int>(1024, 768)}}}, true}},
              {{new LabelBoard{{.label = u8"1280×720", .attrs{.data = std::pair<int, int>(1280, 720)}}}, true}},
              {{new LabelBoard{{.label = u8"1280×768", .attrs{.data = std::pair<int, int>(1280, 768)}}}, true}},
              {{new LabelBoard{{.label = u8"1280×800", .attrs{.data = std::pair<int, int>(1280, 800)}}}, true}},
          },

          .onClick = [this](Widget *widget)
          {
              const auto [w, h] = std::any_cast<std::pair<int, int>>(widget->data());
              g_sdlDevice->setWindowSize(w, h);
              updateWindowSize(w, h, true);
          },
      }}

    , m_pageSystem_musicSlider
      {
          DIR_UPLEFT,
          0,
          0,

          u8"音乐音量",
          60,

          1,
          80,

          [this](float val)
          {
              SDRuntimeConfig_setConfig<RTCFG_BGMVALUE>(m_sdRuntimeConfig, val);
              reportRuntimeConfig(RTCFG_BGMVALUE);
          },
      }

    , m_pageSystem_soundEffectSlider
      {
          DIR_UPLEFT,
          0,
          0,

          u8"声效音量",
          60,

          1,
          80,

          [this](float val)
          {
              SDRuntimeConfig_setConfig<RTCFG_SEFFVALUE>(m_sdRuntimeConfig, val);
              reportRuntimeConfig(RTCFG_SEFFVALUE);
          },
      }

    , m_pageSystem
      {
          DIR_UPLEFT,
          m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30,
          m_leftMenuBackground.dy() + 10,

          w() - (m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30) - 50,
          20,

          {
              {
                  u8"系统",
                  new Widget
                  {{
                      .w = std::nullopt,
                      .h = std::nullopt,

                      .childList
                      {
                          {&m_pageSystem_resolution, DIR_UPLEFT, 0, 0, false},

                          {new CheckLabel{{.label{.text=u8"全屏显示"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_FULLSCREEN>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_FULLSCREEN>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_FULLSCREEN); }}}, DIR_UPLEFT, 0, 40, true},
                          {new CheckLabel{{.label{.text=u8"显示FPS"     }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_SHOWFPS   >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_SHOWFPS   >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_SHOWFPS   ); }}}, DIR_UPLEFT, 0, 65, true},
                          {new CheckLabel{{.label{.text=u8"使用内置拼音"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_PINYIN    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_PINYIN    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_PINYIN    ); }}}, DIR_UPLEFT, 0, 90, true},

                          {new CheckLabel
                          {{
                              .label
                              {
                                  .text = u8"背景音乐",
                              },

                              .getter = [this]
                              {
                                  return SDRuntimeConfig_getConfig<RTCFG_BGM>(m_sdRuntimeConfig);
                              },

                              .setter = [this](bool value)
                              {
                                  SDRuntimeConfig_setConfig<RTCFG_BGM>(m_sdRuntimeConfig, value);
                              },

                              .onChange = [this](bool value)
                              {
                                  reportRuntimeConfig(RTCFG_BGM);
                                  m_pageSystem_musicSlider.setActive(value);
                              },
                          }},
                          DIR_UPLEFT, 0, 125, true},

                          {&m_pageSystem_musicSlider, DIR_UPLEFT, 0, 150, false},

                          {new CheckLabel
                          {{
                              .label
                              {
                                  .text = u8"动作声效",
                              },

                              .getter = [this]
                              {
                                  return SDRuntimeConfig_getConfig<RTCFG_SEFF>(m_sdRuntimeConfig);
                              },

                              .setter = [this](bool value)
                              {
                                  SDRuntimeConfig_setConfig<RTCFG_SEFF>(m_sdRuntimeConfig, value);
                              },

                              .onChange = [this](bool value)
                              {
                                  reportRuntimeConfig(RTCFG_SEFF);
                                  m_pageSystem_soundEffectSlider.setActive(value);
                              },
                          }},
                          DIR_UPLEFT, 0,  185, true},

                          {&m_pageSystem_soundEffectSlider, DIR_UPLEFT, 0, 210, false},
                      },
                  }},
                  true,
              },

              {
                  u8"外观",
                  new Widget
                  {{
                      .w = std::nullopt,
                      .h = std::nullopt,

                      .childList
                      {
                          {
                              .widget = new FontSelector
                              {{

                              }},
                              .autoDelete = true,
                          }
                      },
                  }},
                  true,
              },
          },

          this,
          false,
      }

    , m_pageSocial
      {
          DIR_UPLEFT,
          m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30,
          m_leftMenuBackground.dy() + 10,

          w() - (m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30) - 50,
          20,

          {
              {
                  u8"社交",
                  new Widget
                  {{
                      .w = std::nullopt,
                      .h = std::nullopt,

                      .childList
                      {
                          {new CheckLabel{{.label{.text=u8"允许私聊"        }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许私聊        >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许私聊        >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许私聊        ); }}}, DIR_UPLEFT,   0,   0, true},
                          {new CheckLabel{{.label{.text=u8"允许白字聊天"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许白字聊天    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许白字聊天    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许白字聊天    ); }}}, DIR_UPLEFT,   0,  25, true},
                          {new CheckLabel{{.label{.text=u8"允许地图聊天"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许地图聊天    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许地图聊天    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许地图聊天    ); }}}, DIR_UPLEFT,   0,  50, true},
                          {new CheckLabel{{.label{.text=u8"允许行会聊天"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许行会聊天    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许行会聊天    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许行会聊天    ); }}}, DIR_UPLEFT,   0,  75, true},
                          {new CheckLabel{{.label{.text=u8"允许全服聊天"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许全服聊天    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许全服聊天    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许全服聊天    ); }}}, DIR_UPLEFT,   0, 100, true},
                          {new CheckLabel{{.label{.text=u8"允许加入队伍"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许加入队伍    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许加入队伍    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许加入队伍    ); }}}, DIR_UPLEFT, 200,   0, true},
                          {new CheckLabel{{.label{.text=u8"允许加入行会"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许加入行会    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许加入行会    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许加入行会    ); }}}, DIR_UPLEFT, 200,  25, true},
                          {new CheckLabel{{.label{.text=u8"允许回生术"      }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许回生术      >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许回生术      >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许回生术      ); }}}, DIR_UPLEFT, 200,  50, true},
                          {new CheckLabel{{.label{.text=u8"允许天地合一"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许天地合一    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许天地合一    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许天地合一    ); }}}, DIR_UPLEFT, 200,  75, true},
                          {new CheckLabel{{.label{.text=u8"允许交易"        }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许交易        >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许交易        >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许交易        ); }}}, DIR_UPLEFT, 200, 100, true},
                          {new CheckLabel{{.label{.text=u8"允许添加好友"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许添加好友    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许添加好友    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许添加好友    ); }}}, DIR_UPLEFT, 200, 125, true},
                          {new CheckLabel{{.label{.text=u8"允许行会召唤"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许行会召唤    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许行会召唤    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许行会召唤    ); }}}, DIR_UPLEFT, 200, 150, true},
                          {new CheckLabel{{.label{.text=u8"允许行会杀人提示"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许行会杀人提示>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许行会杀人提示>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许行会杀人提示); }}}, DIR_UPLEFT, 200, 175, true},
                          {new CheckLabel{{.label{.text=u8"允许拜师"        }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许拜师        >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许拜师        >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许拜师        ); }}}, DIR_UPLEFT, 200, 200, true},
                          {new CheckLabel{{.label{.text=u8"允许好友上线提示"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_允许好友上线提示>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_允许好友上线提示>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_允许好友上线提示); }}}, DIR_UPLEFT, 200, 225, true},
                      },
                  }},
                  true,
              },

              {
                  u8"好友",
                  new Widget
                  {{
                      .w = std::nullopt,
                      .h = std::nullopt,

                      .childList
                      {
                          {new LabelBoard
                          {{
                              .label = u8"当加我为好友时：",
                              .font
                              {
                                  .id = 1,
                                  .size = 12,
                              },

                          }}, DIR_UPLEFT, 0, 0, true},

                          {new RadioSelector
                          {
                              DIR_UPLEFT, // ignored
                              0,
                              0,

                              5,
                              5,

                              {
                                  {new LabelBoard{{.label = u8"允许任何人加我微好友", .attrs{.data = to_d(FR_ACCEPT)}}}, true},
                                  {new LabelBoard{{.label = u8"拒绝任何人加我为好友", .attrs{.data = to_d(FR_REJECT)}}}, true},
                                  {new LabelBoard{{.label = u8"好友申请验证"        , .attrs{.data = to_d(FR_VERIFY)}}}, true},
                              },

                              [this](const Widget *radioSelector)
                              {
                                  const Widget *selectedWidget = nullptr;
                                  const auto val = SDRuntimeConfig_getConfig<RTCFG_好友申请>(m_sdRuntimeConfig);

                                  dynamic_cast<const RadioSelector *>(radioSelector)->foreachRadioWidget([&selectedWidget, val](const Widget *widget)
                                  {
                                      if(widget && (std::any_cast<int>(widget->data()) == val)){
                                          selectedWidget = widget;
                                          return true;
                                      }
                                      return false;
                                  });

                                  return selectedWidget;
                              },

                              [this](Widget *, Widget *radioWidget)
                              {
                                  SDRuntimeConfig_setConfig<RTCFG_好友申请>(m_sdRuntimeConfig, std::any_cast<int>(radioWidget->data()));
                                  reportRuntimeConfig(RTCFG_好友申请);
                              },

                          }, DIR_UPLEFT, 0, 20, true},
                      },
                  }},
                  true,
              },
          },

          this,
          false,
      }

    , m_pageGameConfig
      {
          DIR_UPLEFT,
          m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30,
          m_leftMenuBackground.dy() + 10,

          w() - (m_leftMenuBackground.dx() + m_leftMenuBackground.w() + 30) - 50,
          20,

          {
              {
                  u8"常用",
                  new Widget
                  {{
                      .w = std::nullopt,
                      .h = std::nullopt,

                      .childList
                      {
                          {new CheckLabel{{.label{.text=u8"强制攻击"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_强制攻击    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_强制攻击    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_强制攻击    ); }}}, DIR_UPLEFT, 0,   0, true},
                          {new CheckLabel{{.label{.text=u8"显示体力变化"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_显示体力变化>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_显示体力变化>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_显示体力变化); }}}, DIR_UPLEFT, 0,  25, true},
                          {new CheckLabel{{.label{.text=u8"满血不显血"  }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_满血不显血  >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_满血不显血  >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_满血不显血  ); }}}, DIR_UPLEFT, 0,  50, true},
                          {new CheckLabel{{.label{.text=u8"显示血条"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_显示血条    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_显示血条    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_显示血条    ); }}}, DIR_UPLEFT, 0,  75, true},
                          {new CheckLabel{{.label{.text=u8"数字显血"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_数字显血    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_数字显血    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_数字显血    ); }}}, DIR_UPLEFT, 0, 100, true},
                          {new CheckLabel{{.label{.text=u8"综合数字显示"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_综合数字显示>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_综合数字显示>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_综合数字显示); }}}, DIR_UPLEFT, 0, 125, true},
                          {new CheckLabel{{.label{.text=u8"标记攻击目标"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_标记攻击目标>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_标记攻击目标>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_标记攻击目标); }}}, DIR_UPLEFT, 0, 150, true},
                          {new CheckLabel{{.label{.text=u8"单击解除锁定"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_单击解除锁定>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_单击解除锁定>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_单击解除锁定); }}}, DIR_UPLEFT, 0, 175, true},
                          {new CheckLabel{{.label{.text=u8"显示BUFF图标"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_显示BUFF图标>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_显示BUFF图标>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_显示BUFF图标); }}}, DIR_UPLEFT, 0, 200, true},
                          {new CheckLabel{{.label{.text=u8"显示BUFF计时"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_显示BUFF计时>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_显示BUFF计时>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_显示BUFF计时); }}}, DIR_UPLEFT, 0, 225, true},
                          {new CheckLabel{{.label{.text=u8"显示角色名字"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_显示角色名字>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_显示角色名字>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_显示角色名字); }}}, DIR_UPLEFT, 0, 250, true},
                          {new CheckLabel{{.label{.text=u8"关闭组队血条"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_关闭组队血条>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_关闭组队血条>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_关闭组队血条); }}}, DIR_UPLEFT, 0, 275, true},
                          {new CheckLabel{{.label{.text=u8"队友染色"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_队友染色    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_队友染色    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_队友染色    ); }}}, DIR_UPLEFT, 0, 300, true},
                          {new CheckLabel{{.label{.text=u8"显示队友位置"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_显示队友位置>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_显示队友位置>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_显示队友位置); }}}, DIR_UPLEFT, 0, 325, true},
                      },
                  }},
                  true,
              },

              {
                  u8"辅助",
                  new Widget
                  {{
                      .w = std::nullopt,
                      .h = std::nullopt,

                      .childList
                      {
                          {new CheckLabel{{.label{.text=u8"持续盾"      }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_持续盾      >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_持续盾      >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_持续盾      ); }}}, DIR_UPLEFT, 0,   0, true},
                          {new CheckLabel{{.label{.text=u8"持续移花接木"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_持续移花接木>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_持续移花接木>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_持续移花接木); }}}, DIR_UPLEFT, 0,  25, true},
                          {new CheckLabel{{.label{.text=u8"持续金刚"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_持续金刚    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_持续金刚    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_持续金刚    ); }}}, DIR_UPLEFT, 0,  50, true},
                          {new CheckLabel{{.label{.text=u8"持续破血"    }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_持续破血    >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_持续破血    >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_持续破血    ); }}}, DIR_UPLEFT, 0, 100, true},
                          {new CheckLabel{{.label{.text=u8"持续铁布衫"  }, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_持续铁布衫  >(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_持续铁布衫  >(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_持续铁布衫  ); }}}, DIR_UPLEFT, 0, 125, true},
                      },
                  }},
                  true,
              },

              {
                  u8"保护",
                  new Widget
                  {{
                      .w = std::nullopt,
                      .h = std::nullopt,

                      .childList
                      {
                          {new CheckLabel{{.label{.text=u8"自动喝红"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_自动喝红>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_自动喝红>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_自动喝红); }}}, DIR_UPLEFT, 0,  0, true},
                          {new CheckLabel{{.label{.text=u8"保持满血"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_保持满血>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_保持满血>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_保持满血); }}}, DIR_UPLEFT, 0, 25, true},
                          {new CheckLabel{{.label{.text=u8"自动喝蓝"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_自动喝蓝>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_自动喝蓝>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_自动喝蓝); }}}, DIR_UPLEFT, 0, 50, true},
                          {new CheckLabel{{.label{.text=u8"保持满蓝"}, .getter=[this]{ return SDRuntimeConfig_getConfig<RTCFG_保持满蓝>(m_sdRuntimeConfig); }, .setter=[this](bool value){ SDRuntimeConfig_setConfig<RTCFG_保持满蓝>(m_sdRuntimeConfig, value); }, .onChange=[this](bool){ reportRuntimeConfig(RTCFG_保持满蓝); }}}, DIR_UPLEFT, 0, 75, true},

                          {new TextInput{{.labelFirst=u8"等待", .labelSecond=u8"秒", .inputSize{50, 20}}}, DIR_UPLEFT, 0, 110, true},
                      },
                  }},
                  true,
              },
          },

          this,
          false,
      }

    , m_processRun([proc]()
      {
          fflassert(proc); return proc;
      }())
{
    m_leftMenu.loadXML(
        R"###( <layout>                                                 )###""\n"
        R"###(     <par><event id="系统">系 统</event></par><par></par> )###""\n"
        R"###(     <par><event id="社交">社 交</event></par><par></par> )###""\n"
        R"###(     <par><event id="网络">网 络</event></par><par></par> )###""\n"
        R"###(     <par><event id="游戏">游 戏</event></par><par></par> )###""\n"
        R"###(     <par><event id="帮助">帮 助</event></par><par></par> )###""\n"
        R"###( </layout>                                                )###""\n"
    );

    const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();
    updateWindowSize(rendererW, rendererH, false);

    // 1.0f -> SDL_MIX_MAXVOLUME
    // SDL_mixer initial sound/music volume is SDL_MIX_MAXVOLUME

    m_pageSystem_musicSlider      .getSlider()->setValue(0.0, false);
    m_pageSystem_soundEffectSlider.getSlider()->setValue(0.0, false);

    m_pageSystem.setShow(true);
    m_pageSocial.setShow(false);
    m_pageGameConfig.setShow(false);

    setShow(false);
}

void RuntimeConfigBoard::drawDefault(Widget::ROIMap m) const
{
    for(const auto p:
    {
        static_cast<const Widget *>(&m_frameBoard),
        static_cast<const Widget *>(&m_leftMenuBackground),
        static_cast<const Widget *>(&m_leftMenu),
        static_cast<const Widget *>(&m_pageSystem),
        static_cast<const Widget *>(&m_pageSocial),
        static_cast<const Widget *>(&m_pageGameConfig),
    }){
        if(p->show()){
            drawChild(p, m);
        }
    }
}

bool RuntimeConfigBoard::processEventDefault(const SDL_Event &event, bool valid, Widget::ROIMap m)
{
    if(!m.calibrate(this)){
        return false;
    }

    if(!valid){
        return consumeFocus(false);
    }

    if(m_leftMenu      .processEventParent(event, valid, m)){ return true; }
    if(m_frameBoard    .processEventParent(event, valid, m)){ return true; }
    if(m_pageSystem    .processEventParent(event, valid, m)){ return true; }
    if(m_pageSocial    .processEventParent(event, valid, m)){ return true; }
    if(m_pageGameConfig.processEventParent(event, valid, m)){ return true; }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(event.key.keysym.sym == SDLK_ESCAPE){
                    setShow(false);
                    return consumeFocus(false);
                }
                return consumeFocus(true);
            }
        case SDL_MOUSEMOTION:
            {
                if((event.motion.state & SDL_BUTTON_LMASK) && (m.in(event.motion.x, event.motion.y) || focus())){
                    if(const auto par = parent()){
                        moveBy(event.motion.xrel, event.motion.yrel, par->roi());
                    }
                    else{
                        moveBy(event.motion.xrel, event.motion.yrel, Widget::makeROI(0, 0, g_sdlDevice->getRendererSize()));
                    }
                    return consumeFocus(true);
                }
                return false;
            }
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEBUTTONDOWN:
            {
                return consumeFocus(m.in(event.button.x, event.button.y));
            }
        default:
            {
                return false;
            }
    }
}

void RuntimeConfigBoard::setConfig(const SDRuntimeConfig &config)
{
    m_sdRuntimeConfig = config;

    m_pageSystem_musicSlider      .setActive(SDRuntimeConfig_getConfig<RTCFG_BGM >(m_sdRuntimeConfig));
    m_pageSystem_soundEffectSlider.setActive(SDRuntimeConfig_getConfig<RTCFG_SEFF>(m_sdRuntimeConfig));

    m_pageSystem_musicSlider      .getSlider()->setValue(SDRuntimeConfig_getConfig<RTCFG_BGMVALUE >(m_sdRuntimeConfig), false);
    m_pageSystem_soundEffectSlider.getSlider()->setValue(SDRuntimeConfig_getConfig<RTCFG_SEFFVALUE>(m_sdRuntimeConfig), false);

    const auto [winW, winH] = SDRuntimeConfig_getConfig<RTCFG_WINDOWSIZE>(m_sdRuntimeConfig);
    g_sdlDevice->setWindowSize(winW, winH);
}

void RuntimeConfigBoard::reportRuntimeConfig(int rtCfg)
{
    fflassert(rtCfg >= RTCFG_BEGIN, rtCfg);
    fflassert(rtCfg <  RTCFG_END  , rtCfg);

    CMSetRuntimeConfig cmSRC;
    std::memset(&cmSRC, 0, sizeof(cmSRC));

    cmSRC.type = rtCfg;
    cmSRC.buf.assign(m_sdRuntimeConfig.getConfig(rtCfg).value_or(std::string()));

    g_client->send({CM_SETRUNTIMECONFIG, cmSRC});
}

void RuntimeConfigBoard::updateWindowSize(int w, int h, bool saveConfig)
{
    fflassert(w >= 0, w, h);
    fflassert(h >= 0, w, h);

    m_pageSystem_resolution.getTitle()->setText(str_printf(u8"%d×%d", w, h).c_str());
    if(saveConfig){
        SDRuntimeConfig_setConfig<RTCFG_WINDOWSIZE>(m_sdRuntimeConfig, std::make_pair(w, h));
        reportRuntimeConfig(RTCFG_WINDOWSIZE);
    }
}
