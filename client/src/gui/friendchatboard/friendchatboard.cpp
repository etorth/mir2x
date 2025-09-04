#include <initializer_list>
#include "sdldevice.hpp"
#include "client.hpp"
#include "hero.hpp"
#include "pngtexdb.hpp"
#include "processrun.hpp"
#include "chatpage.hpp"
#include "pagecontrol.hpp"
#include "chatpreviewpage.hpp"
#include "friendlistpage.hpp"
#include "searchpage.hpp"
#include "friendchatboard.hpp"
#include "friendchatboardconst.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

FriendChatBoard::FriendChatBoard(Widget::VarInt argX, Widget::VarInt argY, ProcessRun *runPtr, Widget *widgetPtr, bool autoDelete)
    : Widget
      {
          DIR_UPLEFT,
          std::move(argX),
          std::move(argY),

          UIPage_BORDER[2] + UIPage_MIN_WIDTH  + UIPage_BORDER[3],
          UIPage_BORDER[0] + UIPage_MIN_HEIGHT + UIPage_BORDER[1],

          {},

          widgetPtr,
          autoDelete,
      }

    , m_processRun(runPtr)
    , m_frame
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const Widget *){ return g_progUseDB->retrieve(0X00000800); },
      }

    , m_frameCropDup
      {
          DIR_UPLEFT,
          0,
          0,
          [this](const Widget *){ return this->w(); },
          [this](const Widget *){ return this->h(); },

          &m_frame,

          55,
          95,
          285 - 55,
          345 - 95,

          this,
          false,
      }

    , m_background
      {
          DIR_UPLEFT,
          0,
          0,
          {},
          {},
          [](const Widget *){ return g_progUseDB->retrieve(0X00000810); },

          false,
          false,
          0,
          colorf::RGBA(160, 160, 160, 255),
      }

    , m_backgroundCropDup
      {
          DIR_UPLEFT,
          0,
          0,
          [this](const Widget *){ return m_frameCropDup.w(); },
          [this](const Widget *){ return m_frameCropDup.h(); },

          &m_background,

          0,
          0,
          510,
          187,

          this,
          false,
      }

    , m_close
      {
          DIR_UPLEFT,
          [this](const Widget *){ return m_frameCropDup.w() - 38; },
          [this](const Widget *){ return m_frameCropDup.h() - 40; },

          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          nullptr,
          [this](Widget *, int)
          {
              setShow(false);
          },

          0,
          0,
          0,
          0,

          true,
          false,
          true,

          this,
      }

    , m_uiPageList
      {
          UIPage // UIPage_CHAT
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  [this](const Widget *){ return 45 + (m_frameCropDup.w() - 45 - 190) / 2; },
                  [    ](const Widget *){ return 29; },

                  u8"好友名称",
                  1,
                  14,
                  0,colorf::WHITE_A255,

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  [this](const Widget *){ return m_frameCropDup.w() - 42; },
                  [    ](const Widget *){ return 29; },
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008F0, 0X000008F0, 0X000008F1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                                  setUIPage(UIPage_CHATPREVIEW);
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000023, 0X00000023, 0X00000024},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008B0, 0X000008B0, 0X000008B1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000590, 0X00000590, 0X00000591},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                              },
                          },

                          true,
                      },

                  },

                  this,
                  true,
              },

              .slider = new TexSlider
              {
                  DIR_UPLEFT,
                  [this](const Widget *){ return m_frameCropDup.w() -  30; },
                  [    ](const Widget *){ return                       70; },
                  [    ](const Widget *){ return                        9; },
                  [this](const Widget *){ return m_frameCropDup.h() - 140; },

                  false,
                  3,
                  nullptr,

                  this,
                  true,
              },

              .page = new ChatPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2],
                  UIPage_BORDER[0],

                  [this](const Widget *){ return m_frameCropDup.w() - UIPage_BORDER[2] - UIPage_BORDER[3]; }, // UIPage_MARGIN included
                  [this](const Widget *){ return m_frameCropDup.h() - UIPage_BORDER[0] - UIPage_BORDER[1]; },

                  this,
                  true,
              },

              .enter = [this](int, UIPage *uiPage)
              {
                  uiPage->title->setText(to_u8cstr([chatPage = dynamic_cast<ChatPage *>(uiPage->page), this]()
                  {
                      if(chatPage->peer.group() || chatPage->peer.special() || findChatPeer({CP_PLAYER, chatPage->peer.id})){
                          return chatPage->peer.name;
                      }
                      else if(chatPage->peer.id == m_processRun->getMyHeroDBID()){
                          return str_printf("自己 %s", chatPage->peer.name.c_str());
                      }
                      else{
                          return str_printf("陌生人 %s", chatPage->peer.name.c_str());
                      }
                  }()));
              },
          },

          UIPage // UIPage_CHATPREVIEW
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  [this](const Widget *){ return 45 + (m_frameCropDup.w() - 45 - 190) / 2; },
                  [    ](const Widget *){ return 29; },

                  u8"【聊天记录】",
                  1,
                  14,
                  0,colorf::WHITE_A255,

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  [this](const Widget *){ return m_frameCropDup.w() - 42; },
                  [    ](const Widget *){ return 29; },
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000160, 0X00000160, 0X00000161},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                                  setUIPage(UIPage_FRIENDLIST);
                              },
                          },

                          true,
                      },
                  },

                  this,
                  true,
              },

              .slider = new TexSlider
              {
                  DIR_UPLEFT,
                  [this](const Widget *){ return m_frameCropDup.w() -  30; },
                  [    ](const Widget *){ return                       70; },
                  [    ](const Widget *){ return                        9; },
                  [this](const Widget *){ return m_frameCropDup.h() - 140; },

                  false,
                  3,
                  nullptr,

                  this,
                  true,
              },

              .page = new ChatPreviewPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2] + UIPage_MARGIN,
                  UIPage_BORDER[0] + UIPage_MARGIN,

                  [this](const Widget *){ return m_frameCropDup.w() - UIPage_BORDER[2] - UIPage_BORDER[3] - 2 * UIPage_MARGIN; },
                  [this](const Widget *){ return m_frameCropDup.h() - UIPage_BORDER[0] - UIPage_BORDER[1] - 2 * UIPage_MARGIN; },

                  this,
                  true,
              },
          },

          UIPage // UIPage_FRIENDLIST
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  [this](const Widget *){ return 45 + (m_frameCropDup.w() - 45 - 190) / 2; },
                  [    ](const Widget *){ return 29; },

                  u8"【好友列表】",
                  1,
                  14,
                  0,colorf::WHITE_A255,

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  [this](const Widget *){ return m_frameCropDup.w() - 42; },
                  [    ](const Widget *){ return 29; },
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008F0, 0X000008F0, 0X000008F1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                                  setUIPage(UIPage_CHATPREVIEW);
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000900, 0X00000900, 0X00000901},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                                  setUIPage(UIPage_FRIENDSEARCH);
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000170, 0X00000170, 0X00000171},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                                  setUIPage(UIPage_CREATEGROUP);
                              },
                          },

                          true,
                      },
                  },

                  this,
                  true,
              },

              .slider = new TexSlider
              {
                  DIR_UPLEFT,
                  [this](const Widget *){ return m_frameCropDup.w() -  30; },
                  [    ](const Widget *){ return                       70; },
                  [    ](const Widget *){ return                        9; },
                  [this](const Widget *){ return m_frameCropDup.h() - 140; },

                  false,
                  3,
                  nullptr,

                  this,
                  true,
              },

              .page = new FriendListPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2] + UIPage_MARGIN,
                  UIPage_BORDER[0] + UIPage_MARGIN,

                  [this](const Widget *){ return m_frameCropDup.w() - UIPage_BORDER[2] - UIPage_BORDER[3] - 2 * UIPage_MARGIN; },
                  [this](const Widget *){ return m_frameCropDup.h() - UIPage_BORDER[0] - UIPage_BORDER[1] - 2 * UIPage_MARGIN; },

                  this,
                  true,
              },
          },

          UIPage // UIPage_FRIENDSEARCH
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  [this](const Widget *){ return 45 + (m_frameCropDup.w() - 45 - 190) / 2; },
                  [    ](const Widget *){ return 29; },

                  u8"【查找用户】",
                  1,
                  14,
                  0,colorf::WHITE_A255,

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  [this](const Widget *){ return m_frameCropDup.w() - 42; },
                  [    ](const Widget *){ return 29; },
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008F0, 0X000008F0, 0X000008F1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                                  setUIPage(UIPage_CHATPREVIEW);
                              },
                          },

                          true,
                      },
                  },

                  this,
                  true,
              },

              .slider = new TexSlider
              {
                  DIR_UPLEFT,
                  [this](const Widget *){ return m_frameCropDup.w() -  30; },
                  [    ](const Widget *){ return                       70; },
                  [    ](const Widget *){ return                        9; },
                  [this](const Widget *){ return m_frameCropDup.h() - 140; },

                  false,
                  3,
                  nullptr,

                  this,
                  true,
              },

              .page = new SearchPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2] + UIPage_MARGIN,
                  UIPage_BORDER[0] + UIPage_MARGIN,

                  this,
                  true,
              },
          },

          UIPage // UIPage_CREATEGROUP
          {
              .title = new LabelBoard
              {
                  DIR_NONE,
                  [this](const Widget *){ return 45 + (m_frameCropDup.w() - 45 - 190) / 2; },
                  [    ](const Widget *){ return 29; },

                  u8"【创建群聊】",
                  1,
                  14,
                  0,colorf::WHITE_A255,

                  this,
                  true,
              },

              .control = new PageControl
              {
                  DIR_RIGHT,
                  [this](const Widget *){ return m_frameCropDup.w() - 42; },
                  [    ](const Widget *){ return 29; },
                  2,

                  {
                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X000008F0, 0X000008F0, 0X000008F1},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                                  setUIPage(UIPage_CHATPREVIEW);
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000910, 0X00000910, 0X00000911},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                                  std::vector<uint32_t> dbidList;
                                  dynamic_cast<FriendListPage *>(m_uiPageList[UIPage_CREATEGROUP].page)->canvas.foreachChild([&dbidList](const Widget *widget, bool)
                                  {
                                      if(const auto friendItem = dynamic_cast<const FriendItem *>(widget)){
                                          if(const auto checkBox = dynamic_cast<const CheckBox *>(friendItem->hasChild(friendItem->funcWidgetID)); checkBox->getter()){
                                              dbidList.push_back(friendItem->cpid.id());
                                          }
                                      }
                                  });

                                  if(dbidList.empty()){
                                      return;
                                  }

                                  if(dbidList.size() > CMCreateChatGroup().list.capacity()){
                                      throw fflerror("selected too many friends, max %zu", CMCreateChatGroup().list.capacity());
                                  }

                                  auto inputBoardPtr = dynamic_cast<InputStringBoard *>(m_processRun->getWidget("InputStringBoard"));

                                  inputBoardPtr->setSecurity(false);
                                  inputBoardPtr->waitInput(u8"<layout><par>请输入你要建立的群名称</par></layout>", [dbidList, this](std::u8string inputString)
                                  {
                                      if(inputString.empty()){
                                          m_processRun->addCBLog(CBLOG_ERR, u8"无效输入:%s", to_cstr(inputString));
                                          return;
                                      }

                                      CMCreateChatGroup cmCCG;
                                      std::memset(&cmCCG, 0, sizeof(cmCCG));

                                      cmCCG.name.assign(inputString);
                                      cmCCG.list.assign(dbidList.begin(), dbidList.end());

                                      g_client->send({CM_CREATECHATGROUP, cmCCG}, [this](uint8_t headCode, const uint8_t *buf, size_t size)
                                      {
                                          switch(headCode){
                                              case SM_OK:
                                                  {
                                                      addGroup(cerealf::deserialize<SDChatPeer>(buf, size));
                                                      break;
                                                  }
                                              default:
                                                  {
                                                      throw fflerror("failed to create group");
                                                  }
                                          }
                                      });
                                  });
                              },
                          },

                          true,
                      },

                      {
                          new TritexButton
                          {
                              DIR_UPLEFT,
                              0,
                              0,

                              {0X00000860, 0X00000860, 0X00000861},
                              {
                                  SYS_U32NIL,
                                  SYS_U32NIL,
                                  0X01020000 + 105,
                              },

                              nullptr,
                              nullptr,
                              nullptr,
                              [this](Widget *, int)
                              {
                                  dynamic_cast<FriendListPage *>(m_uiPageList[UIPage_CREATEGROUP].page)->canvas.foreachChild([](Widget *widget, bool)
                                  {
                                      if(auto friendItem = dynamic_cast<FriendItem *>(widget)){
                                          if(auto checkBox = dynamic_cast<CheckBox *>(friendItem->hasChild(friendItem->funcWidgetID)); checkBox->getter()){
                                              checkBox->toggle();
                                          }
                                      }
                                  });
                              },
                          },

                          true,
                      },
                  },

                  this,
                  true,
              },

              .slider = new TexSlider
              {
                  DIR_UPLEFT,
                  [this](const Widget *){ return m_frameCropDup.w() -  30; },
                  [    ](const Widget *){ return                       70; },
                  [    ](const Widget *){ return                        9; },
                  [this](const Widget *){ return m_frameCropDup.h() - 140; },

                  false,
                  3,
                  nullptr,

                  this,
                  true,
              },

              .page = new FriendListPage
              {
                  DIR_UPLEFT,
                  UIPage_BORDER[2] + UIPage_MARGIN,
                  UIPage_BORDER[0] + UIPage_MARGIN,
                  [this](const Widget *){ return m_frameCropDup.w() - UIPage_BORDER[2] - UIPage_BORDER[3] - 2 * UIPage_MARGIN; },
                  [this](const Widget *){ return m_frameCropDup.h() - UIPage_BORDER[0] - UIPage_BORDER[1] - 2 * UIPage_MARGIN; },

                  this,
                  true,
              },

              .enter = [this](int, UIPage *uiPage)
              {
                  auto listPage = dynamic_cast<FriendListPage *>(uiPage->page);
                  listPage->canvas.clearChild([this](const Widget *widget, bool)
                  {
                      return std::find_if(m_sdFriendList.begin(), m_sdFriendList.end(), [widget](const auto &x)
                      {
                          return dynamic_cast<const FriendItem *>(widget)->cpid.id() == x.id;

                      }) == m_sdFriendList.end();
                  });

                  for(const auto &peer: m_sdFriendList){
                      if(!listPage->canvas.hasChild([&peer](const Widget *widget, bool)
                      {
                          return dynamic_cast<const FriendItem *>(widget)->cpid.id() == peer.id;

                      })){
                          listPage->append(peer, [](FriendItem *item)
                          {
                              if(auto friendItem = dynamic_cast<FriendItem *>(item)){
                                  if(auto checkBox = dynamic_cast<CheckBox *>(friendItem->hasChild(friendItem->funcWidgetID))){
                                      checkBox->toggle();
                                  }
                              }
                          },

                          {
                              new CheckBox
                              {
                                  DIR_UPLEFT,
                                  0,
                                  0,

                                  FriendItem::HEIGHT / 3,
                                  FriendItem::HEIGHT / 3,

                                  colorf::RGB(231, 231, 189) + colorf::A_SHF(128),

                                  nullptr,
                                  nullptr,
                                  nullptr,
                              },
                              true,
                          });
                      }
                  }
              },
          },
      }

    , m_cachedChatPeerList
      {
          SDChatPeer
          {
              .id = SYS_CHATDBID_SYSTEM,
              .name = "系统助手",
          },

          SDChatPeer
          {
              .id = SYS_CHATDBID_GROUP,
              .name = "群管理助手",
          },

          m_processRun->getMyHeroChatPeer(),
      }
{
    setShow(false);
}

