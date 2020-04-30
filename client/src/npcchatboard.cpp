/*
 * =====================================================================================
 *
 *       Filename: npcchatboard.cpp
 *        Created: 04/12/2020 19:03:35
 *    Description:
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *   Organization: USTC
 *
 * =====================================================================================
 */

#include "toll.hpp"
#include "uidf.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processrun.hpp"
#include "npcchatboard.hpp"

extern PNGTexDB *g_ProgUseDB;
extern SDLDevice *g_SDLDevice;

NPCChatBoard::NPCChatBoard(ProcessRun *proc)
    : Widget(0, 0, 386, 204, nullptr, false)
    , m_margin(20)
    , m_processRun(proc)
    , m_chatBoard
      {
          m_margin,
          m_margin,
          386 - m_margin * 2,

          false,
          {0, 0, 0, 0},
          false,

          1,
          12,
          0,

          colorf::WHITE,
          LALIGN_LEFT,
          0,
          0,

          [this](const std::string &id, int event)
          {
              if(event == BEVENT_DOWN){
                  onClickEvent(id);
              }
          },
          this,
      }
    , m_buttonClose
      {
          100,
          100,
          {SYS_TEXNIL, 0X0000001C, 0X0000001D},

          nullptr,
          [this]()
          {
              show(false);
          },

          0,
          0,
          0,
          0,

          true,
          this,
      }
{
    if(m_margin < 0){
        throw fflerror("invalid margin: %d", m_margin);
    }

    auto fnAssertImage = [](uint32_t key, int w, int h)
    {
        if(auto ptex = g_ProgUseDB->Retrieve(key)){
            if(SDLDevice::getTextureSize(ptex) == std::tuple<int, int>{w, h}){
                return;
            }
        }
        throw fflerror("image assertion failed: key = %llu, w = %d, h = %d", toLLU(key), w, h);
    };

    fnAssertImage(0X00000050, 386, 160);
    fnAssertImage(0X00000051, 386, 160);
    fnAssertImage(0X00000052, 386,  20);
    fnAssertImage(0X00000053, 386,  44);
    fnAssertImage(0X00000054, 386,  44);
    show(false);
}

void NPCChatBoard::drawWithNPCFace()
{
    // |<------------386 ----------->|
    // +-----------------------------+ ---
    // | +----+ +------------------+ |  ^
    // | |    | |                  | |  |
    // | |    | |                  | |  160 + k * 20 + 44
    // | |    | |                  | |  |
    // | +----+ +------------------+ |  v
    // +-----------------------------+ ---

    auto frameUp = g_ProgUseDB->Retrieve(0X00000051);
    auto frameMid = g_ProgUseDB->Retrieve(0X00000052);
    auto frameDown = g_ProgUseDB->Retrieve(0X00000053);
    const uint32_t faceKey = 0X50000000 | uidf::getLookID(m_NPCUID);

    auto faceFrame = g_ProgUseDB->Retrieve(faceKey);
    if(!faceFrame){
        throw fflerror("no valid NPC face image");
    }

    const auto [faceW, faceH] = SDLDevice::getTextureSize(faceFrame);
    const int chatBoardWidth = 386 - m_margin * 3 - faceW;
    if(m_chatBoard.w() > chatBoardWidth){
        m_chatBoard.setLineWidth(chatBoardWidth);
    }

    const auto minHeight = std::max<int>(faceH, m_chatBoard.h());
    const auto k = [minHeight, this]() -> int
    {
        const int minBoardHeight = 160 + 44 - m_margin * 2;
        if(minHeight < minBoardHeight){
            return 0;
        }
        return (minHeight - minBoardHeight + 19) / 20;
    }();

    g_SDLDevice->DrawTexture(frameUp, 0, 0);
    for(int i = 0; i < k; ++i){
        g_SDLDevice->DrawTexture(frameMid, 0, i * 20);
    }

    const int boardHeight = 160 + k * 20 + 44;
    g_SDLDevice->DrawTexture(frameDown, 0, k * 20);
    g_SDLDevice->DrawTexture(faceFrame, 20, (boardHeight - faceH) / 2);
    m_chatBoard.moveTo(m_margin * 2 + faceW, (boardHeight - m_chatBoard.h()) / 2);
    m_chatBoard.draw();
    m_buttonClose.draw();
}

void NPCChatBoard::drawPlain()
{
    // |<------------386 ----------->|
    // +-----------------------------+ ---
    // | +-------------------------+ |  ^
    // | |                         | |  |
    // | |                         | |  160 + k * 20 + 44
    // | |                         | |  |
    // | +-------------------------+ |  v
    // +-----------------------------+ ---

    auto frameUp = g_ProgUseDB->Retrieve(0X00000051);
    auto frameMid = g_ProgUseDB->Retrieve(0X00000052);
    auto frameDown = g_ProgUseDB->Retrieve(0X00000053);

    const int boardWidth = 386 - m_margin * 2;
    if(m_chatBoard.w() < boardWidth){
        m_chatBoard.setLineWidth(boardWidth);
    }

    const auto k = [this]() -> int
    {
        const int minBoardHeight = 160 + 44 - m_margin * 2;
        const int minHeight = m_chatBoard.h();

        if(minHeight < minBoardHeight){
            return 0;
        }
        return (minHeight - minBoardHeight + 19) / 20;
    }();

    g_SDLDevice->DrawTexture(frameUp, 0, 0);
    for(int i = 0; i < k; ++i){
        g_SDLDevice->DrawTexture(frameMid, 0, i * 20);
    }

    const int boardHeight = 160 + k * 20 + 44;
    g_SDLDevice->DrawTexture(frameDown, 0, 160 + k * 20);
    m_chatBoard.moveTo(m_margin, (boardHeight - m_chatBoard.h()) / 2);
    m_chatBoard.draw();

    m_buttonClose.moveTo(346, boardHeight - 43);
    m_buttonClose.draw();
}

void NPCChatBoard::drawEx(int, int, int, int, int, int)
{
    const uint32_t faceKey = 0X50000000 | uidf::getLookID(m_NPCUID);
    if(g_ProgUseDB->Retrieve(faceKey)){
        drawWithNPCFace();
    }
    else{
        drawPlain();
    }
}

void NPCChatBoard::loadXML(uint64_t uid, const char *xmlString)
{
    if(uidf::getUIDType(uid) != UID_NPC){
        throw fflerror("invalid uid type: %s", uidf::getUIDTypeString(uid));
    }

    m_NPCUID = uid;
    m_chatBoard.loadXML(xmlString);
}

void NPCChatBoard::onClickEvent(const std::string &id)
{
    m_processRun->AddOPLog(/* OUTPORT_CONTROLBOARD */ 3 << 1, 1, "", "clickEvent id: %s", id.c_str());
    const int eventID = [&id]() -> int
    {
        try{
            return std::stoi(id);
        }
        catch(...){
            //
        }
        return -1;
    }();

    if(eventID >= 0){
        m_processRun->sendNPCEventID(m_NPCUID, eventID);
    }

    else if(id == "close"){
        show(false);
    }
}
