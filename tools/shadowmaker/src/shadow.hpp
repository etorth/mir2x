#pragma once
#include <cstdint>
// #include <SDL.h>


uint32_t    *TwoPointShadowDecode(uint32_t *, int, int, int &, int &, int, int, int, int, uint32_t);
uint32_t    *ShadowDecode(bool, uint32_t *, int, int, int &, int &, uint32_t);
