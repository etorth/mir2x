/*
 * =====================================================================================
 *
 *       Filename: savepng.cpp
 *        Created: 02/06/2016 04:25:40
 *  Last Modified: 08/22/2017 13:00:55
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

#include "savepng.hpp"

bool SaveRGBABufferToPNG(const uint8_t *bufRGBA, uint32_t nW, uint32_t nH, const char *szFileName)
{
    int           nPixelSize = 4;
    FILE         *fp         = nullptr;
    png_byte    **pRowPtr    = nullptr;
    png_infop     pImageInfo = nullptr;
    png_structp   pImage     = nullptr;

    // will assign bRet as true after setjmp
    // bRet could be optimized into a register while setjmp only save current stack
    // then when jump back bRet may lost or invalid
    volatile bool bRet = false;

    if(!(true
                && bufRGBA
                && nW > 0
                && nH > 0
                && szFileName
                && std::strlen(szFileName))){

        // use goto sttement
        // then all variable declaration should at the very beginning
        goto SaveRGBABufferToPNG_fopen_failed;
    }

    if(!(fp = fopen(szFileName, "wb"))){
        goto SaveRGBABufferToPNG_fopen_failed;
    }

    if(!(pImage = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr))){
        goto SaveRGBABufferToPNG_png_create_write_struct_failed;
    }

    if(!(pImageInfo = png_create_info_struct(pImage))){
        goto SaveRGBABufferToPNG_png_create_info_struct_failed;
    }

    // Set up error handling
    if(setjmp(png_jmpbuf(pImage))){
        goto SaveRGBABufferToPNG_png_failure;
    }

    // Initialize header information for png
    // RGBA means: [0] : R  : [07:00]
    //             [1] : G  : [15:08]
    //             [2] : B  : [23:16]
    //             [3] : A  : [31:24]
    png_set_IHDR(pImage, pImageInfo,
            nW, nH, 8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);

    // Initialize rows of PNG
    pRowPtr =(png_byte **)(png_malloc(pImage, nH * sizeof(png_byte *)));
    for(int nY = 0; nY < (int)(nH); ++nY){
        png_byte *pRow =(png_byte *)(png_malloc(pImage, sizeof(uint8_t) * nW * nPixelSize));
        std::memcpy(pRow, bufRGBA + nW * nY * nPixelSize * sizeof(uint8_t), nW * nPixelSize * sizeof(uint8_t));
        pRowPtr[nY] = pRow;
    }

    // write data to binary file
    // all data has been stored in pimage
    png_init_io(pImage, fp);
    png_set_rows(pImage, pImageInfo, pRowPtr);
    png_write_png(pImage, pImageInfo, PNG_TRANSFORM_IDENTITY, nullptr);

    bRet = true;

    for(int y = 0; y < (int)nH; y++){
        png_free(pImage, pRowPtr[y]);
    }

    png_free(pImage, pRowPtr);

SaveRGBABufferToPNG_png_failure:
SaveRGBABufferToPNG_png_create_info_struct_failed:
    png_destroy_write_struct(&pImage, &pImageInfo);
SaveRGBABufferToPNG_png_create_write_struct_failed:
    fclose(fp);
SaveRGBABufferToPNG_fopen_failed:
    return bRet;
}
