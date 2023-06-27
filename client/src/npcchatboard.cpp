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

NPCChatBoard::NPCChatBoard(ProcessRun *proc, Widget *pwidget, bool autoDelete)
    : Widget(DIR_UPLEFT, 0, 0, 386, 204, pwidget, autoDelete)
    , m_margin(20)
    , m_process(proc)
    , m_chatBoard
      {
          DIR_UPLEFT,
          m_margin,
          m_margin,
          386 - m_margin * 2,

          false,
          {0, 0, 0, 0},
          false,

          1,
          12,
          0,

          colorf::WHITE + colorf::A_SHF(255),
          0,

          LALIGN_LEFT,
          0,
          0,

          [this](const std::unordered_map<std::string, std::string> &attrList, int oldEvent, int newEvent)
          {
              if(oldEvent == BEVENT_DOWN && newEvent == BEVENT_ON){
                  const auto fnFindAttrValue = [&attrList](const char *key, const char *valDefault) -> const char *
                  {
                      if(auto p = attrList.find(key); p != attrList.end() && str_haschar(p->second)){
                          return p->second.c_str();
                      }
                      return valDefault;
                  };

                  const auto autoClose = [attrVal = str_toupper(fnFindAttrValue("close", "false"))]() -> bool
                  {
                      for(const auto  trueStr: {"1", "TRUE" }){ if(attrVal ==  trueStr){ return true ; }}
                      for(const auto falseStr: {"0", "FALSE"}){ if(attrVal == falseStr){ return false; }}
                      throw fflerror("invalid close attribute: %s", attrVal.c_str());
                  }();

                  if(const auto id = fnFindAttrValue("id", nullptr)){
                      onClickEvent(fnFindAttrValue("path", m_eventPath.c_str()), id, fnFindAttrValue("arg", nullptr), autoClose || to_sv(id) == SYS_EXIT);
                  }
              }
          },
          this,
      }
    , m_buttonClose
      {
          DIR_UPLEFT,
          100,
          100,
          {SYS_U32NIL, 0X0000001C, 0X0000001D},
          {
              SYS_U32NIL,
              SYS_U32NIL,
              0X01020000 + 105,
          },

          nullptr,
          nullptr,
          [this]()
          {
              setShow(false);
              m_process->sendNPCEvent(m_npcUID, m_eventPath, SYS_EXIT);
          },

          0,
          0,
          0,
          0,

          true,
          true,
          this,
      }
{
    if(m_margin < 0){
        throw fflerror("invalid margin: %d", m_margin);
    }

    auto fnAssertImage = [](uint32_t key, int w, int h)
    {
        if(auto texPtr = g_progUseDB->retrieve(key)){
            if(SDLDeviceHelper::getTextureSize(texPtr) == std::tuple<int, int>{w, h}){
                return;
            }
        }
        throw fflerror("image assertion failed: key = %llu, w = %d, h = %d", to_llu(key), w, h);
    };

    fnAssertImage(0X00000050, 386, 160);
    fnAssertImage(0X00000051, 386, 160);
    fnAssertImage(0X00000052, 386,  20);
    fnAssertImage(0X00000053, 386,  44);
    fnAssertImage(0X00000054, 386,  44);
    setShow(false);
}

void NPCChatBoard::drawWithNPCFace() const
{
    // |<------------386 ----------->|
    // +-----------------------------+ ---
    // | +----+ +------------------+ |  ^
    // | |    | |                  | |  |
    // | |    | |                  | |  160 + k * 20 + 44
    // | |    | |                  | |  |
    // | +----+ +------------------+ |  v
    // +-----------------------------+ ---

    auto facePtr = g_progUseDB->retrieve(getNPCFaceKey());
    if(!facePtr){
        throw fflerror("no valid NPC face image");
    }

    drawFrame();
    g_sdlDevice->drawTexture(facePtr, m_margin, (h() - SDLDeviceHelper::getTextureHeight(facePtr)) / 2);

    m_chatBoard.draw();
    m_buttonClose.draw();
}

