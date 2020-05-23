/*
 * =====================================================================================
 *
 *       Filename: texaniboard.cpp
 *        Created: 07/20/2017 00:34:13
 *    Description: 
 *
 *        Version: 1.0
 *       Revision: none
 *       Compiler: gcc
 *
 *         Author: ANHONG
 *          Email: anhonghe@gmail.com
 *
 * =====================================================================================
 */

#include "pngtexdb.hpp"
#include "sdldevice.hpp"
#include "texanimation.hpp"

extern PNGTexDB *g_progUseDB;
extern SDLDevice *g_SDLDevice;

TexAniBoard::TexAniBoard(int x, int y, uint32_t texID, size_t frameCount, size_t fps, bool fadeInout, bool loop, Widget *pwidget, bool autoDelete)
    : Widget(x, y, 0, 0, pwidget, autoDelete)
    : m_fps(fps)
    , m_accuTime(0.0)
    , m_loop(loop)
    , m_fadeInout(fadeInout)
{
    m_texSeq.resize(frameCount);
    std::iota(m_texSeq.begin(), m_texSeq.end(), texID);
}

void TexAniBoard::update(double fUpdateTime)
{
    m_accuTime += fUpdateTime;
}

void TexAniBoard::drawEx(int dstX, int dstY, int, int, int, int)
{
    const double frameTime = 1000.0 / m_fps;
    const auto frame = [this]() -> int
    {
        const auto frame = std::lround(std::floor(m_accuTime / frameTime));
        if(m_loop){
            return frame;
        }
        return std::min<int>(frame, m_texSeq.size() - 1);
    }();

    const auto alpha = [this]() -> int
    {
        if(m_fadeInout){
            return (int)(std::lround(255 * std::max<double>(0.0, m_accuTime / frameTime - frame)));
        }
        return 0;
    }();

    const uint32_t currTexId = m_texSeq[(frame + 0) % m_texSeq.size()];
    const uint32_t nextTexId = m_texSeq[(frame + 1) % m_texSeq.size()];

    auto currTexPtr = g_progUseDB->Retrieve(currTexId);
    auto nextTexPtr = g_progUseDB->Retrieve(nextTexId);

    SDL_SetTextureAlphaMod(currTexPtr,       alpha);
    SDL_SetTextureAlphaMod(nextTexPtr, 255 - alpha);

    g_SDLDevice->DrawTexture(currTexPtr, dstX, dstY);
    g_SDLDevice->DrawTexture(nextTexPtr, dstX, dstY);
}
