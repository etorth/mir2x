#include "log.hpp"
#include "jobf.hpp"
#include "pathf.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "bgmusicdb.hpp"
#include "soundeffectdb.hpp"
#include "pngtexoffdb.hpp"
#include "sdldevice.hpp"
#include "layoutboard.hpp"
#include "processselectchar.hpp"

extern Log *g_log;
extern Client *g_client;
extern BGMusicDB *g_bgmDB;
extern SoundEffectDB *g_seffDB;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern PNGTexOffDB *g_selectCharDB;

ProcessSelectChar::ProcessSelectChar()
    : Process()
	, m_start (DIR_UPLEFT, 335,  75, {0X0C000030, 0X0C000030, 0X0C000031}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, [this](Widget *){ onStart (); })
	, m_create(DIR_UPLEFT, 565, 130, {0X0C000010, 0X0C000010, 0X0C000011}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, [this](Widget *){ onCreate(); })
	, m_delete(DIR_UPLEFT, 110, 305, {0X0C000020, 0X0C000020, 0X0C000021}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, [this](Widget *){ onDelete(); })
	, m_exit  (DIR_UPLEFT,  45, 544, {0X0C000040, 0X0C000040, 0X0C000041}, {SYS_U32NIL, SYS_U32NIL, 0X01020000 + 105}, nullptr, nullptr, [this](Widget *){ onExit  (); })

    , m_notifyBoard
      {
          DIR_UPLEFT,
          0,
          0,
          0, // automatically resize
          1,
          15,
          0,
          colorf::YELLOW + colorf::A_SHF(255),
          5000,
          1,
      }

    , m_deleteInput
      {
          DIR_NONE,
          400,
          300,
          true,
      }
{
    m_start .setActive(false);
    m_create.setActive(false);
    m_delete.setActive(false);
    m_notifyBoard.addLog(u8"正在下载游戏角色");
    g_client->send(CM_QUERYCHAR);
    g_sdlDevice->playBGM(g_bgmDB->retrieve(0X00040002));
}

ProcessSelectChar::~ProcessSelectChar()
{
    g_sdlDevice->stopBGM();
    g_sdlDevice->stopSoundEffect();
}

void ProcessSelectChar::update(double fUpdateTime)
{
    m_charAniTime += fUpdateTime;
    m_notifyBoard.update(fUpdateTime);

    if(hasChar()){
        switchCharGfx();
    }
}

void ProcessSelectChar::draw() const
{
    SDLDeviceHelper::RenderNewFrame newFrame;
    if(auto texPtr = g_progUseDB->retrieve(0X0C000000)){
        g_sdlDevice->drawTexture(texPtr, 0, 0);
    }

    m_start .draw();
    m_create.draw();
    m_delete.draw();
    m_exit  .draw();

    if(hasChar()){
        drawChar();
        drawCharName();
    }

    m_deleteInput.draw();

    const int notifX = (800 - m_notifyBoard.pw()) / 2;
    const int notifY = (600 - m_notifyBoard. h()) / 2;
    const int margin = 15;

    if(!m_notifyBoard.empty()){
        g_sdlDevice->fillRectangle(colorf::RGBA(0, 0,   0, 128), notifX - margin, notifY - margin, m_notifyBoard.pw() + margin * 2, m_notifyBoard.h() + margin * 2, 8);
        g_sdlDevice->drawRectangle(colorf::RGBA(0, 0, 255, 128), notifX - margin, notifY - margin, m_notifyBoard.pw() + margin * 2, m_notifyBoard.h() + margin * 2, 8);
    }
    m_notifyBoard.drawAt(DIR_UPLEFT, notifX, notifY);
}

void ProcessSelectChar::processEvent(const SDL_Event &event)
{
    bool tookEvent = false;
    tookEvent |= m_start      .processEvent(event, !tookEvent);
    tookEvent |= m_create     .processEvent(event, !tookEvent);
    tookEvent |= m_delete     .processEvent(event, !tookEvent);
    tookEvent |= m_exit       .processEvent(event, !tookEvent);
    tookEvent |= m_deleteInput.processEvent(event, !tookEvent);

    if(!tookEvent){
    }
}

