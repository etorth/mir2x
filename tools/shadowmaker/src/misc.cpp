/*
 * =====================================================================================
 *
 *       Filename: misc.cpp
 *        Created: 7/24/2015 7:20:18 PM
 *  Last Modified: 09/07/2015 7:40:53 PM
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

#define _CRT_SECURE_NO_DEPRECATE
#include <png.h>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <dirent.h>
#include <string>
// #include <SDL.h>
// #include <SDL_image.h>
#include <unordered_map>
#include <string>
#include <algorithm>

#ifdef _WIN32
#include <direct.h>
#include <io.h>
#endif

bool SaveRGBABufferToPNG(const uint8_t *rgbaBuff, uint32_t nW, uint32_t nH, const char *fileFullName)
{
    // if(nW == 0 || nH == 0){
    //     printf("invalid: %s\n", fileFullName);
    // }
    png_structp   png_ptr      = nullptr;
    png_infop     info_ptr     = nullptr;
    int           nPixelSize   = 4;
    png_byte    **row_pointers = nullptr;
    FILE         *fp           = nullptr;
    bool          status       = false;

    if((fp = fopen(fileFullName, "wb")) == nullptr){
        goto SaveRGBABufferToPNG_fopen_failed;
    }

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if(png_ptr == nullptr){
        goto SaveRGBABufferToPNG_png_create_write_struct_failed;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if(info_ptr == nullptr){
        goto SaveRGBABufferToPNG_png_create_info_struct_failed;
    }

    /* Set up error handling. */
    if(setjmp(png_jmpbuf(png_ptr))){
        goto SaveRGBABufferToPNG_png_failure;
    }

    /* Set image attributes. */
    png_set_IHDR(png_ptr, info_ptr,
            nW, nH, 8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);

    /* Initialize rows of PNG. */
    row_pointers =(png_byte **)png_malloc(png_ptr, nH * sizeof(png_byte *));
    for(int y = 0; (uint32_t)y < nH; ++y){
        png_byte *row =(png_byte *)png_malloc(png_ptr, sizeof(uint8_t) * nW * nPixelSize);
        std::memcpy(row, rgbaBuff + nW * y * nPixelSize * sizeof(uint8_t), nW * nPixelSize * sizeof(uint8_t));
        row_pointers[y] = row;
    }

    /* Write the image data to "fp". */

    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

    status = true;

    for(int y = 0; (uint32_t)y < nH; y++){
        png_free(png_ptr, row_pointers[y]);
    }

    png_free(png_ptr, row_pointers);

SaveRGBABufferToPNG_png_failure:
SaveRGBABufferToPNG_png_create_info_struct_failed:
    png_destroy_write_struct(&png_ptr, &info_ptr);
SaveRGBABufferToPNG_png_create_write_struct_failed:
    fclose(fp);
SaveRGBABufferToPNG_fopen_failed:
    return status;
}

bool RemoveDir(const char *szAbsolutePath)
{
    std::string szPathName = szAbsolutePath;
    if(szPathName.back() != '/'){
        szPathName += "/";
    }

    auto stDir = opendir(szPathName.c_str());
    if(stDir == nullptr){
        return false;
    }

    struct dirent *stDirItem = nullptr;
    while(stDirItem = readdir(stDir)){
        if(!std::strcmp(stDirItem->d_name, ".")){
            continue;
        }
        if(!std::strcmp(stDirItem->d_name, "..")){
            continue;
        }

        if(stDirItem->d_type == DT_DIR){
            if(!RemoveDir((szPathName + stDirItem->d_name).c_str())){
                return false;
            }else{
                rewinddir(stDir);
                continue;
            }
        }

        // not std::remove(), they are different functions
        if(remove((szPathName + stDirItem->d_name).c_str()) != 0){
            return false;
		}else{
			rewinddir(stDir);
			continue;
		}
    }
    return !closedir(stDir) &&
#ifdef _WIN32
        !_rmdir
#else
        !rmdir
#endif
        (szPathName.c_str());

}

bool MakeDir(const char *szDirName)
{
    return 
#ifdef _WIN32
        !_mkdir
#else
        !mkdir
#endif
        (szDirName);
}

