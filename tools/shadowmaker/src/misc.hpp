#pragma once
#include <cstdint>
// #include <SDL.h>


uint32_t    *TwoPointShadowDecode(uint32_t *, int, int, int &, int &, int, int, int, int, uint32_t);
uint32_t    *ShadowDecode(bool, uint32_t *, int, int, int &, int &, uint32_t);
bool         MyCopyFile(const char *, const char *);
bool         SaveRGBABufferToPNG(const uint8_t *, uint32_t, uint32_t, const char *);
bool         RemoveDir(const char *);
bool         RemoveFile(const char *);
bool         MakeDir(const char *);
bool         FileExist(const char *);
bool         PointInSegment(int, int, int);
bool         PointInTriangle(double, double, double, double, double, double, double, double);
bool         PointInRect(int, int, int, int, int, int);
// SDL_Texture *LoadSDLTextureFromFile(const char *, SDL_Renderer *);
// char         SDLKeyEventCharName(SDL_Event &);