void FriendChatBoard::drawEx(int dstX, int dstY, const Widget::ROIOpt &roi) const
{
    const auto roiOpt = cropDrawROI(dstX, dstY, roi);
    if(!roiOpt.has_value()){
        return;
    }

    for(const auto &p:
    {
        static_cast<const Widget *>(&m_backgroundCropDup),
        static_cast<const Widget *>( m_uiPageList[m_uiPage].page),
        static_cast<const Widget *>(&m_frameCropDup),
        static_cast<const Widget *>( m_uiPageList[m_uiPage].title),
        static_cast<const Widget *>( m_uiPageList[m_uiPage].control),
        static_cast<const Widget *>( m_uiPageList[m_uiPage].slider),
        static_cast<const Widget *>(&m_close),
    }){
        int drawDstX = dstX;
        int drawDstY = dstY;
        int drawSrcX = roiOpt->x;
        int drawSrcY = roiOpt->y;
        int drawSrcW = roiOpt->w;
        int drawSrcH = roiOpt->h;

        if(mathf::cropChildROI(
                    &drawSrcX, &drawSrcY,
                    &drawSrcW, &drawSrcH,
                    &drawDstX, &drawDstY,

                    w(),
                    h(),

                    p->dx(),
                    p->dy(),
                    p-> w(),
                    p-> h())){
            p->drawEx(drawDstX, drawDstY, {drawSrcX, drawSrcY, drawSrcW, drawSrcH});
        }
    }
}

