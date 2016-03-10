/*
 * =====================================================================================
 *
 *       Filename: sdldevice.cpp
 *        Created: 03/07/2016 23:57:04
 *  Last Modified: 03/10/2016 01:09:56
 *
 *    Description: copy from flare-engine:
 *		   SDLDevice.h/cpp
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

SDLDevice::SDLDevice(XMLExt &stXMLExt,
        const std::function<void(const char *, varglistXXXX))
    : m_Renderer(nullptr)
    , m_Window(nullptr)
{
}

SDLDevice::~SDLDevice() {
}

void SDLDevice::SetWindowIcon()
{
    Uint16 pRawData[16 * 16] = {
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0AAB, 0X0789, 0X0BCC, 0X0EEE, 0X09AA, 0X099A, 0X0DDD,
        0X0FFF, 0X0EEE, 0X0899, 0X0FFF, 0X0FFF, 0X1FFF, 0X0DDE, 0X0DEE,
        0X0FFF, 0XABBC, 0XF779, 0X8CDD, 0X3FFF, 0X9BBC, 0XAAAB, 0X6FFF,
        0X0FFF, 0X3FFF, 0XBAAB, 0X0FFF, 0X0FFF, 0X6689, 0X6FFF, 0X0DEE,
        0XE678, 0XF134, 0X8ABB, 0XF235, 0XF678, 0XF013, 0XF568, 0XF001,
        0XD889, 0X7ABC, 0XF001, 0X0FFF, 0X0FFF, 0X0BCC, 0X9124, 0X5FFF,
        0XF124, 0XF356, 0X3EEE, 0X0FFF, 0X7BBC, 0XF124, 0X0789, 0X2FFF,
        0XF002, 0XD789, 0XF024, 0X0FFF, 0X0FFF, 0X0002, 0X0134, 0XD79A,
        0X1FFF, 0XF023, 0XF000, 0XF124, 0XC99A, 0XF024, 0X0567, 0X0FFF,
        0XF002, 0XE678, 0XF013, 0X0FFF, 0X0DDD, 0X0FFF, 0X0FFF, 0XB689,
        0X8ABB, 0X0FFF, 0X0FFF, 0XF001, 0XF235, 0XF013, 0X0FFF, 0XD789,
        0XF002, 0X9899, 0XF001, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0XE789,
        0XF023, 0XF000, 0XF001, 0XE456, 0X8BCC, 0XF013, 0XF002, 0XF012,
        0X1767, 0X5AAA, 0XF013, 0XF001, 0XF000, 0X0FFF, 0X7FFF, 0XF124,
        0X0FFF, 0X089A, 0X0578, 0X0FFF, 0X089A, 0X0013, 0X0245, 0X0EFF,
        0X0223, 0X0DDE, 0X0135, 0X0789, 0X0DDD, 0XBBBC, 0XF346, 0X0467,
        0X0FFF, 0X4EEE, 0X3DDD, 0X0EDD, 0X0DEE, 0X0FFF, 0X0FFF, 0X0DEE,
        0X0DEF, 0X08AB, 0X0FFF, 0X7FFF, 0XFABC, 0XF356, 0X0457, 0X0467,
        0X0FFF, 0X0BCD, 0X4BDE, 0X9BCC, 0X8DEE, 0X8EFF, 0X8FFF, 0X9FFF,
        0XADEE, 0XECCD, 0XF689, 0XC357, 0X2356, 0X0356, 0X0467, 0X0467,
        0X0FFF, 0X0CCD, 0X0BDD, 0X0CDD, 0X0AAA, 0X2234, 0X4135, 0X4346,
        0X5356, 0X2246, 0X0346, 0X0356, 0X0467, 0X0356, 0X0467, 0X0467,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF,
        0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF, 0X0FFF};

    SDL_Surface *pstSurface = SDL_CreateRGBSurfaceFrom(pRawData, 16, 16, 16, 16*2, 0x0f00, 0x00f0, 0x000f, 0xf000);

    // The icon is attached to the window pointer
    SDL_SetWindowIcon(window, pstSurface);
    SDL_FreeSurface(pstSurface);
}

int SDLDevice::CreateContext()
{
    bool settings_changed = (fullscreen != FULLSCREEN || hwsurface != HWSURFACE || vsync != VSYNC || texture_filter != TEXTURE_FILTER);

    Uint32 w_flags = 0;
    Uint32 r_flags = 0;
    int window_w = SCREEN_W;
    int window_h = SCREEN_H;

    if (FULLSCREEN) {
        w_flags = w_flags | SDL_WINDOW_FULLSCREEN_DESKTOP;

        // make the window the same size as the desktop resolution
        SDL_DisplayMode desktop;
        if (SDL_GetDesktopDisplayMode(0, &desktop) == 0) {
            window_w = desktop.w;
            window_h = desktop.h;
        }
    }
    else if (fullscreen && is_initialized) {
        // if the game was previously in fullscreen, resize the window when returning to windowed mode
        window_w = MIN_SCREEN_W;
        window_h = MIN_SCREEN_H;
        w_flags = w_flags | SDL_WINDOW_SHOWN;
    }
    else {
        w_flags = w_flags | SDL_WINDOW_SHOWN;
    }

    w_flags = w_flags | SDL_WINDOW_RESIZABLE;

    if (HWSURFACE) {
        r_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE;
    }
    else {
        r_flags = SDL_RENDERER_SOFTWARE | SDL_RENDERER_TARGETTEXTURE;
        VSYNC = false; // can't have software mode & vsync at the same time
    }
    if (VSYNC) r_flags = r_flags | SDL_RENDERER_PRESENTVSYNC;

    if (settings_changed || !is_initialized) {
        destroyContext();

        window = SDL_CreateWindow(NULL, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, w_flags);
        if (window) {
            m_Renderer = SDL_CreateRenderer(window, -1, r_flags);
            if (m_Renderer) {
                if (TEXTURE_FILTER && !IGNORE_TEXTURE_FILTER)
                    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
                else
                    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

                windowResize();
            }

            SDL_SetWindowMinimumSize(window, MIN_SCREEN_W, MIN_SCREEN_H);
            // setting minimum size might move the window, so set position again
            SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }

        bool window_created = window != NULL && m_Renderer != NULL;

        if (!window_created) {
            if (allow_fallback) {
                // try previous setting first
                FULLSCREEN = fullscreen;
                HWSURFACE = hwsurface;
                VSYNC = vsync;
                TEXTURE_FILTER = texture_filter;
                if (createContext(false) == -1) {
                    // last resort, try turning everything off
                    FULLSCREEN = false;
                    HWSURFACE = false;
                    VSYNC = false;
                    TEXTURE_FILTER = false;
                    int last_resort = createContext(false);
                    if (last_resort == -1 && !is_initialized) {
                        // If this is the first attempt and it failed we are not
                        // getting anywhere.
                        logError("SDLDevice: createContext() failed: %s", SDL_GetError());
                        Exit(1);
                    }
                    return last_resort;
                }
                else {
                    return 0;
                }
            }
        }
        else {
            fullscreen = FULLSCREEN;
            hwsurface = HWSURFACE;
            vsync = VSYNC;
            texture_filter = TEXTURE_FILTER;
            is_initialized = true;
        }
    }

    if (is_initialized) {
        // update minimum window size if it has changed
        if (min_screen.x != MIN_SCREEN_W || min_screen.y != MIN_SCREEN_H) {
            min_screen.x = MIN_SCREEN_W;
            min_screen.y = MIN_SCREEN_H;
            SDL_SetWindowMinimumSize(window, MIN_SCREEN_W, MIN_SCREEN_H);
            SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
        }

        windowResize();

        // update title bar text and icon
        updateTitleBar();

        // load persistent resources
        SharedResources::loadIcons();
        delete curs;
        curs = new CursorManager();
    }

    return (is_initialized ? 0 : -1);
}

SDL_Texture *SDLDevice::CreateTexture(const void *pMem, int nSize)
{
    // TODO
    //
    // currently it doesn't support dynamic set of context
    // because all textures are based on current m_Renderer
    //
    // if it's changed
    // all the texture need to be re-load
    //
    if(pMem == nullptr || nSize <= 0){ return nullptr; }

    SDL_RWops   *pstRWops   = nullptr;
    SDL_Surface *pstSurface = nullptr;
    SDL_Texture *pstTexture = nullptr;

    pstRWops = SDL_RWFromConstMem(pMem);
    if(pstRWops){
        pstSurface = SDL_LoadPNG_RW(pstRWops);
        if(pstSurface){
            pstTexture = SDL_CreateTextureFromSurface(m_Renderer, pstSurface);
        }
    }

    if(pstRWops){
        SDL_FreeRW(pstRWops);
    }

    if(pstSurface){
        SDL_FreeSurface(pstSurface);
    }

    return pstTexture;
}

void SDLDevice::DrawTexture(SDL_Texture *pstTexture, int nX, int nY)
{
    if(pstTexture){

        SDL_Rect stSrc;
        SDL_Rect stDst;

        int nW, nH;

        if(!SDL_QueryTexture(pstTexture, nullptr, nullptr, &nW, &nH)){

            // TODO
            // not sure whether need to adjust for boundary
            // hope not!
            stSrc = { 0,  0, nW, nH};
            stDst = {nX, nY, nW, nH};

            SDL_RenderCopy(m_Renderer, pstTexture, &stSrc, &stDst);
        }
    }
}
