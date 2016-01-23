/*
 * =====================================================================================
 *
 *       Filename: update.cpp
 *        Created: 8/12/2015 9:59:15 PM
 *  Last Modified: 01/14/2016 22:09:51
 *
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

Uint32 Game::StateTickCount()
{
    return SDL_GetTicks() - m_StateStart;
}

void Game::UpdateOnLogo()
{
    Uint32 nCount  = StateTickCount();
    Uint32 nDCount = (nCount < m_StateLogoFullTicks / 2) ? nCount : (nCount - m_StateLogoFullTicks / 2);
    Uint8  bColor  = (Uint8)(255.0 * nDCount * 2.0 / m_StateLogoFullTicks);

    SDL_SetRenderDrawColor(GetDeviceManager()->GetRenderer(), bColor, bColor, bColor, bColor);

    if(nCount >= m_StateLogoFullTicks){
        SwitchProcess(PROCESSID_LOGO, PROCESSID_SYRC);
    }
}

void Game::UpdateOnSyrc()
{
    // do nothing
    int nFinished = GetFinishedOnSyrc();
}

void Game::UpdateOnLogin()
{
    m_IDBox.Update();
    m_PasswordBox.Update();
}

void Game::UpdateOnRun()
{
}