bool FriendChatBoard::processEventDefault(const SDL_Event &event, bool valid, int startDstX, int startDstY, const Widget::ROIOpt &roi)
{
    const auto roiOpt = cropDrawROI(startDstX, startDstY, roi);
    if(!roiOpt.has_value()){
        return false;
    }

    if(!valid){
        m_dragIndex.reset();
        return consumeFocus(false);
    }

    if(m_close                        .processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){ return true; }
    if(m_uiPageList[m_uiPage].slider ->processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){ return true; }
    if(m_uiPageList[m_uiPage].page   ->processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){ return true; }
    if(m_uiPageList[m_uiPage].control->processParentEvent(event, valid, startDstX, startDstY, roiOpt.value())){ return true; }

    switch(event.type){
        case SDL_KEYDOWN:
            {
                if(focus()){
                    switch(event.key.keysym.sym){
                        case SDLK_ESCAPE:
                            {
                                setShow(false);
                                setFocus(false);
                                return true;
                            }
                        default:
                            {
                                return false;
                            }
                    }
                }
                return false;
            }
        case SDL_MOUSEBUTTONDOWN:
            {
                if(m_uiPageList[m_uiPage].page->parentIn(event.button.x, event.button.y, startDstX, startDstY, roiOpt.value())){
                    if(m_uiPageList[m_uiPage].page->processParentEvent(event, true, startDstX, startDstY, roiOpt.value())){
                        return consumeFocus(true, m_uiPageList[m_uiPage].page);
                    }
                }

                m_dragIndex = getEdgeDragIndex(event.button.x, event.button.y);
                return consumeFocus(in(event.button.x, event.button.y, startDstX, startDstY, roiOpt.value()));
            }
        case SDL_MOUSEBUTTONUP:
            {
                m_dragIndex.reset();
                return consumeFocus(in(event.button.x, event.button.y, startDstX, startDstY, roiOpt.value()));
            }
        case SDL_MOUSEMOTION:
            {
                if(event.motion.state & SDL_BUTTON_LMASK){
                    if(m_dragIndex.has_value()){
                        bool sizeChanged = false;
                        const auto fnAdjustW = [&sizeChanged, this](int dw, bool adjustOff)
                        {
                            const int oldW = w();
                            const int newW = std::max<int>(oldW + dw, UIPage_BORDER[2] + UIPage_MIN_WIDTH + UIPage_BORDER[3]);

                            if(oldW != newW){
                                sizeChanged = true;
                                setW(newW);
                                if(adjustOff){
                                    moveBy(oldW - newW, 0);
                                }
                            }
                        };

                        const auto fnAdjustH = [&sizeChanged, this](int dh, bool adjustOff)
                        {
                            const int oldH = h();
                            const int newH = std::max<int>(oldH + dh, UIPage_BORDER[0] + UIPage_MIN_HEIGHT + UIPage_BORDER[1]);

                            if(oldH != newH){
                                sizeChanged = true;
                                setH(newH);
                                if(adjustOff){
                                    moveBy(0, oldH - newH);
                                }
                            }
                        };

                        if     (m_dragIndex.value() == 0){ fnAdjustW(-event.motion.xrel, 1); fnAdjustH(-event.motion.yrel, 1); }
                        else if(m_dragIndex.value() == 1){                                   fnAdjustH(-event.motion.yrel, 1); }
                        else if(m_dragIndex.value() == 2){ fnAdjustW( event.motion.xrel, 0); fnAdjustH(-event.motion.yrel, 1); }
                        else if(m_dragIndex.value() == 3){ fnAdjustW(-event.motion.xrel, 1);                                   }
                        else if(m_dragIndex.value() == 4){ fnAdjustW( event.motion.xrel, 0);                                   }
                        else if(m_dragIndex.value() == 5){ fnAdjustW(-event.motion.xrel, 1); fnAdjustH( event.motion.yrel, 0); }
                        else if(m_dragIndex.value() == 6){                                   fnAdjustH( event.motion.yrel, 0); }
                        else                             { fnAdjustW( event.motion.xrel, 0); fnAdjustH( event.motion.yrel, 0); }

                        if(sizeChanged){
                            afterResize();
                        }
                    }
                    else{
                        const auto remapXDiff = startDstX - roiOpt->x;
                        const auto remapYDiff = startDstY - roiOpt->y;

                        const auto [rendererW, rendererH] = g_sdlDevice->getRendererSize();

                        const int maxX = rendererW - w();
                        const int maxY = rendererH - h();

                        const int newX = std::max<int>(0, std::min<int>(maxX, remapXDiff + event.motion.xrel));
                        const int newY = std::max<int>(0, std::min<int>(maxY, remapYDiff + event.motion.yrel));

                        moveBy(newX - remapXDiff, newY - remapYDiff);
                    }
                    return consumeFocus(true);
                }
                return false;
            }
        case SDL_MOUSEWHEEL:
            {
                if(m_uiPageList[m_uiPage].page->focus()){
                    if(m_uiPageList[m_uiPage].page->processEvent(event, true, startDstX, startDstY, roiOpt.value())){
                        return consumeFocus(true, m_uiPageList[m_uiPage].page);
                    }
                }
                return false;
            }
        default:
            {
                return false;
            }
    }
}

