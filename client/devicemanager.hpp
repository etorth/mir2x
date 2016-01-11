/*
 * =====================================================================================
 *
 *       Filename: devicemanager.hpp
 *        Created: 6/17/2015 6:00:50 PM
 *  Last Modified: 08/31/2015 5:13:02 AM
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


#pragma once
#include <SDL.h>

class DeviceManager
{
    private:
        int         m_RendererFlag;

    private:
        SDL_Window      *m_Window;
        SDL_Renderer    *m_Renderer;

	private:
		Uint32			 m_WindowFlag;

    private:
        DeviceManager() = default;

    public:
        bool            Init();
        void            Release();

    public:
        SDL_Renderer   *GetRenderer();
        void            Clear();
        void            Present();
        void            SetRenderDrawColor(Uint8, Uint8, Uint8, Uint8);

    public:
        int             WindowSizeW();
        int             WindowSizeH();

    public:
        friend DeviceManager *GetDeviceManager();
};
