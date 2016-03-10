/*
 * =====================================================================================
 *
 *       Filename: renderdevice.hpp
 *        Created: 08/31/2015 10:45:48
 *  Last Modified: 03/09/2016 21:13:24
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
#include <SDL2/SDL.h>

class RenderDevice
{
	public:
		RenderDevice();
		virtual ~RenderDevice();

		/** Context operations */
		virtual int createContext(bool allow_fallback = true) = 0;
		virtual void destroyContext() = 0;
		virtual void setGamma(float g) = 0;
		virtual void updateTitleBar() = 0;

		/** factory functions for Image */
		virtual Image *loadImage(const std::string& filename,
				const std::string& errormessage = "Couldn't load image",
				bool IfNotFoundExit = false) = 0;
		virtual Image *createImage(int width, int height) = 0;
		virtual void freeImage(Image *image) = 0;

		/** Screen operations */
		virtual int render(Sprite* r) = 0;
		virtual int render(Renderable& r, Rect& dest) = 0;
		virtual int renderToImage(Image* src_image, Rect& src, Image* dest_image, Rect& dest) = 0;
		virtual int renderText(FontStyle *font_style, const std::string& text, const Color& color, Rect& dest) = 0;
		virtual Image* renderTextToImage(FontStyle* font_style, const std::string& text, const Color& color, bool blended = true) = 0;
		virtual void blankScreen() = 0;
		virtual void commitFrame() = 0;
		virtual void drawPixel(int x, int y, const Color& color) = 0;
		virtual void drawRectangle(const Point& p0, const Point& p1, const Color& color) = 0;
		virtual void windowResize() = 0;

		bool reloadGraphics();

	protected:
		/* Compute clipping and global position from local frame. */
		bool localToGlobal(Sprite *r);

		/* Image cache operations */
		Image *cacheLookup(const std::string &filename);
		void cacheStore(const std::string &filename, Image *);
		void cacheRemove(Image *image);
		void cacheRemoveAll();

		bool fullscreen;
		bool hwsurface;
		bool vsync;
		bool texture_filter;
		Point min_screen;

		bool is_initialized;
		bool reload_graphics;

		Rect m_clip;
		Rect m_dest;

	private:
		typedef std::map<std::string, Image *> IMAGE_CACHE_CONTAINER;
		typedef IMAGE_CACHE_CONTAINER::iterator IMAGE_CACHE_CONTAINER_ITER;

		IMAGE_CACHE_CONTAINER cache;

		virtual void drawLine(int x0, int y0, int x1, int y1, const Color& color) = 0;
};