void FriendChatBoard::addFriendListChatPeer(const SDChatPeerID &sdCPID)
{
    queryChatPeer(sdCPID, [this](const SDChatPeer *peer, bool)
    {
        if(!peer){
            return;
        }

        dynamic_cast<FriendListPage *>(m_uiPageList[UIPage_FRIENDLIST].page)->append(*peer, [peerInst = *peer, this](FriendItem *item)
        {
            setChatPeer(peerInst, true);
            setUIPage(UIPage_CHAT);
            m_processRun->requestLatestChatMessage({item->cpid.asU64()}, 50, true, true);
        });
    });
}

void FriendChatBoard::setFriendList(const SDFriendList &sdFL)
{
    m_sdFriendList = sdFL;
    std::unordered_set<uint64_t> seenCPIDList;

    const auto fnAddFriend = [&seenCPIDList, this](const SDChatPeerID &sdCPID)
    {
        if(!seenCPIDList.contains(sdCPID.asU64())){
            seenCPIDList.insert(sdCPID.asU64());
            addFriendListChatPeer(sdCPID);
        }
    };

    fnAddFriend(SDChatPeerID(CP_SPECIAL, SYS_CHATDBID_SYSTEM));
    fnAddFriend(m_processRun->getMyHero()->cpid());

    for(const auto &sdCP: sdFL){
        fnAddFriend(sdCP.cpid());
    }
}

