#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processcreatechar.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;

ProcessCreateChar::ProcessCreateChar()
    : Process()
	, m_warrior(DIR_UPLEFT, 270,  60, {0X0D000030, 0X0D000031, 0X0D000032}, nullptr, nullptr, [this](){ m_job = JOB_WARRIOR; })
	, m_wizard (DIR_UPLEFT, 455, 103, {0X0D000040, 0X0D000041, 0X0D000042}, nullptr, nullptr, [this](){ m_job = JOB_TAOIST ; })
	, m_taoist (DIR_UPLEFT,  85, 256, {0X0D000050, 0X0D000051, 0X0D000052}, nullptr, nullptr, [this](){ m_job = JOB_WIZARD ; })
	, m_submit (DIR_UPLEFT, 472, 436, {0X0D000010, 0X0D000011, 0X0D000012}, nullptr, nullptr, [this](){ onSubmit();          })
	, m_exit   (DIR_UPLEFT, 514, 436, {0X0D000020, 0X0D000021, 0X0D000022}, nullptr, nullptr, [this](){ onExit();            })

    , m_nameLine
      {
          DIR_UPLEFT,
          7,
          105,
          343,
          17,

          1,
          12,

          0,
          colorf::WHITE + colorf::A_SHF(255),

          2,
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
          10240,
          1,
          15,
          0,
          colorf::YELLOW + colorf::A_SHF(255),
          5000,
          10,
      }
{}

void ProcessCreateChar::update(double)
{}

void ProcessCreateChar::draw() const
{
    if(auto texPtr = g_progUseDB->retrieve(0X0C000000)){
        g_sdlDevice->drawTexture(texPtr, 0, 0);
    }

    m_warrior.draw();
    m_wizard .draw();
    m_taoist .draw();

    m_submit.draw();
    m_exit  .draw();
}

void ProcessCreateChar::processEvent(const SDL_Event &)
{
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
