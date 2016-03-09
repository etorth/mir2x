/*
 * =====================================================================================
 *
 *       Filename: sdldevice.cpp
 *        Created: 03/07/2016 23:57:04
 *  Last Modified: 03/09/2016 00:49:37
 *
 *    Description: copy from flare-engine:
 *		   SDLHardwareRenderDevice.h/cpp
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

#include "sdlharedwaredevice.hpp"

SDLDevice::SDLDevice(XMLExt &stXMLExt)
    : m_Window(nullptr)
    , m_Renderer(nullptr)
    , texture(nullptr)
    , titlebar_icon(nullptr)
    , title(nullptr)
{
    fullscreen = FULLSCREEN;
    hwsurface = HWSURFACE;
    vsync = VSYNC;
    texture_filter = TEXTURE_FILTER;

    min_screen.x = MIN_SCREEN_W;
    min_screen.y = MIN_SCREEN_H;
}

void SDLDevice::CreateContext(const XMLExt &stXMLExt)
{
    CreateWindow(stXMLExt);
}

void SDLDevice::CreateWindow(const XMLExt &stXMLExt)
{
    m_WindowW = 0;
    m_WindowH = 0;

    try{
	if(stXMLExt.FindNode("Root/Window/W")
		&& stXMLExt.FindNode("Root/Window/H")){
	    m_WindowW = stXMLExt.NodeAtoi("Root/Window/W");
	    m_WindowH = stXMLExt.NodeAtoi("Root/Window/H");
	}else{
	    if(stXMLExt.FindNode("Root/Window/FullResolution")){
		SDL_DisplayMode desktop;
		if(!SDL_GetDesktopDisplayMode(0, &desktop)){
		    m_WindowW = desktop.w;
		    m_WindowH = desktop.h;
		}
	    }
	}
    }catch(std::exception &stExp){
	fnLog("%s\n", stExp.what());
    }

    if(!(m_WindowW && m_WindowH)){
	m_WindowW = 800;
	m_WindowH = 600;
    }

    try{
	m_WindowFullScreen = stXMLExt.NodeAtob("Root/Window/FullScreen");
    }catch(...){
	m_WindowFullScreen = true;
	std::


    }

    if(stXMLExt.FindNode("Root/Window/FullScreen")){
    }




}

int SDLDevice::createContext(bool allow_fallback)
{
    const char *szWindowW = stXMLExt.Find("Root/Window/W");
    const char *szWindowH = stXMLExt.Find("Root/Window/H");

    if(szWindowW && szWindowH){
	m_WindowW = std::atoi(szWindowW);
	m_WindowH = std::atoi(szWindowH);
    }else{
	const char *szFullRes = stXMLExt.Find("Root/Window/FullResolution");
    }
    (stXMLExt)


	int nMaxW = 0;
    int nMaxH = 0;


    else{
	m_WindowW = 800;
	m_WindowH = 600;
    }
}

if(stXMLExt.FindNode("Root/Window") && )


bool settings_changed = (fullscreen != FULLSCREEN || hwsurface != HWSURFACE || vsync != VSYNC || texture_filter != TEXTURE_FILTER);

Uint32 w_flags = 0;
Uint32 r_flags = 0;
int window_w = SCREEN_W;
int window_h = SCREEN_H;

if (FULLSCREEN) {
    w_flags = w_flags | SDL_WINDOW_FULLSCREEN_DESKTOP;

    // make the m_Window the same size as the desktop resolution
}
else if (fullscreen && is_initialized) {
    // if the game was previously in fullscreen, resize the m_Window when returning to windowed mode
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

    m_Window = SDL_CreateWindow(nullptr, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_w, window_h, w_flags);
    if (m_Window) {
	m_Renderer = SDL_CreateRenderer(m_Window, -1, r_flags);
	if (m_Renderer) {
	    if (TEXTURE_FILTER && !IGNORE_TEXTURE_FILTER)
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1");
	    else
		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	    windowResize();
	}

	SDL_SetWindowMinimumSize(m_Window, MIN_SCREEN_W, MIN_SCREEN_H);
	// setting minimum size might move the m_Window, so set position again
	SDL_SetWindowPosition(m_Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    }

    bool window_created = m_Window != nullptr && m_Renderer != nullptr;

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
    // update minimum m_Window size if it has changed
    if (min_screen.x != MIN_SCREEN_W || min_screen.y != MIN_SCREEN_H) {
	min_screen.x = MIN_SCREEN_W;
	min_screen.y = MIN_SCREEN_H;
	SDL_SetWindowMinimumSize(m_Window, MIN_SCREEN_W, MIN_SCREEN_H);
	SDL_SetWindowPosition(m_Window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
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

int SDLDevice::render(Renderable& r, Rect& dest)
{
    dest.w = r.src.w;
    dest.h = r.src.h;
    SDL_Rect src = r.src;
    SDL_Rect _dest = dest;
    SDL_SetRenderTarget(m_Renderer, texture);
    return SDL_RenderCopy(m_Renderer, static_cast<SDLHardwareImage *>(r.image)->surface, &src, &_dest);
}

int SDLDevice::render(Sprite *r) {
    if (r == nullptr) {
	return -1;
    }
    if ( !localToGlobal(r) ) {
	return -1;
    }

    // negative x and y clip causes weird stretching
    // adjust for that here
    if (m_clip.x < 0) {
	m_clip.w -= abs(m_clip.x);
	m_dest.x += abs(m_clip.x);
	m_clip.x = 0;
    }
    if (m_clip.y < 0) {
	m_clip.h -= abs(m_clip.y);
	m_dest.y += abs(m_clip.y);
	m_clip.y = 0;
    }

    m_dest.w = m_clip.w;
    m_dest.h = m_clip.h;

    SDL_Rect src = m_clip;
    SDL_Rect dest = m_dest;
    SDL_SetRenderTarget(m_Renderer, texture);
    return SDL_RenderCopy(m_Renderer, static_cast<SDLHardwareImage *>(r->getGraphics())->surface, &src, &dest);
}

int SDLDevice::renderToImage(Image* src_image, Rect& src, Image* dest_image, Rect& dest) {
    if (!src_image || !dest_image)
	return -1;

    if (SDL_SetRenderTarget(m_Renderer, static_cast<SDLHardwareImage *>(dest_image)->surface) != 0)
	return -1;

    dest.w = src.w;
    dest.h = src.h;
    SDL_Rect _src = src;
    SDL_Rect _dest = dest;

    SDL_SetTextureBlendMode(static_cast<SDLHardwareImage *>(dest_image)->surface, SDL_BLENDMODE_BLEND);
    SDL_RenderCopy(m_Renderer, static_cast<SDLHardwareImage *>(src_image)->surface, &_src, &_dest);
    SDL_SetRenderTarget(m_Renderer, nullptr);
    return 0;
}

int SDLDevice::renderText(
	FontStyle *font_style,
	const std::string& text,
	const Color& color,
	Rect& dest
	)
{
    int ret = 0;
    SDL_Texture *surface = nullptr;

    SDL_Surface *cleanup = TTF_RenderUTF8_Blended(static_cast<SDLFontStyle *>(font_style)->ttfont, text.c_str(), color);
    if (cleanup) {
	surface = SDL_CreateTextureFromSurface(m_Renderer,cleanup);
	SDL_FreeSurface(cleanup);
    }

    if (surface == nullptr)
	return -1;

    SDL_Rect clip;
    int w, h;
    SDL_QueryTexture(surface, nullptr, nullptr, &w, &h);

    clip.x = clip.y = 0;
    clip.w = w;
    clip.h = h;

    dest.w = clip.w;
    dest.h = clip.h;
    SDL_Rect _dest = dest;

    SDL_SetRenderTarget(m_Renderer, texture);
    ret = SDL_RenderCopy(m_Renderer, surface, &clip, &_dest);

    SDL_DestroyTexture(surface);

    return ret;
}

Image * SDLDevice::renderTextToImage(FontStyle* font_style, const std::string& text, const Color& color, bool blended) {
    SDLHardwareImage *image = new SDLHardwareImage(this, m_Renderer);

    SDL_Surface *cleanup;

    if (blended) {
	cleanup = TTF_RenderUTF8_Blended(static_cast<SDLFontStyle *>(font_style)->ttfont, text.c_str(), color);
    }
    else {
	cleanup = TTF_RenderUTF8_Solid(static_cast<SDLFontStyle *>(font_style)->ttfont, text.c_str(), color);
    }

    if (cleanup) {
	image->surface = SDL_CreateTextureFromSurface(m_Renderer, cleanup);
	SDL_FreeSurface(cleanup);
	return image;
    }

    delete image;
    return nullptr;
}

void SDLDevice::DrawPixel(int nX, int nY, const Color& color)
{
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawPoint(m_Renderer, x, y);
}

void SDLDevice::drawLine(int x0, int y0, int x1, int y1, const Color& color) {
    SDL_SetRenderDrawColor(m_Renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawLine(m_Renderer, x0, y0, x1, y1);
}

void SDLDevice::drawRectangle(const Point& p0, const Point& p1, const Color& color) {
    drawLine(p0.x, p0.y, p1.x, p0.y, color);
    drawLine(p1.x, p0.y, p1.x, p1.y, color);
    drawLine(p0.x, p0.y, p0.x, p1.y, color);
    drawLine(p0.x, p1.y, p1.x, p1.y, color);
}

void SDLDevice::blankScreen() {
    SDL_SetRenderTarget(m_Renderer, texture);
    SDL_SetRenderDrawColor(m_Renderer, 0, 0, 0, 255);
    SDL_RenderClear(m_Renderer);
    return;
}

void SDLDevice::commitFrame() {
    SDL_SetRenderTarget(m_Renderer, nullptr);
    SDL_RenderCopy(m_Renderer, texture, nullptr, nullptr);
    SDL_RenderPresent(m_Renderer);
    inpt->window_resized = false;

    return;
}

void SDLDevice::destroyContext() {
    // we need to free all loaded graphics as they may be tied to the current context
    RenderDevice::cacheRemoveAll();
    reload_graphics = true;

    if (curs) {
	delete curs;
	curs = nullptr;
    }

    SDL_FreeSurface(titlebar_icon);
    titlebar_icon = nullptr;

    SDL_DestroyRenderer(m_Renderer);
    m_Renderer = nullptr;

    SDL_DestroyWindow(m_Window);
    m_Window = nullptr;

    SDL_DestroyTexture(texture);
    texture = nullptr;

    if (title) {
	free(title);
	title = nullptr;
    }

    return;
}

/**
 * create blank surface
 */