const SDChatPeer *FriendChatBoard::findFriendChatPeer(const SDChatPeerID &sdCPID) const
{
    const auto fnOp = [&sdCPID](const SDChatPeer &peer)
    {
        return peer.cpid() == sdCPID;
    };

    if(auto p = std::find_if(m_sdFriendList.begin(), m_sdFriendList.end(), fnOp); p != m_sdFriendList.end()){
        return std::addressof(*p);
    }

    return nullptr;
}

const SDChatPeer *FriendChatBoard::findChatPeer(const SDChatPeerID &sdCPID) const
{
    const auto fnOp = [&sdCPID](const SDChatPeer &peer)
    {
        return peer.cpid() == sdCPID;
    };

    if(auto p = std::find_if(m_sdFriendList.begin(), m_sdFriendList.end(), fnOp); p != m_sdFriendList.end()){
        return std::addressof(*p);
    }

    if(auto p = std::find_if(m_cachedChatPeerList.begin(), m_cachedChatPeerList.end(), fnOp); p != m_cachedChatPeerList.end()){
        return std::addressof(*p);
    }

    return nullptr;
}

void FriendChatBoard::queryChatMessage(uint64_t argMsgID, std::function<void(const SDChatMessage *, bool)> argOp)
{
    if(auto p = m_cachedChatMessageList.find(argMsgID); p != m_cachedChatMessageList.end()){
        if(argOp){
            argOp(std::addressof(p->second), false);
        }
    }

    else{
        CMQueryChatMessage cmQCM;
        std::memset(&cmQCM, 0, sizeof(cmQCM));

        cmQCM.msgid = argMsgID;
        g_client->send({CM_QUERYCHATMESSAGE, cmQCM}, [argMsgID, argOp = std::move(argOp), this](uint8_t headCode, const uint8_t *data, size_t size)
        {
            if(headCode == SM_OK){
                const auto sdCM = cerealf::deserialize<SDChatMessage>(data, size);
                fflassert(sdCM.seq.has_value());
                fflassert(sdCM.seq.value().id == argMsgID);

                auto iter = m_cachedChatMessageList.emplace(argMsgID, sdCM);
                if(argOp){
                    argOp(std::addressof(iter.first->second), true);
                }
            }
            else if(argOp){
                argOp(nullptr, true);
            }
        });
    }
}

void FriendChatBoard::queryChatPeer(const SDChatPeerID &sdCPID, std::function<void(const SDChatPeer *, bool)> argOp)
{
    if(auto p = sdCPID.group() ? nullptr : findChatPeer(sdCPID)){
        if(argOp){
            argOp(p, false);
        }
    }

    else{
        CMQueryChatPeerList cmQCPL;
        std::memset(&cmQCPL, 0, sizeof(cmQCPL));

        cmQCPL.input.assign(std::to_string(sdCPID.id()));
        g_client->send({CM_QUERYCHATPEERLIST, cmQCPL}, [sdCPID, argOp = std::move(argOp), this](uint8_t headCode, const uint8_t *data, size_t size)
        {
            switch(headCode){
                case SM_OK:
                  {
                      if(const auto sdPCL = cerealf::deserialize<SDChatPeerList>(data, size); sdPCL.empty()){
                          if(argOp){
                              argOp(nullptr, true);
                          }
                          return;
                      }
                      else{
                          for(const auto &peer: sdPCL){
                              if(peer.cpid() == sdCPID){
                                  if(argOp){
                                      argOp(&peer, true);
                                  }
                                  return;
                              }
                          }

                          if(argOp){
                              argOp(nullptr, true);
                          }
                          return;
                      }
                  }
              default:
                  {
                      throw fflerror("query failed in server");
                  }
            }
        });
    }
}

