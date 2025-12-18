#include <string_view>
#include "uidf.hpp"
#include "totype.hpp"
#include "sysconst.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "npcchatboard.hpp"
#include "clientargparser.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

// with face
// all margins are same
//
// +-----------------------------+
// | +----+ +------------------+ |
// | |    | |                  | |
// | |    | |                  | |
// | |    | |                  | |
// | +----+ +------------------+ |
// +-----------------------------+
//
// without face
// +-----------------------------+
// | +-------------------------+ |
// | |                         | |
// | |                         | |
// | |                         | |
// | +-------------------------+ |
// +-----------------------------+

NPCChatBoard::NPCChatBoard(
        Widget::VarDir argDir,
        Widget::VarInt argX,
        Widget::VarInt argY,

        ProcessRun *argProc,

        Widget *argParent,
        bool    argAutoDelete)

    : Widget
      {{
          .dir = std::move(argDir),

          .x = std::move(argX),
          .y = std::move(argY),

          .parent
          {
              .widget = argParent,
              .autoDelete = argAutoDelete,
          }
      }}

    , m_processRun(argProc)

    , m_bg
      {
          DIR_UPLEFT,
          0,
          0,

          [this]{ return w(); },
          [this]{ return h(); },

          this,
          false,
      }

    , m_face
      {{
          .x = m_margin,
          .y = m_margin,

          .texLoadFunc = [this]{ return g_progUseDB->retrieve(getNPCFaceKey()); },
          .parent{this},
      }}

    , m_chatBoard
      {{
          .dir = [this]
          {
              if(g_progUseDB->retrieve(getNPCFaceKey())){
                  return DIR_LEFT;
              }
              else{
                  return DIR_NONE;
              }
          },

          .x = [this]
          {
              if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
                  return m_margin * 2 + SDLDeviceHelper::getTextureWidth(texPtr);
              }
              else{
                  return w() / 2;
              }
          },

          .y = [this](const Widget *)
          {
              return h() / 2;
          },

          .font
          {
              .id = 1,
              .size = 12,
          },

          .lineAlign = LALIGN_JUSTIFY,

          .onClickText = [this](const std::unordered_map<std::string, std::string> &attrList, int event)
          {
              if(event == BEVENT_RELEASE){
                  if(const auto id = LayoutBoard::findAttrValue(attrList, "id", nullptr)){
                      const auto autoClose = [id, closeAttr = LayoutBoard::findAttrValue(attrList, "close", nullptr)]() -> bool
                      {
                          return closeAttr ? to_parsedbool(closeAttr) : false;
                      }();
                      onClickEvent(LayoutBoard::findAttrValue(attrList, "path", m_eventPath.c_str()), id, LayoutBoard::findAttrValue(attrList, "args", nullptr), autoClose);
                  }
              }
          },

          .parent{this},
      }}

    , m_buttonClose
      {{
          .x = [this]{ return w() - 40; },
          .y = [this]{ return h() - 43; },

          .texIDList
          {
              .on   = 0X0000001C,
              .down = 0X0000001D,
          },

          .onTrigger = [this](Widget *, int)
          {
              setShow(false);
          },

          .parent{this},
      }}
{
    fflassert(m_margin >= 0);
    setShow(false);

    m_face.setShow([this] -> bool
    {
        return g_progUseDB->retrieve(getNPCFaceKey());
    });

    setSize([this]
    {
        if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
            return m_margin * 3 + SDLDeviceHelper::getTextureWidth(texPtr) + m_chatBoard.w();
        }
        else{
            return m_margin * 2 + m_chatBoard.w();
        }
    },

    [this]
    {
        if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
            return m_margin * 2 + std::max<int>(SDLDeviceHelper::getTextureHeight(texPtr), m_chatBoard.h());
        }
        else{
            return m_margin * 2 + m_chatBoard.h();
        }
    });
}

void NPCChatBoard::loadXML(uint64_t uid, const char *eventPath, const char *xmlString)
{
    fflassert(uidf::isNPChar(uid), uidf::getUIDString(uid));
    fflassert(str_haschar(eventPath));
    fflassert(str_haschar(xmlString));

    m_npcUID = uid;
    m_eventPath = eventPath;

    m_chatBoard.clear();

    const int screenWidth = g_sdlDevice->getRendererWidth();
    const int  boardWidth = std::max<int>(screenWidth / 3, 300);

    if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
        m_chatBoard.setLineWidth(boardWidth - m_margin * 3 - SDLDeviceHelper::getTextureWidth(texPtr));
    }
    else{
        m_chatBoard.setLineWidth(boardWidth - m_margin * 2);
    }

    m_chatBoard.loadXML(xmlString);
}

void NPCChatBoard::onClickEvent(const char *path, const char *id, const char *args, bool autoClose)
{
    if(g_clientArgParser->debugClickEvent){
        m_processRun->addCBLog(CBLOG_SYS, u8"clickEvent: path = %s, id = %s, args = %s", to_cstr(path), to_cstr(id), to_cstr(args));
    }

    fflassert(str_haschar(id));
    m_processRun->sendNPCEvent(m_npcUID, path, id, args ? std::make_optional<std::string>(args) : std::nullopt);

    if(autoClose){
        setShow(false);
    }
}
