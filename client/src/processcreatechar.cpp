#include "mathf.hpp"
#include "client.hpp"
#include "fflerror.hpp"
#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "bgmusicdb.hpp"
#include "pngtexoffdb.hpp"
#include "processcreatechar.hpp"

extern Client *g_client;
extern SDLDevice *g_sdlDevice;
extern BGMusicDB *g_bgmDB;
extern PNGTexDB *g_progUseDB;
extern PNGTexOffDB *g_selectCharDB;

ProcessCreateChar::ProcessCreateChar()
    : Process()
    , m_warrior(DIR_UPLEFT, 339, 539, {0X0D000030, 0X0D000031, 0X0D000032}, nullptr, nullptr, [this](){ m_job = JOB_WARRIOR; })
    , m_wizard (DIR_UPLEFT, 381, 539, {0X0D000040, 0X0D000041, 0X0D000042}, nullptr, nullptr, [this](){ m_job = JOB_WIZARD ; })
    , m_taoist (DIR_UPLEFT, 424, 539, {0X0D000050, 0X0D000051, 0X0D000052}, nullptr, nullptr, [this](){ m_job = JOB_TAOIST ; })
    , m_submit (DIR_UPLEFT, 512, 549, {0X0D000010, 0X0D000011, 0X0D000012}, nullptr, nullptr, [this](){ onSubmit(); }, 0, 0, 0, 0, true, false)
    , m_exit   (DIR_UPLEFT, 554, 549, {0X0D000020, 0X0D000021, 0X0D000022}, nullptr, nullptr, [this](){ onExit();   }, 0, 0, 0, 0, true, false)

    , m_nameLine
      {
          DIR_UPLEFT,
          355,
          520,
          85,
          15,

          1,
          12,

          0,
          colorf::WHITE + colorf::A_SHF(255),

          1,
          colorf::WHITE + colorf::A_SHF(255),

          nullptr,
          [this]()
          {
              onSubmit();
          },
      }

    , m_notifyBoard
      {
          DIR_UPLEFT,
          0,
          0,
          0,
          1,
          15,
          0,
          colorf::YELLOW + colorf::A_SHF(255),
          5000,
          10,
      }
{
    g_sdlDevice->playBGM(g_bgmDB->retrieve(0X00000003));
}

void ProcessCreateChar::update(double fUpdateTime)
{
    m_aniTime += fUpdateTime;
    m_notifyBoard.update(fUpdateTime);
}

void ProcessCreateChar::draw() const
{
    SDLDeviceHelper::RenderNewFrame newFrame;
    if(auto texPtr = g_progUseDB->retrieve(0X0D000000)){
        g_sdlDevice->drawTexture(texPtr, 0, 0);
    }

    drawChar( true, 193, 215);
    drawChar(false, 495, 220);

    if(auto texPtr = g_progUseDB->retrieve(0X0D000002)){
        SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::RGBA(255, 255, 255, 150));
        g_sdlDevice->drawTexture(texPtr, 268, 555);
    }

    g_sdlDevice->fillRectangle(colorf::RGBA(  0,   0,   0, 255), 355, 520, 90, 15);
    m_nameLine.draw();
    g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 100), 355, 520, 90, 15);

    if(auto texPtr = g_progUseDB->retrieve(0X0D000001)){
        g_sdlDevice->drawTexture(texPtr, 320, 500);
    }

    m_warrior.draw();
    m_wizard .draw();
    m_taoist .draw();

    m_submit.draw();
    m_exit  .draw();

    const int notifX = (800 - m_notifyBoard.pw()) / 2;
    const int notifY = (600 - m_notifyBoard. h()) / 2;
    const int margin = 15;

    if(!m_notifyBoard.empty()){
        g_sdlDevice->fillRectangle(colorf::RGBA(0, 0,   0, 128), notifX - margin, notifY - margin, m_notifyBoard.pw() + margin * 2, m_notifyBoard.h() + margin * 2, 8);
        g_sdlDevice->drawRectangle(colorf::RGBA(0, 0, 255, 128), notifX - margin, notifY - margin, m_notifyBoard.pw() + margin * 2, m_notifyBoard.h() + margin * 2, 8);
    }
    m_notifyBoard.drawAt(DIR_UPLEFT, notifX, notifY);
}