void FriendChatBoard::addMessage(std::optional<uint64_t> localPendingID, const SDChatMessage &sdCM)
{
    fflassert(sdCM.seq.has_value());
    const auto peerCPID = [&sdCM, this]
    {
        if(sdCM.from == m_processRun->getMyHero()->cpid()){
            return sdCM.to;
        }
        else if(sdCM.to == m_processRun->getMyHero()->cpid()){
            return sdCM.from;
        }
        else if(sdCM.to.group() && findFriendChatPeer(sdCM.to)){
            return sdCM.to;
        }
        else{
            throw fflerror("received invalid chat message: from %llu, to %llu, self %llu", to_llu(sdCM.from.asU64()), to_llu(sdCM.to.asU64()), to_llu(m_processRun->getMyHero()->cpid().asU64()));
        }
    }();

    auto peerIter = std::find_if(m_friendMessageList.begin(), m_friendMessageList.end(), [peerCPID, &sdCM](const auto &item)
    {
        return item.cpid == peerCPID;
    });

    if(peerIter == m_friendMessageList.end()){
        m_friendMessageList.emplace_front(peerCPID);
    }
    else if(peerIter != m_friendMessageList.begin()){
        m_friendMessageList.splice(m_friendMessageList.begin(), m_friendMessageList, peerIter, std::next(peerIter));
    }

    peerIter = m_friendMessageList.begin();
    if(std::find_if(peerIter->list.begin(), peerIter->list.end(), [&sdCM](const auto &msg){ return msg.seq.value().id == sdCM.seq.value().id; }) == peerIter->list.end()){
        peerIter->unread++;
        peerIter->list.push_back(sdCM);

        auto chatPage = dynamic_cast<ChatPage *>(m_uiPageList[UIPage_CHAT].page);
        auto chatPreviewPage = dynamic_cast<ChatPreviewPage *>(m_uiPageList[UIPage_CHATPREVIEW].page);

        if(peerIter->list.size() >= 2 && peerIter->list.back().seq.value().timestamp < peerIter->list.rbegin()[1].seq.value().timestamp){
            std::sort(peerIter->list.begin(), peerIter->list.end(), [](const auto &x, const auto &y)
            {
                if(x.seq.value().timestamp != y.seq.value().timestamp){
                    return x.seq.value().timestamp < y.seq.value().timestamp;
                }
                else{
                    return x.seq.value().id < y.seq.value().id;
                }
            });

            if(chatPage->peer.cpid() == peerIter->cpid){
                loadChatPage();
            }
        }
        else{
            if(chatPage->peer.cpid() == peerIter->cpid){
                if(localPendingID.has_value()){
                    if(auto p = chatPage->chat.canvas.hasDescendant(localPendingID.value())){
                        dynamic_cast<ChatItem *>(p)->pending = false;
                        dynamic_cast<ChatItem *>(p)->msgID   = sdCM.seq.value().id;
                    }
                }
                else{
                    chatPage->chat.append(peerIter->list.back(), nullptr);
                }
            }
        }

        if(chatPage->peer.cpid() == peerIter->cpid){
            if(chatPage->chat.h() >= chatPage->chat.canvas.h()){
                m_uiPageList[UIPage_CHAT].slider->setShow(false);
            }
            else{
                m_uiPageList[UIPage_CHAT].slider->setShow(true);
                m_uiPageList[UIPage_CHAT].slider->setValue(1.0, false);
            }
        }

        chatPreviewPage->updateChatPreview(peerCPID, cerealf::deserialize<std::string>(peerIter->list.back().message));
    }
}

void FriendChatBoard::addMessagePending(uint64_t localPendingID, const SDChatMessage &sdCM)
{
    fflassert(!sdCM.seq.has_value());
    if(!m_localMessageList.emplace(localPendingID, sdCM).second){
        throw fflerror("adding a pending message with local pending id which has already been used: %llu", to_llu(localPendingID));
    }
}

void FriendChatBoard::finishMessagePending(size_t localPendingID, const SDChatMessageDBSeq &sdCMDBS)
{
    if(auto p = m_localMessageList.find(localPendingID); p != m_localMessageList.end()){
        auto chatMessage = std::move(p->second);
        m_localMessageList.erase(p);

        chatMessage.seq = sdCMDBS;
        addMessage(localPendingID, chatMessage);
    }
    else{
        throw fflerror("invalid local pending message id: %zu", localPendingID);
    }
}

void FriendChatBoard::setChatPeer(const SDChatPeer &sdCP, bool forceReload)
{
    if(auto chatPage = dynamic_cast<ChatPage *>(m_uiPageList[UIPage_CHAT].page); (chatPage->peer.id != sdCP.id) || forceReload){
        chatPage->peer = sdCP;
        loadChatPage();
    }
}