bool FileExist(const char *szFileName)
{
    return 
#ifdef _WIN32
        !_access(szFileName, 0)
#else
        !access(szFileName, F_OK)
#endif
        ;
}

bool PointInSegment(int nX, int nStartX, int nW)
{
    return nX >= nStartX && nX < nStartX + nW;
}

bool PointInRect(int nX, int nY, int nStartX, int nStartY, int nW, int nH)
{
    return PointInSegment(nX, nStartX, nW) && PointInSegment(nY, nStartY, nH);
}

bool PointInTriangle(double fX, double fY,
        double fX1, double fY1, double fX2, double fY2, double fX3, double fY3)
{
    auto bSign = [](double p1X, double p1Y, double p2X, double p2Y, double p3X, double p3Y){
        return (p1X - p3X) * (p2Y - p3Y) - (p2X - p3X) * (p1Y - p3Y);
    };

    bool b1 = bSign(fX, fY, fX1, fY1, fX2, fY2) < 0.0f;
    bool b2 = bSign(fX, fY, fX2, fY2, fX3, fY3) < 0.0f;
    bool b3 = bSign(fX, fY, fX3, fY3, fX1, fY1) < 0.0f;

    return ((b1 == b2) && (b2 == b3));
}

// SDL_Texture *LoadSDLTextureFromFile(const char * szFileFullName, SDL_Renderer * pRenderer)
// {
//     SDL_Surface *pSurface = IMG_Load(szFileFullName);
//     SDL_Texture *pTexture = nullptr;
//     if(pSurface && pRenderer){
//         pTexture = SDL_CreateTextureFromSurface(pRenderer, pSurface);
//         SDL_FreeSurface(pSurface);
//     }
//     return pTexture;
// }
//
// char SDLKeyEventCharName(SDL_Event &stEvent)
// {
//     // to optimize it, make a table here
//     // if(stEvent.key.keysym.sym >= SDLK_0 && stEvent.key.keysym.sym <= SDLK_9){
//     //     return '0' + stEvent.key.keysym.sym - SDLK_0;
//     // }
//
//     // if(stEvent.key.keysym.sym >= SDLK_a && stEvent.key.keysym.sym <= SDLK_z){
//     //     return 'a' + stEvent.key.keysym.sym - SDLK_a;
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_SPACE){
//     //     return ' ';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_AMPERSAND){
//     //     return '&';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_ASTERISK){
//     //     return '*';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_AT){
//     //     return '@';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_CARET){
//     //     return '^';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_COLON){
//     //     return ':';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_DOLLAR){
//     //     return '$';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_EXCLAIM){
//     //     return '!';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_GREATER){
//     //     return '>';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_HASH){
//     //     return '#';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_LEFTPAREN){
//     //     return '(';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_LESS){
//     //     return '<';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_PERCENT){
//     //     return '%';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_PLUS){
//     //     return '+';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_QUESTION){
//     //     return '?';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_QUOTEDBL){
//     //     return '"';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_RIGHTPAREN){
//     //     return ')';
//     // }
//
//     // if(stEvent.key.keysym.sym == SDLK_UNDERSCORE){
//     //     return '_';
//     // }
//
//     // SDL_Keycode lookup table
//
//
// 	static std::unordered_map<SDL_Keycode, std::string> stLoopupTable = {
//         {SDLK_SPACE, "  "},
//         {SDLK_QUOTE, "'\""},
//         {SDLK_COMMA, ",<"},
//         {SDLK_MINUS, "-_"},
//         {SDLK_PERIOD, ".>"},
//         {SDLK_SLASH, "/?"},
//         {SDLK_0, "0)"},
//         {SDLK_1, "1!"},
//         {SDLK_2, "2@"},
//         {SDLK_3, "3#"},
//         {SDLK_4, "4$"},
//         {SDLK_5, "5%"},
//         {SDLK_6, "6^"},
//         {SDLK_7, "7&"},
//         {SDLK_8, "8*"},
//         {SDLK_9, "9("},
//         {SDLK_SEMICOLON, ";:"},
//         {SDLK_EQUALS, "=+"},
//         {SDLK_LEFTBRACKET, "[{"},
//         {SDLK_BACKSLASH, "\\|"},
//         {SDLK_RIGHTBRACKET, "]}"},
//         {SDLK_BACKQUOTE, "`~"},
//         {SDLK_a, "aA"},
//         {SDLK_b, "bB"},
//         {SDLK_c, "cC"},
//         {SDLK_d, "dD"},
//         {SDLK_e, "eE"},
//         {SDLK_f, "fF"},
//         {SDLK_g, "gG"},
//         {SDLK_h, "hH"},
//         {SDLK_i, "iI"},
//         {SDLK_j, "jJ"},
//         {SDLK_k, "kK"},
//         {SDLK_l, "lL"},
//         {SDLK_m, "mM"},
//         {SDLK_n, "nN"},
//         {SDLK_o, "oO"},
//         {SDLK_p, "pP"},
//         {SDLK_q, "qQ"},
//         {SDLK_r, "rR"},
//         {SDLK_s, "sS"},
//         {SDLK_t, "tT"},
//         {SDLK_u, "uU"},
//         {SDLK_v, "vV"},
//         {SDLK_w, "wW"},
//         {SDLK_x, "xX"},
//         {SDLK_y, "yY"},
//         {SDLK_z, "zZ"}
//     };
//
//     if(stLoopupTable.find(stEvent.key.keysym.sym) != stLoopupTable.end()){
//         if(false
//                 || (stEvent.key.keysym.mod & KMOD_LSHIFT)
//                 || (stEvent.key.keysym.mod & KMOD_RSHIFT)
//           ){
//             return stLoopupTable[stEvent.key.keysym.sym][1];
//         }else{
//             return stLoopupTable[stEvent.key.keysym.sym][0];
//         }
//     }
//     return '\0';
// }

bool MyCopyFile(const char *szDst, const char *szSrc)
{
    FILE *fSrc = fopen(szSrc, "rb+");
    if(fSrc == nullptr){
        return false;
    }
    FILE *fDst = fopen(szDst, "wb+");
    if(fDst == nullptr){
        fclose(fSrc);
        return false;
    }

    char fileBuf[4096];
    int  fileSize;

    fseek(fSrc, 0L, SEEK_END);
    fileSize = ftell(fSrc);
    fseek(fSrc, 0L, SEEK_SET);

    for(int i = 0; i < fileSize / 4096; ++i){
        fread( fileBuf, 4096, 1, fSrc);
        fwrite(fileBuf, 4096, 1, fDst);
    }

    fread( fileBuf, fileSize % 4096, 1, fSrc);
    fwrite(fileBuf, fileSize % 4096, 1, fDst);
    fclose(fSrc);
    fclose(fDst);

    return true;
}

uint32_t *ShadowDecode(bool bProject,
        uint32_t *pData,
        int nW, int nH,
        int &nSW, int &nSH,
        uint32_t nColor)
{
	if(false
            || pData == nullptr
            || nW <= 0
            || nH <= 0
      ){
		return nullptr;
	}
    // if project, pShadowData should be of (nW + nH / 2) * (nH / 2 + 1)
    // otherwise of nW * nH
    if(bProject){
        int nNewW = (nW + nH / 2);
        int nNewH = (nH / 2 + 1);
        auto pShadowData = new uint32_t[nNewW * nNewH];
        std::memset(pShadowData, 0, nNewW * nNewH * sizeof(uint32_t));
        for(int nY = 0; nY < nH; ++nY){
            int nYCnt = nY - nY / 2;
            for(int nX = 0; nX < nW; ++nX){
                int nXCnt = nX + (nH - nY) / 2;
                if(pData[nY * nW + nX] & 0XFF000000){
                    pShadowData[nYCnt * nNewW + nXCnt] = nColor;
                }
            }
        }
        nSW = nNewW;
        nSH = nNewH;
        return pShadowData;
    }else{
        auto pShadowData = new uint32_t[nW * nH];
        std::memset(pShadowData, 0, nW * nH * sizeof(uint32_t));
        for(int nY = 0; nY < nH; ++nY){
            for(int nX = 0; nX < nW; ++nX){
                if(pData[nY * nW + nX] & 0XFF000000){
                    pShadowData[nY * nW + nX] = nColor;
                }
            }
        }
        nSW = nW;
        nSH = nH;
        return pShadowData;
    }
    return nullptr;
}

uint32_t *TwoPointShadowDecode(uint32_t *pData,
        int nW, int nH, int &nSW, int &nSH,
        int nX1, int nY1, int nX2, int nY2,
        uint32_t nShadowColor)
{
    if(nX1 > nX2){
        std::swap(nX1, nX2);
        std::swap(nY1, nY2);
    }

    if(nY1 <= nY2){
        // +----+--+-------+ (V1)
        // |    |  |       |
        // |(A1)|A2| (A3)  |
        // |    |  |       |
        // |(P1)|  |       |
        // +----+  |       |
        // |     \ |       |
        // |      \|       |
        // |   (P2)+-------+    
        // |               |
        // |(A4)           |
        // +---------------+
        // pixel in A123 will create shadow
        // we drop pixel in A4

        int nNewW = nY2 / 2 + nW;
        int nNewH = nY1 / 2 + nY2 - nY1;

        nSW = nNewW;
        nSH = nNewH;

        auto pSData = new uint32_t[nNewW * nNewH];
        std::memset(pSData, 0, nNewW * nNewH * sizeof(uint32_t));
        for(int nYCnt = 0; nYCnt < nNewH; ++nYCnt){
            for(int nXCnt = 0; nXCnt < nNewW; ++nXCnt){
                if(true
                        && (nYCnt + nY1 / 2 - nY1) > 0
                        && (nYCnt + nY1 / 2 - nY1) * (nX2 - nX1) > (nY2 - nY1) * (nXCnt - nX1)
                  ){
                    continue;
                }

                int  nMapX, nMapY;
                int  nCrossX = nXCnt - (nNewH - nYCnt);
                bool bExpand = false;
                bool bMapped = false;

                if(nX2 <= nCrossX && nCrossX < nW){
                    nMapX   = nCrossX;
                    nMapY   = nY2 - (nNewH - nYCnt) * 2;
                    bExpand = true;
                    bMapped = true;
                }else if(nX1 - (nY2 - nY1) <= nCrossX && nCrossX < nX2){
                    double dRatio = 1.0 * 
                        (nCrossX - (nX1 - (nY2 - nY1))) / (std::max)(1, nX2 - (nX1 - (nY2 - nY1)));
                    dRatio = (std::max)(0.0, (std::min)(dRatio, 1.0));
                    int nRealCrossX = nX1 + std::lround((nX2 - nX1) * dRatio);
                    int nRealCrossY = nY1 + std::lround((nY2 - nY1) * dRatio);
                    nMapX   = nRealCrossX;
                    nMapY   = nRealCrossY - 2 * (nRealCrossY - (nYCnt + nY1 / 2));
                    bExpand = false;
                    bMapped = true;
                }else if(-(nY2 - nY1) <= nCrossX && nCrossX < nX1 - (nY2 - nY1)){
                    nMapX   = nCrossX + (nY2 - nY1);
                    nMapY   = nY1 - 2 * (nY1 / 2 - nYCnt);
                    bExpand = true;
                    bMapped = true;
                }else{
                    nMapX   = -50000;
                    nMapY   = -50000;
                    bExpand =  false;
                    bMapped =  false; // not a valid mapping point
                }

                if(bMapped){
                    if(bExpand){
                        {
                            for(int nVX = -1; nVX <= 1; ++nVX){
                                for(int nVY = -1; nVY <= 1; ++nVY){
                                    int nMapXX = nMapX + nVX;
                                    int nMapYY = nMapY + nVY;
                                    if(nMapXX >= 0 && nMapXX < nW && nMapYY >= 0 && nMapYY < nH){
                                        if(pData[nMapXX + nMapYY * nW] & 0XFF000000){
                                            pSData[nXCnt + nYCnt * nNewW] = nShadowColor;
                                            goto __TwoPointShadowDecode_Pixel_Set;
                                        }
                                    }
                                }
                            }
__TwoPointShadowDecode_Pixel_Set: ;
                        }
                    }else{
                        if(nMapX >= 0 && nMapX < nW && nMapY >= 0 && nMapY < nH){
                            if(pData[nMapX + nMapY * nW] & 0XFF000000){
                                pSData[nXCnt + nYCnt * nNewW] = nShadowColor;
                            }
                        }
                    }
                }
            }
        }
        return pSData;
    }else{
        // this logic is much complicated, because of slope of (P1---P2)
        // which makes something prohibit us from applying previous logic
        //
        // nY1 > nY2
        //
        // +----+--+-------+ (V1)
        // |    |  |       |
        // |(A1)|A2| (A3)  |
        // |    |  |       |
        // |    |  +-------+
        // |    | /(P2)    |
        // |    |/         |
        // +----+          |
        // |   (P1)        |
        // |          (A4) |
        // +---------------+
        // pixel in A123 will create shadow
        // we drop pixel in A4

        int nNewW = nY2 / 2 + nW;
        int nNewH = nY2 / 2 + nY1 - nY2;

        nSW = nNewW;
        nSH = nNewH;

        auto pSData = new uint32_t[nNewW * nNewH];
        std::memset(pSData, 0, nNewW * nNewH * sizeof(uint32_t));
        for(int nYCnt = 0; nYCnt < nNewH; ++nYCnt){
            for(int nXCnt = 0; nXCnt < nNewW; ++nXCnt){
                if(true
                        && (nYCnt + nY2 / 2 - nY2) > 0
                        && (nYCnt + nY2 / 2 - nY1) * (nX2 - nX1) > (nY2 - nY1) * (nXCnt - nX1)){
                    continue;
                }
                if(nX1 <= nX2 - (nY1 - nY2)){
                    // normal, easy to use previous logic
                    int  nMapX, nMapY;
                    int  nCrossX = nXCnt - (nNewH - nYCnt);
                    bool bMapped = false;
                    bool bMiddle = false;

                    if(nX2 - (nY1 - nY2) <= nCrossX && nCrossX < nW - (nY1 - nY2)){
                        nMapX   = nCrossX + (nY1 - nY2);
                        nMapY   = nY2 - 2 * (nY2 / 2 - nYCnt);
                        bMapped = true;
                        bMiddle = false;
                    }else if(nX1 <= nCrossX && nCrossX <  nX2 - (nY1 - nY2)){
                        double dRatio = 1.0 * 
                            (nCrossX - nX1) / (std::max)(1, (nX2 - (nY1 - nY2)) - nX1);
                        dRatio = (std::max)(0.0, (std::min)(dRatio, 1.0));
                        int nRealCrossX = nX1 + std::lround((nX2 - nX1) * dRatio);
                        int nRealCrossY = nY2 + std::lround((nY1 - nY2) * (1.0 - dRatio));
                        nMapX   = nRealCrossX;
                        nMapY   = nRealCrossY - 2 * (nRealCrossY - (nYCnt + nY2 / 2));
                        bMapped = true;
                        bMiddle = true;
                    }else if(0 <= nCrossX && nCrossX < nX1){
                        nMapX   = nCrossX;
                        nMapY   = nY1 - 2 * (nNewH - nYCnt);
                        bMapped = true;
                        bMiddle = false;
                    }else{
                        bMapped = false;
                        bMiddle = false;
                    }

                    if(bMapped){
                        int nExpandCount = 0;
                        if(bMiddle){
                            double dSlope = 1.0 * (nY1 - nY2) / (std::max)(1, nX2 - nX1);
                            nExpandCount  = std::lround(1.4 * (std::max)(0.0, dSlope) + 1.2);
                        }else{
                            nExpandCount = 1;
                        }

                        { // expand with proper expand count
                            for(int nVX = -nExpandCount; nVX <= nExpandCount; ++nVX){
                                for(int nVY = -nExpandCount; nVY <= nExpandCount; ++nVY){
                                    int nMapXX = nMapX + nVX;
                                    int nMapYY = nMapY + nVY;
                                    if(nMapXX >= 0 && nMapXX < nW && nMapYY >= 0 && nMapYY < nH){
                                        if(pData[nMapXX + nMapYY * nW] & 0XFF000000){
                                            pSData[nXCnt + nYCnt * nNewW] = nShadowColor;
                                            goto __TwoPointShadowDecode_Pixel_Set2;
                                        }
                                    }
                                }
                            }
__TwoPointShadowDecode_Pixel_Set2: ;
                        }
                    }
                }else{
                    // nX1 > nX2 - (nY1 - nY2)
                    // the 45 line will cross with three point in the middle part !!!
                    //
                    // still we need nCrossX
                    int nCrossX = nXCnt - (nNewH - nYCnt);
                    if(nX2 - (nY1 - nY2) <= nCrossX && nCrossX < nX1){
                        // three cross point
                        int nMapX[3], nMapY[3];
                        nMapX[0] = nCrossX;
                        nMapY[0] = nY1 - 2 * (nNewH - nYCnt);
                        nMapX[2] = nCrossX + (nY1 - nY2);
                        nMapY[2] = nY2 - 2 * (nY2 / 2 - nYCnt);
                        { // middle point, since nX1 > (nX2 - (nY1 - nY2)) here, skip max(1, x)
                            double dRatio = 1.0 * 
                                (nCrossX - (nX2 - (nY1 - nY2))) / (nX1 - (nX2 - (nY1 - nY2)));
                            int nRealCrossX = nX1 + std::lround((nX2 - nX1) * (1.0 - dRatio));
                            int nRealCrossY = nY2 + std::lround((nY1 - nY2) * dRatio);
                            nMapX[1] = nRealCrossX;
                            nMapY[1] = nRealCrossY - 2 * (nRealCrossY - (nYCnt + nY2 / 2));
                        }
                        { // check three mapping points
                            for(int nIndex = 0; nIndex < 3; ++nIndex){
                                for(int nVX = -1; nVX <= 1; ++nVX){
                                    for(int nVY = -1; nVY <= 1; ++nVY){
                                        int nMapXX = nMapX[nIndex] + nVX;
                                        int nMapYY = nMapY[nIndex] + nVY;
                                        if(nMapXX >= 0 && nMapXX < nW && nMapYY >= 0 && nMapYY < nH){
                                            if(pData[nMapXX + nMapYY * nW] & 0XFF000000){
                                                pSData[nXCnt + nYCnt * nNewW] = nShadowColor;
                                                goto __TwoPointShadowDecode_Pixel_Set3;
                                            }
                                        }
                                    }
                                }
                            }
__TwoPointShadowDecode_Pixel_Set3: ;
                        }
                    }else{
                        int  nMapX   = -100;
                        int  nMapY   = -100;
                        bool bMapped =  false;
                        if(nX1 <= nCrossX && nCrossX < nW - (nY1 - nY2)){
                            nMapX   = nCrossX + (nY1 - nY2);
                            nMapY   = nY2 - 2 * (nY2 / 2 - nYCnt);
                            bMapped = true;
                        }else if(0 <= nCrossX && nCrossX < nX2 - (nY1 - nY2)){
                            // TODO maybe 0 > nX2 - (nY1 - nY2)
                            nMapX   = nCrossX;
                            nMapY   = nY1 - 2 * (nNewH - nYCnt);
                            bMapped = true;
                        }
                        if(bMapped){
                            { // check only one mapping point, fixedly expand to 3
                                for(int nVX = -1; nVX <= 1; ++nVX){
                                    for(int nVY = -1; nVY <= 1; ++nVY){
                                        int nMapXX = nMapX + nVX;
                                        int nMapYY = nMapY + nVY;
                                        if(nMapXX >= 0 && nMapXX < nW && nMapYY >= 0 && nMapYY < nH){
                                            if(pData[nMapXX + nMapYY * nW] & 0XFF000000){
                                                pSData[nXCnt + nYCnt * nNewW] = nShadowColor;
                                                goto __TwoPointShadowDecode_Pixel_Set4;
                                            }
                                        }
                                    }
                                }
__TwoPointShadowDecode_Pixel_Set4: ;
                            }
                        }
                    }
                }
            }
        }
        return pSData;
    }
    return nullptr;
}