void NPCChatBoard::drawPlain() const
{
    // |<------------386 ----------->|
    // +-----------------------------+ ---
    // | +-------------------------+ |  ^
    // | |                         | |  |
    // | |                         | |  160 + k * 20 + 44
    // | |                         | |  |
    // | +-------------------------+ |  v
    // +-----------------------------+ ---

    drawFrame();
    m_chatBoard.draw();
    m_buttonClose.draw();
}

void NPCChatBoard::drawEx(int, int, int, int, int, int) const
{
    if(g_progUseDB->retrieve(getNPCFaceKey())){
        drawWithNPCFace();
    }
    else{
        drawPlain();
    }
}

void NPCChatBoard::loadXML(uint64_t uid, const char *eventPath, const char *xmlString)
{
    fflassert(uidf::isNPChar(uid), uidf::getUIDString(uid));
    fflassert(str_haschar(eventPath));
    fflassert(str_haschar(xmlString));

    m_npcUID = uid;
    m_eventPath = eventPath;
    m_chatBoard.clear();

    if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
        m_chatBoard.setLineWidth(386 - m_margin * 3 - SDLDeviceHelper::getTextureWidth(texPtr));
    }
    else{
        m_chatBoard.setLineWidth(386 - m_margin * 2);
    }
    m_chatBoard.loadXML(xmlString);

    m_w = 386;
    m_h = 160 + 20 * getMiddleCount() + 44;

    m_buttonClose.moveTo(w() - 40, h() - 43);
    if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
        m_chatBoard.moveTo(m_margin * 2 + SDLDeviceHelper::getTextureWidth(texPtr), (h() - m_chatBoard.h()) / 2);
    }
    else{
        m_chatBoard.moveTo(m_margin, (h() - m_chatBoard.h()) / 2);
    }
}

void NPCChatBoard::onClickEvent(const char *path, const char *id, const char *arg, bool autoClose)
{
    if(g_clientArgParser->debugClickEvent){
        m_process->addCBLog(CBLOG_SYS, u8"clickEvent: path = %s, id = %s, arg = %s", to_cstr(path), to_cstr(id), to_cstr(arg));
    }

    fflassert(str_haschar(id));
    m_process->sendNPCEvent(m_npcUID, path, id, arg ? std::make_optional<std::string>(arg) : std::nullopt);

    if(autoClose){
        setShow(false);
    }
}

int NPCChatBoard::getMiddleCount() const
{
    if(auto texPtr = g_progUseDB->retrieve(getNPCFaceKey())){
        const auto [faceW, faceH] = SDLDeviceHelper::getTextureSize(texPtr);
        if(faceW + m_margin * 3 + m_chatBoard.w() >= 386){
            throw fflerror("chat board size error");
        }

        const auto minBoardHeight = 160 + 44 - m_margin * 2;
        const auto maxHeight = std::max<int>(faceH, m_chatBoard.h());

        if(maxHeight < minBoardHeight){
            return 0;
        }
        return (maxHeight - minBoardHeight + 19) / 20;
    }

    // not using NPC face key
    // plain drawing

    const int minPlainHeight = 160 + 44 - m_margin * 2;
    if(m_chatBoard.h() < minPlainHeight){
        return 0;
    }
    return (m_chatBoard.h() - minPlainHeight + 19) / 20;
}

void NPCChatBoard::drawFrame() const
{
    const auto k = getMiddleCount();
    auto frameUp = g_progUseDB->retrieve(0X00000051);
    auto frameMid = g_progUseDB->retrieve(0X00000052);
    auto frameDown = g_progUseDB->retrieve(0X00000053);

    g_sdlDevice->drawTexture(frameUp, 0, 0);
    for(int i = 0; i < k; ++i){
        g_sdlDevice->drawTexture(frameMid, 0, 160 + i * 20);
    }
    g_sdlDevice->drawTexture(frameDown, 0, 160 + k * 20);
}