void FriendChatBoard::setUIPage(int uiPage)
{
    fflassert(uiPage >= 0, uiPage);
    fflassert(uiPage < UIPage_END, uiPage);

    const auto fromPage = m_uiPage;
    const auto   toPage =   uiPage;

    if(fromPage != toPage){
        if(m_uiPageList[fromPage].exit){
            m_uiPageList[fromPage].exit(toPage, std::addressof(m_uiPageList[fromPage]));
        }

        if(m_uiPageList[toPage].enter){
            m_uiPageList[toPage].enter(fromPage, std::addressof(m_uiPageList[toPage]));
        }

        m_uiLastPage = fromPage;
        m_uiPage     =   toPage;

        m_uiPageList[fromPage].page->setFocus(false);
        m_uiPageList[  toPage].page->setFocus(true );
    }
}

void FriendChatBoard::loadChatPage()
{
    auto chatPage = dynamic_cast<ChatPage *>(m_uiPageList[UIPage_CHAT].page);
    chatPage->chat.clearChatItem();

    for(const auto &elem: m_friendMessageList){
        if(elem.cpid == chatPage->peer.cpid()){
            for(const auto &msg: elem.list){
                chatPage->chat.append(msg, nullptr);
            }
            break;
        }
    }

    for(const auto &[localID, sdCM]: m_localMessageList){
        if(sdCM.to == chatPage->peer.cpid()){
            chatPage->chat.append(sdCM, nullptr);
        }
    }
}

void FriendChatBoard::addGroup(const SDChatPeer &sdCP)
{
    if(findChatPeer(sdCP.cpid())){
        return;
    }

    m_sdFriendList.push_back(sdCP);
    addFriendListChatPeer(sdCP.cpid());
    dynamic_cast<ChatPreviewPage *>(m_uiPageList[UIPage_CHATPREVIEW].page)->updateChatPreview(sdCP.cpid(), R"###(<layout><par>你已经加入了群聊，现在就可以聊天了。</par></layout>)###");
}

FriendChatBoard *FriendChatBoard::getParentBoard(Widget *widget)
{
    fflassert(widget);
    while(widget){
        if(auto p = dynamic_cast<FriendChatBoard *>(widget)){
            return p;
        }
        else{
            widget = widget->parent();
        }
    }
    throw fflerror("widget is not a decedent of FriendChatBoard");
}

const FriendChatBoard *FriendChatBoard::getParentBoard(const Widget *widget)
{
    fflassert(widget);
    while(widget){
        if(auto p = dynamic_cast<const FriendChatBoard *>(widget)){
            return p;
        }
        else{
            widget = widget->parent();
        }
    }
    throw fflerror("widget is not a decedent of FriendChatBoard");
}

void FriendChatBoard::requestAddFriend(const SDChatPeer &argCP, bool switchToChatPreview)
{
    CMAddFriend cmAF;
    std::memset(&cmAF, 0, sizeof(cmAF));

    cmAF.cpid = argCP.cpid().asU64();
    g_client->send({CM_ADDFRIEND, cmAF}, [argCP, switchToChatPreview, this](uint8_t headCode, const uint8_t *buf, size_t bufSize)
    {
        switch(headCode){
            case SM_OK:
                {
                    switch(const auto sdAFN = cerealf::deserialize<SDAddFriendNotif>(buf, bufSize); sdAFN.notif){
                        case AF_ACCEPTED:
                            {
                                onAddFriendAccepted(argCP);
                                if(switchToChatPreview){
                                    setUIPage(UIPage_CHATPREVIEW);
                                }
                                break;
                            }
                        case AF_REJECTED:
                            {
                                onAddFriendRejected(argCP);
                                break;
                            }
                        case AF_EXIST:
                            {
                                m_processRun->addCBParLog(u8R"###(<par bgcolor="rgb(0x00, 0x80, 0x00)">重复添加好友<t color="red">%s</t></par>)###", to_cstr(argCP.name));
                                break;
                            }
                        case AF_PENDING:
                            {
                                m_processRun->addCBParLog(u8R"###(<par bgcolor="rgb(0x00, 0x80, 0x00)">等待<t color="red">%s</t>处理你的好友验证</par>)###", to_cstr(argCP.name));
                                break;
                            }
                        case AF_BLOCKED:
                            {
                                m_processRun->addCBParLog(u8R"###(<par bgcolor="rgb(0x00, 0x80, 0x00)">你已经被<t color="red">%s</t>加入了黑名单</par>)###", to_cstr(argCP.name));
                                break;
                            }
                        default:
                            {
                                break;
                            }
                    }
                    break;
                }
            default:
                {
                    m_processRun->addCBParLog(u8R"###(<par bgcolor="rgb(0x00, 0x80, 0x00)">无效的请求。</par>)###");
                    break;
                }
        }
    });
}

void FriendChatBoard::requestAcceptAddFriend(const SDChatPeer &argCP)
{
    CMAcceptAddFriend cmAAF;
    std::memset(&cmAAF, 0, sizeof(cmAAF));

    cmAAF.cpid = argCP.cpid().asU64();
    g_client->send({CM_ACCEPTADDFRIEND, cmAAF}, [argCP, this](uint8_t headCode, const uint8_t *, size_t)
    {
        switch(headCode){
            case SM_OK:
                {
                    m_processRun->addCBParLog(u8R"###(<par bgcolor="rgb(0x00, 0x80, 0x00)">你已经通过<t color="red">%s</t>的好友申请</par>)###", to_cstr(argCP.name));
                    break;
                }
            default:
                {
                    m_processRun->addCBParLog(u8R"###(<par bgcolor="red">无效的请求。</par>)###");
                    break;
                }
        }
    });
}

