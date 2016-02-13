#pragma once
#include <cstdint>
#include <SDL.h>

bool         SaveRGBABufferToPNG(const uint8_t *, uint32_t, uint32_t, const char *);
bool         RemoveDir(const char *);
bool         MakeDir(const char *);
bool         FileExist(const char *);
bool         PointInSegment(int, int, int);
bool         PointInTriangle(double, double, double, double, double, double, double, double);
bool         PointInRect(int, int, int, int, int, int);
SDL_Texture *LoadSDLTextureFromFile(const char *, SDL_Renderer *);
char         SDLKeyEventCharName(SDL_Event &);
