/*
 * =====================================================================================
 *
 *       Filename: switchprocess.cpp
 *        Created: 01/23/2016 04:18:45
 *  Last Modified: 01/23/2016 04:28:31
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

void Game::SwitchProcess(int nNewID)
{
    SwitchProcess(m_ProcessID, nNewID);
}

void Game::SwitchProcess(int nOldID, int nNewID)
{
    std::assert(nOldID != nNewID);
    m_ProcessID = nNewID;

    switch(nOldID)
    {
        case PROCESSID_NULL:
            {
                switch(nNewID)
                {
                    case PROCESSID_LOGO:
                        {
                            // on initialization
                            m_TexLogo = m_GUITextureManager.Retrieve(0);
                            SDL_SetTextureBlendMode(m_TextureLogo, SDL_BLENDMODE_MOD);
                            break;
                        }
                    case PROCESSID_EXIT:
                        {
                            // exit immediately when logo shows
                            break;
                        }
                    default:
                        {
                            break;
                        }


                }
            }

    }
}