Image *SDLDevice::createImage(int width, int height) {

    SDLHardwareImage *image = new SDLHardwareImage(this, m_Renderer);

    if (width > 0 && height > 0) {
	image->surface = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, width, height);
	if(image->surface == nullptr) {
	    logError("SDLDevice: SDL_CreateTexture failed: %s", SDL_GetError());
	}
	else {
	    SDL_SetRenderTarget(m_Renderer, image->surface);
	    SDL_SetTextureBlendMode(image->surface, SDL_BLENDMODE_BLEND);
	    SDL_SetRenderDrawColor(m_Renderer, 0,0,0,0);
	    SDL_RenderClear(m_Renderer);
	    SDL_SetRenderTarget(m_Renderer, nullptr);
	}
    }

    return image;
}

void SDLDevice::setGamma(float g) {
    Uint16 ramp[256];
    SDL_CalculateGammaRamp(g, ramp);
    SDL_SetWindowGammaRamp(m_Window, ramp, ramp, ramp);
}

void SDLDevice::updateTitleBar() {
    if (title) free(title);
    title = nullptr;
    if (titlebar_icon) SDL_FreeSurface(titlebar_icon);
    titlebar_icon = nullptr;

    if (!m_Window) return;

    title = strdup(msg->get(WINDOW_TITLE).c_str());
    titlebar_icon = IMG_Load(mods->locate("images/logo/icon.png").c_str());

    if (title) SDL_SetWindowTitle(m_Window, title);
    if (titlebar_icon) SDL_SetWindowIcon(m_Window, titlebar_icon);
}

