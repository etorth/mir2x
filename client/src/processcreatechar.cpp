#include "mathf.hpp"
#include "client.hpp"
#include "fflerror.hpp"
#include "sdldevice.hpp"
#include "pngtexdb.hpp"
#include "bgmusicdb.hpp"
#include "soundeffectdb.hpp"
#include "pngtexoffdb.hpp"
#include "imeboard.hpp"
#include "clientargparser.hpp"
#include "processcreatechar.hpp"

extern Client *g_client;
extern SDLDevice *g_sdlDevice;
extern IMEBoard *g_imeBoard;
extern BGMusicDB *g_bgmDB;
extern SoundEffectDB *g_seffDB;
extern PNGTexDB *g_progUseDB;
extern PNGTexOffDB *g_selectCharDB;
extern ClientArgParser *g_clientArgParser;

ProcessCreateChar::ProcessCreateChar()
    : Process()
    , m_warrior(DIR_UPLEFT, 339, 539, {0X0D000030, 0X0D000031, 0X0D000032}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ m_job = JOB_WARRIOR; })
    , m_wizard (DIR_UPLEFT, 381, 539, {0X0D000040, 0X0D000041, 0X0D000042}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ m_job = JOB_WIZARD ; })
    , m_taoist (DIR_UPLEFT, 424, 539, {0X0D000050, 0X0D000051, 0X0D000052}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ m_job = JOB_TAOIST ; })
    , m_submit (DIR_UPLEFT, 512, 549, {0X0D000010, 0X0D000011, 0X0D000012}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ onSubmit(); }, 0, 0, 0, 0, true, false)
    , m_exit   (DIR_UPLEFT, 554, 549, {0X0D000020, 0X0D000021, 0X0D000022}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, nullptr, [this](Widget *, int){ onExit();   }, 0, 0, 0, 0, true, false)

    , m_nameBox
      {
          DIR_UPLEFT,
          355,
          520,
          85,
          15,

          true,

          1,
          12,

          0,
          colorf::WHITE_A255,

          1,
          colorf::WHITE_A255,

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
    g_sdlDevice->playBGM(g_bgmDB->retrieve(0X00040001));
    if(!g_clientArgParser->disableIME){
        g_imeBoard->dropFocus();
    }
}

ProcessCreateChar::~ProcessCreateChar()
{
    g_sdlDevice->stopBGM();
    g_sdlDevice->stopSoundEffect();
}

void ProcessCreateChar::update(double fUpdateTime)
{
    m_aniTime += fUpdateTime;
    m_notifyBoard.update(fUpdateTime);
    if(!g_clientArgParser->disableIME){
        g_imeBoard->update(fUpdateTime);
    }

    if(const uint32_t frameCount = charFrameCount(m_job, m_activeGender); frameCount > 0){
        if(const auto currAbsFrame = absFrame(); ((currAbsFrame % frameCount) == 0) && (m_lastStartAbsFrame != currAbsFrame)){
            playMagicSoundEffect();
            m_lastStartAbsFrame = currAbsFrame;
        }
    }
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
    m_nameBox.drawRoot();
    g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 100), 355, 520, 90, 15);

    if(auto texPtr = g_progUseDB->retrieve(0X0D000001)){
        g_sdlDevice->drawTexture(texPtr, 320, 500);
    }

    m_warrior.drawRoot();
    m_wizard .drawRoot();
    m_taoist .drawRoot();

    m_submit.drawRoot();
    m_exit  .drawRoot();

    if(!g_clientArgParser->disableIME){
        g_imeBoard->drawRoot();
    }

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
    if(!g_clientArgParser->disableIME){
        tookEvent |= g_imeBoard->applyRootEvent(event, !tookEvent, 0, 0);
    }

    tookEvent |= m_warrior.applyRootEvent(event, !tookEvent, 0, 0);
    tookEvent |= m_wizard .applyRootEvent(event, !tookEvent, 0, 0);
    tookEvent |= m_taoist .applyRootEvent(event, !tookEvent, 0, 0);
    tookEvent |= m_submit .applyRootEvent(event, !tookEvent, 0, 0);
    tookEvent |= m_exit   .applyRootEvent(event, !tookEvent, 0, 0);
    tookEvent |= m_nameBox.applyRootEvent(event, !tookEvent, 0, 0);

    if(!tookEvent){
        switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_TAB:
                        {
                            m_nameBox.setFocus(true);
                            break;
                        }
                    default:
                        {
                            break;
                        }
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
                {
                    const auto [px, py] = SDLDeviceHelper::getMousePLoc();
                    if(mathf::pointInRectangle(px, py, 200, 290, 90, 260)){
                        m_activeGender = true;
                    }
                    else if(mathf::pointInRectangle(px, py, 510, 310, 90, 260)){
                        m_activeGender = false;
                    }

                    m_aniTime = 0.0;
                    m_lastStartAbsFrame = UINT32_MAX; // force playing sound effect when switching chars
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
    const auto nameStr = m_nameBox.getRawString();

    if(nameStr.empty() || nameStr.size() >= cmCC.name.capacity()){
        m_notifyBoard.addLog(u8"无效的角色名");
    }
    else{
        cmCC.name.assign(nameStr);
        cmCC.job = m_job;
        cmCC.gender = m_activeGender;
        g_client->send({CM_CREATECHAR, cmCC});
        m_notifyBoard.addLog(u8"提交中");
    }
}

void ProcessCreateChar::onExit()
{
    g_client->requestProcess(PROCESSID_SELECTCHAR);
}

void ProcessCreateChar::setGUIActive(bool active)
{
    m_warrior.setActive(active);
    m_wizard .setActive(active);
    m_taoist .setActive(active);
    m_submit .setActive(active);
    m_exit   .setActive(active);
    m_nameBox.setActive(active);
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

void ProcessCreateChar::playMagicSoundEffect()
{
    const int offGender = to_d(m_activeGender);
    const int offJob = m_job - JOB_BEGIN;

    const uint32_t seffID = UINT32_C(0X00010000) // base
        | (to_u32(offGender) << 4)               //
        | (to_u32(offJob   ) << 8)               //
        | (to_u32(0        ) << 0);              // 0 for create, 1 for select

    g_sdlDevice->playSoundEffect(g_seffDB->retrieve(seffID));
}