void FriendChatBoard::requestRejectAddFriend(const SDChatPeer &argCP)
{
    CMRejectAddFriend cmRAF;
    std::memset(&cmRAF, 0, sizeof(cmRAF));

    cmRAF.cpid = argCP.cpid().asU64();
    g_client->send({CM_REJECTADDFRIEND, cmRAF}, [argCP, this](uint8_t headCode, const uint8_t *, size_t)
    {
        switch(headCode){
            case SM_OK:
                {
                    m_processRun->addCBParLog(u8R"###(<par bgcolor="rgb(0x00, 0x80, 0x00)">你已经拒绝<t color="red">%s</t>的好友申请。</par>)###", to_cstr(argCP.name));
                    break;
                }
            default:
                {
                    m_processRun->addCBParLog(u8R"###(<par bgcolor="red">无效的请求。</par>)###");
                    break;
                }
        }
    });
}

void FriendChatBoard::requestBlockPlayer(const SDChatPeer &argCP)
{
    CMBlockPlayer cmBP;
    std::memset(&cmBP, 0, sizeof(cmBP));

    cmBP.cpid = argCP.cpid().asU64();
    g_client->send({CM_BLOCKPLAYER, cmBP}, [argCP, this](uint8_t headCode, const uint8_t *, size_t)
    {
        switch(headCode){
            case SM_OK:
                {
                    m_processRun->addCBParLog(u8R"###(<par bgcolor="rgb(0x00, 0x80, 0x00)">你已经拉黑<t color="red">%s</t>。</par>)###", to_cstr(argCP.name));
                    break;
                }
            default:
                {
                    m_processRun->addCBParLog(u8R"###(<par bgcolor="rgb(0x00, 0x80, 0x00)">无效的拉黑请求。</par>)###");
                    break;
                }
        }
    });
}

void FriendChatBoard::onAddFriendAccepted(const SDChatPeer &argCP)
{
    if(!findFriendChatPeer(argCP.cpid())){
        m_sdFriendList.push_back(argCP);
        addFriendListChatPeer(argCP.cpid());

        m_processRun->addCBParLog(u8R"###(<par bgcolor="rgb(0x00, 0x80, 0x00)"><t color="red">%s</t>已经通过了你的好友请求。</par>)###", to_cstr(argCP.name));
        dynamic_cast<ChatPreviewPage *>(m_uiPageList[UIPage_CHATPREVIEW].page)->updateChatPreview(argCP.cpid(), str_printf(R"###(<layout><par><t color="red">%s</t>已经通过你的好友申请，现在可以开始聊天了。</par></layout>)###", to_cstr(argCP.name)));
    }
}

void FriendChatBoard::onAddFriendRejected(const SDChatPeer &argCP)
{
    m_processRun->addCBParLog(u8R"###(<par bgcolor="rgb(0x00, 0x80, 0x00)"><t color="red">%s</t>已经拒绝了你的好友请求。</par>)###", to_cstr(argCP.name));
}

std::optional<int> FriendChatBoard::getEdgeDragIndex(int eventX, int eventY) const
{
    // ->|w0|<----w1------->|w2|<-   |
    //   x0 x1              x2       v
    //   +--+---------------+--+ y0  -
    //   |0)|       1)      |2)|     h0
    //   +--+---------------+--+ y1  -
    //   |  |               |  |     ^
    //   |  |               |  |     |
    //   |3)|               |4)|     h1
    //   |  |               |  |     |
    //   |  |               |  |     v
    //   +--+---------------+--+ y2  -
    //   |5)|       6)      |7)|     h2
    //   +--+---------------+--+     -
    //                               ^
    //                               |

    const int x0 = eventX;
    const int x1 = x0       + UIPage_DRAGBORDER[2];
    const int x2 = x0 + w() - UIPage_DRAGBORDER[3];

    const int y0 = eventY;
    const int y1 = y0       + UIPage_DRAGBORDER[0];
    const int y2 = y0 + h() - UIPage_DRAGBORDER[1];

    const int w0 =                              UIPage_DRAGBORDER[2];
    const int w1 = w() - UIPage_DRAGBORDER[2] - UIPage_DRAGBORDER[3];
    const int w2 =                              UIPage_DRAGBORDER[3];

    const int h0 =                              UIPage_DRAGBORDER[0];
    const int h1 = h() - UIPage_DRAGBORDER[0] - UIPage_DRAGBORDER[1];
    const int h2 =                              UIPage_DRAGBORDER[1];

    if     (mathf::pointInRectangle<int>(eventX, eventY, x0, y0, w0, h0)) return 0;
    else if(mathf::pointInRectangle<int>(eventX, eventY, x1, y0, w1, h0)) return 1;
    else if(mathf::pointInRectangle<int>(eventX, eventY, x2, y0, w2, h0)) return 2;
    else if(mathf::pointInRectangle<int>(eventX, eventY, x0, y1, w0, h1)) return 3;
    else if(mathf::pointInRectangle<int>(eventX, eventY, x2, y1, w2, h1)) return 4;
    else if(mathf::pointInRectangle<int>(eventX, eventY, x0, y2, w0, h2)) return 5;
    else if(mathf::pointInRectangle<int>(eventX, eventY, x1, y2, w1, h2)) return 6;
    else if(mathf::pointInRectangle<int>(eventX, eventY, x2, y2, w2, h2)) return 7;
    else                                                                  return std::nullopt;
}
