/*
 * =====================================================================================
 *
 *       Filename: draw.cpp
 *        Created: 01/14/2016 06:26:55
 *  Last Modified: 01/23/2016 03:55:59
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


void Game::DrawOnSyrc()
{
    SDL_Rect stRectSrc, stRectDst;
	int nW, nH;
	SDL_QueryTexture(GetGUITextureManager()->Retrieve(), nullptr, nullptr, &nW, &nH);

    stRectSrc.x = 0;
    stRectSrc.y = 0;
    stRectSrc.w = std::lround(nW * (GetFinishedOnSyrc() / 100.0));
	stRectSrc.h = nH;
    stRectDst.x = 112;
    stRectDst.y = 528;
    stRectDst.w = stRectSrc.w;
    stRectDst.h = stRectSrc.h;

    SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
            GetGUITextureManager()->Retrieve(TEXTUREID_PROGRESSBAR) &stRectSrc, &stRectDst);
    SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
            GetGUITextureManager()->Retrieve(TEXTUREID_BACKGROUND), nullptr, nullptr);

	m_Info.Draw();
}

void Game::DrawOnLogin()
{
    SDL_Rect stRectDst1, stRectDst2;
    stRectDst1.x = 0;
    stRectDst1.y = 75;
    stRectDst1.w = 800;
    stRectDst1.h = 450;

    stRectDst2.x = 0;
    stRectDst2.y = 465;
    stRectDst2.w = 800;
    stRectDst2.h = 60;

    SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
            GetGUITextureManager()->Retrieve(1), nullptr, &stRectDst1);
    SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
            GetGUITextureManager()->Retrieve(2), nullptr, &stRectDst1);

    SDL_Rect stRectDst3 = {103, 536, 456, 26};
    SDL_RenderCopy(GetDeviceManager()->GetRenderer(),
            GetGUITextureManager()->Retrieve(3), nullptr, &stRectDst3);

    m_Button1.Draw();
    m_Button2.Draw();
    m_Button3.Draw();
    m_Button4.Draw();

    m_IDBox.Draw();
    m_PasswordBox.Draw();
}
