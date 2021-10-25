#include "log.hpp"
#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "layoutboard.hpp"
#include "selectchardb.hpp"
#include "processselectchar.hpp"

extern Log *g_log;
extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern SelectCharDB *g_selectCharDB;

ProcessSelectChar::ProcessSelectChar()
    : Process()
	, m_start (DIR_UPLEFT, 335,  75, {0X0C000030, 0X0C000030, 0X0C000031}, nullptr, nullptr, [this](){ onStart (); })
	, m_create(DIR_UPLEFT, 565, 130, {0X0C000010, 0X0C000010, 0X0C000011}, nullptr, nullptr, [this](){ onCreate(); })
	, m_delete(DIR_UPLEFT, 110, 305, {0X0C000020, 0X0C000020, 0X0C000021}, nullptr, nullptr, [this](){ onDelete(); })
	, m_exit  (DIR_UPLEFT,  45, 544, {0X0C000040, 0X0C000040, 0X0C000041}, nullptr, nullptr, [this](){ onExit  (); })

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
          10,
      }

    , m_deleteInput
      {
          0,
          0,
      }
{
    m_start .active(false);
    m_create.active(false);
    m_delete.active(false);
    m_notifyBoard.addLog(u8"正在下载游戏角色");
    g_client->send(CM_QUERYCHAR);
}

void ProcessSelectChar::update(double fUpdateTime)
{
    m_charAniTime += fUpdateTime;
    m_notifyBoard.update(fUpdateTime);

    if(hasChar()){
        switchCharGfx();
    }
}

void ProcessSelectChar::draw()
{
    SDLDeviceHelper::RenderNewFrame newFrame;
    if(auto texPtr = g_progUseDB->retrieve(0X0C000000)){
        g_sdlDevice->drawTexture(texPtr, 0, 0);
    }

    m_start .draw();
    m_create.draw();
    m_delete.draw();
    m_exit  .draw();

    if(m_smChar.has_value() && !m_smChar.value().name.empty()){
        drawChar();
        drawCharName();
    }
    else{
        m_notifyBoard.drawAt(DIR_NONE, 400, 300);
    }
}

void ProcessSelectChar::processEvent(const SDL_Event &event)
{
    bool tookEvent = false;
    tookEvent |= m_start .processEvent(event, !tookEvent);
    tookEvent |= m_create.processEvent(event, !tookEvent);
    tookEvent |= m_delete.processEvent(event, !tookEvent);
    tookEvent |= m_exit  .processEvent(event, !tookEvent);

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
    if(m_smChar.has_value() && !m_smChar.value().name.empty()){
        m_deleteInput.show(true);
        m_deleteInput.waitInput(u8"删除的角色将无法还原，请谨慎操作。如果确定删除，请输入游戏密码，并点击YES。", [this](std::u8string inputString)
        {
            CMDeleteChar cmDC;
            std::memset(&cmDC, 0, sizeof(cmDC));
            if(inputString.empty() || inputString.size() > SYS_PWDSIZE){
                m_notifyBoard.addLog(u8"无效的密码");
            }
            else{
                cmDC.password.assign(inputString);
                g_client->send(CM_DELETECHAR, cmDC);
            }
            m_deleteInput.show(false);
        });
    }
}

void ProcessSelectChar::onExit()
{
    g_client->requestProcess(PROCESSID_LOGIN);
}

void ProcessSelectChar::drawCharName() const
{
    if(m_smChar.has_value() && !m_smChar.value().name.empty()){
        const auto exp  = m_smChar.value().exp;
        const auto name = m_smChar.value().name.to_str();
        const auto job  = m_smChar.value().job.deserialize<std::vector<int>>().at(0);

        LayoutBoard charBoard
        {
            DIR_UPLEFT,
            0,
            0,
            200,

            false,
            {0, 0, 0, 0},

            false,

            1,
            15,
            0,
            colorf::WHITE + colorf::A_SHF(255),
            0,
        };

        std::u8string xmlStr;
        xmlStr += str_printf(u8R"###( <layout> )###""\n");
        xmlStr += str_printf(u8R"###(     <par color='RGB(237,226,200)'>角色：%s</par> )###""\n", to_cstr(name));
        xmlStr += str_printf(u8R"###(     <par color='RGB(175,196,175)'>等级：%d</par> )###""\n", to_d(SYS_LEVEL(exp)));
        xmlStr += str_printf(u8R"###(     <par color='RGB(231,231,189)'>职业：%s</par> )###""\n", to_cstr(jobName(job)));
        xmlStr += str_printf(u8R"###( </layout> )###""\n");
        charBoard.loadXML(to_cstr(xmlStr));

        constexpr int drawBoardX = 120;
        constexpr int drawBoardY = 200;
        constexpr int drawBoardMargin = 15;

        g_sdlDevice->fillRectangle(colorf::RGBA(0, 0,   0, 128), drawBoardX - drawBoardMargin, drawBoardY - drawBoardMargin, charBoard.w() + drawBoardMargin * 2, charBoard.h() + drawBoardMargin * 2, 5);
        g_sdlDevice->drawRectangle(colorf::RGBA(0, 0, 255, 128), drawBoardX - drawBoardMargin, drawBoardY - drawBoardMargin, charBoard.w() + drawBoardMargin * 2, charBoard.h() + drawBoardMargin * 2, 5);
        charBoard.drawAt(DIR_UPLEFT, drawBoardX, drawBoardY);
    }
}

uint32_t ProcessSelectChar::charFrameCount() const
{
    if(m_smChar.has_value() && !m_smChar.value().name.empty()){
        const bool gender = m_smChar.value().gender;
        const int job = m_smChar.value().job.deserialize<std::vector<int>>().at(0);

        fflassert(job >= JOB_BEGIN);
        fflassert(job <  JOB_END);

        fflassert(m_charAni >= 0);
        fflassert(m_charAni <  4);

        constexpr bool   male =  true;
        constexpr bool female = false;

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

        if(auto p = s_frameCount.find({job, gender, m_charAni}); p != s_frameCount.end()){
            return p->second;
        }
    }
    return 0;
}

std::optional<uint32_t> ProcessSelectChar::charGfxBaseID() const
{
    if(m_smChar.has_value() && !m_smChar.value().name.empty()){
        const bool gender = m_smChar.value().gender;
        const int job = m_smChar.value().job.deserialize<std::vector<int>>().at(0);

        fflassert(job >= JOB_BEGIN);
        fflassert(job <  JOB_END);

        fflassert(m_charAni >= 0);
        fflassert(m_charAni <  4);

        // 14     : max =  2   shadow
        // 13     : max =  2   magic
        // 10 - 12: max =  8   job
        // 09     : max =  2   gender: male as true
        // 05 - 08: max = 16   motion
        // 00 - 04: max = 32   frame

        return 0
            + (to_u32(job - JOB_BEGIN) << 10)
            + (to_u32(gender         ) <<  9)
            + (to_u32(m_charAni      ) <<  5);
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

        const uint32_t shadowMask = to_u32(14) << 20;
        const uint32_t  magicMask = to_u32(13) << 20;
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
            fnDrawTexture(frameIndex |  magicMask);
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
                throw bad_reach();
            }
    }
}
