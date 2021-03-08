/*
 * =====================================================================================
 *
 *       Filename: pngf.cpp
 *        Created: 02/06/2016 04:25:40
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

#include <png.h>
#include <cstdio>
#include <cstring>
#include <csetjmp>
#include "pngf.hpp"

bool pngf::saveRGBABuffer(const uint8_t *bufRGBA, uint32_t imgWidth, uint32_t imgHeight, const char *fileName)
{
    // libpng uses longjmp
    // requres initializatio at beginning

    constexpr int pixelSize    = 4;
    FILE         *fp           = nullptr;
    png_infop     imgInfoPtr   = nullptr;
    png_structp   imgPtr       = nullptr;
    size_t        rowPtrBufLen = 0;
    png_byte    **rowPtrBuf    = nullptr;

    // result could be optimized into a register while setjmp only save current stack
    // then when jump back result may lost or invalid
    volatile bool result = false;

    if(!(true
                && bufRGBA
                && imgWidth > 0
                && imgHeight > 0
                && fileName
                && std::strlen(fileName))){

        // use goto sttement
        // then all variable declaration should at the very beginning
        goto pngf_saveRGBABuffer_check_argument_failed;
    }

    if(!(fp = std::fopen(fileName, "wb"))){
        goto pngf_saveRGBABuffer_fopen_failed;
    }

    if(!(imgPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr))){
        goto pngf_saveRGBABuffer_png_create_write_struct_failed;
    }

    if(!(imgInfoPtr = png_create_info_struct(imgPtr))){
        goto pngf_saveRGBABuffer_png_create_info_struct_failed;
    }

    // after this line
    // any failure in following png_* calls jumps back here, triggered by png_error()

    if(setjmp(png_jmpbuf(imgPtr))){
        goto pngf_saveRGBABuffer_failed;
    }

    // Initialize header information for png
    // RGBA means: [0] : R  : [07:00]
    //             [1] : G  : [15:08]
    //             [2] : B  : [23:16]
    //             [3] : A  : [31:24]

    png_set_IHDR(
            imgPtr,
            imgInfoPtr,
            imgWidth,
            imgHeight,
            8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);

    //  be extreme careful for memory allocation
    //  I use this function to generate extremely large PNG files, i.e. render the whole map

#ifdef PNG_FLAG_MALLOC_NULL_MEM_OK
#define CHECK_PNG_MALLOC_RESULT(img, buf) \
    do{ \
        if(((img)->flags & PNG_FLAG_MALLOC_NULL_MEM_OK) || ((buf) == nullptr)){ \
            png_error(img, "png module requires a safe allocator"); \
        } \
    }while(0)
#else
#define CHECK_PNG_MALLOC_RESULT(img, buf) \
    do{ \
        if((buf) == nullptr){ \
            png_error(img, "png module requires a safe allocator"); \
        } \
    }while(0)
#endif

    rowPtrBufLen = imgHeight * sizeof(png_byte *);
    rowPtrBuf =(png_byte **)(png_malloc(imgPtr, rowPtrBufLen));

    CHECK_PNG_MALLOC_RESULT(imgPtr, rowPtrBuf);
    std::memset(rowPtrBuf, 0, rowPtrBufLen);

    for(int y = 0; y < to_d(imgHeight); ++y){
        const size_t rowBufLen = sizeof(uint8_t) * pixelSize * imgWidth;
        auto rowBuf =(png_byte *)(png_malloc(imgPtr, rowBufLen));

        CHECK_PNG_MALLOC_RESULT(imgPtr, rowBuf);
        std::memcpy(rowBuf, bufRGBA + rowBufLen * y, rowBufLen);
        rowPtrBuf[y] = rowBuf;
    }

    // all data has been stored in imgPtr 
    // write data to binary file

    png_init_io(imgPtr, fp);
    png_set_rows(imgPtr, imgInfoPtr, rowPtrBuf);
    png_write_png(imgPtr, imgInfoPtr, PNG_TRANSFORM_IDENTITY, nullptr);
    result = true;

pngf_saveRGBABuffer_failed:
    if(rowPtrBuf){
        for(int y = 0; y < to_d(imgHeight); y++){
            if(rowPtrBuf[y]){
                png_free(imgPtr, rowPtrBuf[y]);
            }
        }
        png_free(imgPtr, rowPtrBuf);
    }

pngf_saveRGBABuffer_png_create_info_struct_failed:
    png_destroy_write_struct(&imgPtr, &imgInfoPtr);

pngf_saveRGBABuffer_png_create_write_struct_failed:
    std::fclose(fp);

pngf_saveRGBABuffer_fopen_failed:
pngf_saveRGBABuffer_check_argument_failed:
    return result;
}