void ProcessCreateChar::processEvent(const SDL_Event &event)
{
    bool tookEvent = false;
    tookEvent |= m_warrior .processEvent(event, !tookEvent);
    tookEvent |= m_wizard  .processEvent(event, !tookEvent);
    tookEvent |= m_taoist  .processEvent(event, !tookEvent);
    tookEvent |= m_submit  .processEvent(event, !tookEvent);
    tookEvent |= m_exit    .processEvent(event, !tookEvent);
    tookEvent |= m_nameLine.processEvent(event, !tookEvent);

    if(!tookEvent){
        switch(event.type){
            case SDL_MOUSEBUTTONDOWN:
                {
                    const auto [px, py] = SDLDeviceHelper::getMousePLoc();
                    if(mathf::pointInRectangle(px, py, 200, 290, 90, 260)){
                        m_activeGender = true;
                    }
                    else if(mathf::pointInRectangle(px, py, 510, 310, 90, 260)){
                        m_activeGender = false;
                    }
                    break;
                }
            default:
                {
                    break;
                }
        }
    }
}

uint32_t ProcessCreateChar::charFrameCount(int job, bool gender)
{
    fflassert(job >= JOB_BEGIN);
    fflassert(job <  JOB_END  );

    const static std::map<std::tuple<int, bool, int>, uint32_t> s_frameCount
    {
        #include "selectcharframecount.inc"
    };

    if(auto p = s_frameCount.find({job, gender, 4}); p != s_frameCount.end()){
        return p->second;
    }
    return 0;
}

uint32_t ProcessCreateChar::charGfxBaseID(int job, bool gender)
{
    fflassert(job >= JOB_BEGIN);
    fflassert(job <  JOB_END  );

    // 14     : max =  2   shadow
    // 13     : max =  2   magic
    // 10 - 12: max =  8   job
    // 09     : max =  2   gender: male as true
    // 05 - 08: max = 16   motion
    // 00 - 04: max = 32   frame

    return 0
        + (to_u32(job - JOB_BEGIN) << 10)
        + (to_u32(gender         ) <<  9)
        + (to_u32(4              ) <<  5);
}

void ProcessCreateChar::onSubmit()
{
    CMCreateChar cmCC;
    std::memset(&cmCC, 0, sizeof(cmCC));
    const auto nameStr = m_nameLine.getRawString();

    if(nameStr.empty() || nameStr.size() >= cmCC.name.capacity()){
        m_notifyBoard.addLog(u8"无效的角色名");
    }
    else{
        cmCC.name.assign(nameStr);
        cmCC.job = m_job;
        cmCC.gender = m_activeGender;
        g_client->send(CM_CREATECHAR, cmCC);
        m_notifyBoard.addLog(u8"提交中");
    }
}

void ProcessCreateChar::onExit()
{
    g_client->requestProcess(PROCESSID_SELECTCHAR);
}

void ProcessCreateChar::setGUIActive(bool active)
{
    m_warrior .active(active);
    m_wizard  .active(active);
    m_taoist  .active(active);
    m_submit  .active(active);
    m_exit    .active(active);
    m_nameLine.active(active);
}

void ProcessCreateChar::drawChar(bool gender, int drawX, int drawY) const
{
    const uint32_t frameCount = charFrameCount(m_job, gender);
    if(frameCount <= 0){
        return;
    }

    const uint32_t shadowMask = to_u32(1) << 14;
    const uint32_t  magicMask = to_u32(1) << 13;

    const bool active = (gender == m_activeGender);
    const uint32_t frameIndex = charGfxBaseID(m_job, gender) + (active ? (absFrame() % frameCount) : 0);

    // TODO hack code
    // seems for female warrior offset-y is mis-aligned
    if(gender == false && m_job == JOB_WARRIOR){
        drawY += 40;
    }

    const auto fnDrawTexture = [drawX, drawY, active](uint32_t texIndex, bool alpha = false) -> bool
    {
        if(const auto [texPtr, dx, dy] = g_selectCharDB->retrieve(texIndex); texPtr){
            const auto modColor = [active, alpha]() -> uint32_t
            {
                if(active){
                    return colorf::RGBA(255, 255, 255, alpha ? 150 : 255);
                }
                else{
                    return colorf::RGBA(128, 128, 128, alpha ? 150 : 255);
                }
            }();

            SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, modColor);
            g_sdlDevice->drawTexture(texPtr, drawX + dx, drawY + dy);
            return true;
        }
        return false;
    };

    if(fnDrawTexture(frameIndex)){
        fnDrawTexture(frameIndex | shadowMask, true);
        fnDrawTexture(frameIndex);
        fnDrawTexture(frameIndex | magicMask);
    }
}
