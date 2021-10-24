#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processselectchar.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ProcessSelectChar::ProcessSelectChar()
    : Process()
	, m_start (DIR_UPLEFT, 270,  60, {0X0C000010, 0X0C000010, 0X0C000011}, nullptr, nullptr, [this](){ onStart (); })
	, m_create(DIR_UPLEFT, 455, 103, {0X0C000020, 0X0C000020, 0X0C000021}, nullptr, nullptr, [this](){ onCreate(); })
	, m_delete(DIR_UPLEFT,  85, 256, {0X0C000030, 0X0C000030, 0X0C000031}, nullptr, nullptr, [this](){ onDelete(); })
	, m_exit  (DIR_UPLEFT,  50, 428, {0X0C000040, 0X0C000040, 0X0C000041}, nullptr, nullptr, [this](){ onExit  (); })

    , m_notifyBoard
      {
          DIR_UPLEFT,
          0,
          0,
          10240,
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
    m_notifyBoard.update(fUpdateTime);
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
        //
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