Image *SDLDevice::loadImage(const std::string&filename, const std::string& errormessage, bool IfNotFoundExit) {
    // lookup image in cache
    Image *img;
    img = cacheLookup(filename);
    if (img != nullptr) return img;

    // load image
    SDLHardwareImage *image = new SDLHardwareImage(this, m_Renderer);
    if (!image) return nullptr;

    image->surface = IMG_LoadTexture(m_Renderer, mods->locate(filename).c_str());

    if(image->surface == nullptr) {
	delete image;
	if (!errormessage.empty())
	    logError("SDLDevice: [%s] %s: %s", filename.c_str(), errormessage.c_str(), IMG_GetError());
	if (IfNotFoundExit) {
	    Exit(1);
	}
	return nullptr;
    }

    // store image to cache
    cacheStore(filename, image);
    return image;
}

void SDLDevice::windowResize() {
    int w,h;
    SDL_GetWindowSize(m_Window, &w, &h);
    SCREEN_W = static_cast<unsigned short>(w);
    SCREEN_H = static_cast<unsigned short>(h);

    float scale = static_cast<float>(VIEW_H) / static_cast<float>(SCREEN_H);
    VIEW_W = static_cast<unsigned short>(static_cast<float>(SCREEN_W) * scale);

    // letterbox if too tall
    if (VIEW_W < MIN_SCREEN_W) {
	VIEW_W = MIN_SCREEN_W;
    }

    VIEW_W_HALF = VIEW_W/2;

    SDL_RenderSetLogicalSize(m_Renderer, VIEW_W, VIEW_H);

    if (texture) SDL_DestroyTexture(texture);
    texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, VIEW_W, VIEW_H);
    SDL_SetRenderTarget(m_Renderer, texture);

    updateScreenVars();
}
