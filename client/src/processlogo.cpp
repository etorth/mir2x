#include "client.hpp"
#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "processlogo.hpp"
#include "clientargparser.hpp"

extern Client *g_client;
extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_sdlDevice;
extern ClientArgParser *g_clientArgParser;

void ProcessLogo::processEvent(const SDL_Event &event)
{
    switch(event.type){
        case SDL_KEYDOWN:
            {
                switch(event.key.keysym.sym){
                    case SDLK_SPACE:
                    case SDLK_ESCAPE:
                        {
                            g_client->requestProcess(PROCESSID_SYRC);
                        }
                        break;
                    default:
                        break;
                }
                break;
            }
        default:
            break;
    }
}

void ProcessLogo::update(double fDTime)
{
    m_totalTime += fDTime;
    if(m_totalTime >= m_fullTime || g_clientArgParser->autoLogin){
        g_client->requestProcess(PROCESSID_SYRC);
    }
}

void ProcessLogo::draw() const
{
    SDLDeviceHelper::RenderNewFrame newFrame;
    if(auto texPtr = g_progUseDB->retrieve(0X00000000)){
        const auto c = to_u8(to_dround(255 * colorRatio()));
        const SDLDeviceHelper::EnableTextureModColor modColor(texPtr, colorf::RGBA(c, c, c, 0XFF));

        const auto winW = g_sdlDevice->getRendererWidth();
        const auto winH = g_sdlDevice->getRendererHeight();
        g_sdlDevice->drawTexture(texPtr, 0, 0, 0, 0, winW, winH);
    }
}

double ProcessLogo::colorRatio() const
{
    const double fRatio = m_totalTime / m_fullTime;
    if(fRatio < m_timeR1){
        return fRatio / m_timeR1;
    }

    else if(fRatio < m_timeR1 + m_timeR2){
        return 1.0;
    }

    else{
        return 1.0 - (fRatio - m_timeR1 - m_timeR2) / (1.0 - m_timeR1 - m_timeR2);
    }
}