void ProcessSelectChar::onStart()
{
    if(m_smChar.has_value()){
        if(m_smChar.value().name.empty()){
            m_notifyBoard.addLog(u8"请先创建游戏角色");
        }
        else{
            g_client->send(CM_ONLINE);
        }
    }
    else{
        m_notifyBoard.addLog(u8"正在下载游戏角色");
    }
}

void ProcessSelectChar::onCreate()
{
    if(m_smChar.has_value()){
        if(m_smChar.value().name.empty()){
            g_client->requestProcess(PROCESSID_CREATECHAR);
        }
        else{
            m_notifyBoard.addLog(u8"一个账号只能创建一个游戏角色");
        }
    }
    else{
        m_notifyBoard.addLog(u8"正在下载游戏角色");
    }
}

void ProcessSelectChar::onDelete()
{
    if(hasChar()){
        m_deleteInput.setShow(true);
        m_deleteInput.waitInput(u8"<layout><par>删除的角色将无法还原，请谨慎操作。如果确定删除，请输入游戏密码，并点击YES。</par></layout>", [this](std::u8string inputString)
        {
            CMDeleteChar cmDC;
            std::memset(&cmDC, 0, sizeof(cmDC));
            if(inputString.empty() || inputString.size() > SYS_PWDSIZE){
                m_notifyBoard.addLog(u8"无效的密码");
            }
            else{
                cmDC.password.assign(inputString);
                g_client->send({CM_DELETECHAR, cmDC});
            }
            m_deleteInput.setShow(false);
        });
    }
    else{
        m_notifyBoard.addLog(u8"此账号没有角色");
    }
}

void ProcessSelectChar::onExit()
{
    g_client->requestProcess(PROCESSID_SELECTCHAR);
}

void ProcessSelectChar::drawCharName() const
{
    if(m_smChar.has_value() && !m_smChar.value().name.empty()){
        const auto exp = m_smChar.value().exp;
        const auto name = m_smChar.value().name.to_str();

        fflassert(str_haschar(name));

        std::u8string xmlStr;
        xmlStr += str_printf(u8R"###( <layout> )###""\n");
        xmlStr += str_printf(u8R"###(     <par color='RGB(237,226,200)'>角色：%s</par> )###""\n", to_cstr(name));
        xmlStr += str_printf(u8R"###(     <par color='RGB(175,196,175)'>等级：%d</par> )###""\n", to_d(SYS_LEVEL(exp)));
        for(const auto jobStr: jobf::jobName(m_smChar.value().job)){
            if(jobStr){
                xmlStr += str_printf(u8R"###( <par color='RGB(231,231,189)'>职业：%s</par> )###""\n", to_cstr(jobStr));
            }
            else{
                break;
            }
        }
        xmlStr += str_printf(u8R"###( </layout> )###""\n");

        const LayoutBoard charBoard
        {
            DIR_UPLEFT,
            0,
            0,
            200,

            to_cstr(xmlStr),
            0,

            {},
            false,
            false,
            false,
            false,

            1,
            15,
        };

        const int drawBoardX = 120;
        const int drawBoardY = 260 - charBoard.h();
        const int drawBoardMargin = 15;

        g_sdlDevice->fillRectangle(colorf::RGBA(  0,   0,   0, 128), drawBoardX - drawBoardMargin, drawBoardY - drawBoardMargin, charBoard.w() + drawBoardMargin * 2, charBoard.h() + drawBoardMargin * 2, 5);
        g_sdlDevice->drawRectangle(colorf::RGBA(231, 231, 189, 128), drawBoardX - drawBoardMargin, drawBoardY - drawBoardMargin, charBoard.w() + drawBoardMargin * 2, charBoard.h() + drawBoardMargin * 2, 5);
        charBoard.drawAt(DIR_UPLEFT, drawBoardX, drawBoardY);
    }
}

uint32_t ProcessSelectChar::charFrameCount() const
{
    if(m_smChar.has_value() && !m_smChar.value().name.empty()){
        const bool gender = m_smChar.value().gender;
        const int firstJob = jobf::firstJob(m_smChar.value().job);

        fflassert(jobf::jobValid(firstJob), firstJob);
        fflassert(m_charAni >= 0);
        fflassert(m_charAni <  4);

        // motion:
        // 0 : stand, inactive
        // 1 : on select
        // 2 : stand, active
        // 3 : on deselect
        // 4 : on create char in ProcessCreateChar, not used here

        // (job, gender, motion) -> frameCount
        const static std::map<std::tuple<int, bool, int>, uint32_t> s_frameCount
        {
            #include "selectcharframecount.inc"
        };

        if(auto p = s_frameCount.find({firstJob, gender, m_charAni}); p != s_frameCount.end()){
            return p->second;
        }
    }
    return 0;
}

std::optional<uint32_t> ProcessSelectChar::charGfxBaseID() const
{
    if(m_smChar.has_value() && !m_smChar.value().name.empty()){
        const bool gender = m_smChar.value().gender;
        const auto jobIndexOpt = jobf::jobGfxIndex(m_smChar.value().job).front();

        fflassert(jobIndexOpt.has_value());
        fflassert(m_charAni >= 0);
        fflassert(m_charAni <  4);

        // 14     : max =  2   shadow
        // 13     : max =  2   magic
        // 10 - 12: max =  8   job
        // 09     : max =  2   gender: male as true
        // 05 - 08: max = 16   motion
        // 00 - 04: max = 32   frame

        return 0
            + (to_u32(jobIndexOpt.value()) << 10)
            + (to_u32(gender             ) <<  9)
            + (to_u32(m_charAni          ) <<  5);
    }
    return {};
}

void ProcessSelectChar::drawChar() const
{
    if(m_smChar.has_value() && !m_smChar.value().name.empty()){
        const auto frameCount = charFrameCount();
        if(frameCount <= 0){
            return;
        }

        const auto gfxIDOpt = charGfxBaseID();
        if(!gfxIDOpt.has_value()){
            return;
        }

        const uint32_t shadowMask = to_u32(1) << 14;
        const uint32_t  magicMask = to_u32(1) << 13;
        const uint32_t frameIndex = gfxIDOpt.value() + absFrame() % frameCount;

        const auto fnDrawTexture = [](uint32_t texIndex, bool alpha = false) -> bool
        {
            constexpr int drawX = 430;
            constexpr int drawY = 300;

            if(const auto [texPtr, dx, dy] = g_selectCharDB->retrieve(texIndex); texPtr){
                SDLDeviceHelper::EnableTextureModColor enableModColor(texPtr, colorf::WHITE + colorf::A_SHF(alpha ? 150 : 255));
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
}

void ProcessSelectChar::switchCharGfx()
{
    // switch the char animation to show all gfx
    // since for mir2x I only allow 1 char per account, so there is no select/deselect

    if(!hasChar()){
        return;
    }

    const auto frameCount = charFrameCount();
    if(frameCount <= 0){
        return;
    }

    const auto absFrameIndex = absFrame();
    if(absFrameIndex % frameCount){
        return;
    }

    // can switch now
    // but if current frame already did, we skip

    if(m_charAniSwitchFrame == absFrameIndex){
        return;
    }

    m_charAniSwitchFrame = absFrameIndex;
    switch(m_charAni){
        case 0:
            {
                if(mathf::rand<int>(0, 1) == 0){
                    m_charAni = 1;
                }
                break;
            }
        case 1:
            {
                m_charAni = 2;
                break;
            }
        case 2:
            {
                if(mathf::rand<int>(0, 1) == 0){
                    m_charAni = 3;
                }
                break;
            }
        case 3:
            {
                m_charAni = 0;
                break;
            }
        default:
            {
                throw fflreach();
            }
    }

    if(m_charAni == 1){
        const int offGender = to_d(m_smChar.value().gender);
        const int offJob = jobf::jobGfxIndex(m_smChar.value().job).front().value();

        const uint32_t seffID = UINT32_C(0X00010000) // base
            | (to_u32(offGender) << 4)               //
            | (to_u32(offJob   ) << 8)               //
            | (to_u32(1        ) << 0);              // 0 for create, 1 for select
        g_sdlDevice->playSoundEffect(g_seffDB->retrieve(seffID));
    }
}

void ProcessSelectChar::updateGUIActive()
{
    m_start .setActive( hasChar());
    m_create.setActive(!hasChar());
    m_delete.setActive( hasChar());
}
