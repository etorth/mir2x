/*
 * =====================================================================================
 *
 *       Filename: imgf.cpp
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
#include "imgf.hpp"
#include "mathf.hpp"
#include "colorf.hpp"
#include "totype.hpp"
#include "fflerror.hpp"

bool imgf::roiCrop(
        int &dstX, int &dstY,
        int &srcX, int &srcY,
        int &srcW, int &srcH,

        int origSrcW,
        int origSrcH,

        int roiSrcX, int roiSrcY, int roiSrcW, int roiSrcH,
        int roiDstX, int roiDstY, int roiDstW, int roiDstH)
{
    fflassert(origSrcW > 0);
    fflassert(origSrcH > 0);

    if(roiSrcW < 0){
        roiSrcW = origSrcW - roiSrcX;
    }

    if(roiSrcH < 0){
        roiSrcH = origSrcH - roiSrcY;
    }

    int roiDst2SrcX = roiDstX + (srcX - dstX);
    int roiDst2SrcY = roiDstY + (srcY - dstY);

    if(roiDstW < 0){
        roiDstW = origSrcW - roiDst2SrcX;
    }

    if(roiDstH < 0){
        roiDstH = origSrcH - roiDst2SrcY;
    }

    int roiDst2SrcW = roiDstW;
    int roiDst2SrcH = roiDstH;

    if(!mathf::rectangleOverlapRegion(roiSrcX, roiSrcY, roiSrcW, roiSrcH, roiDst2SrcX, roiDst2SrcY, roiDst2SrcW, roiDst2SrcH)){
        return false;
    }

    int oldSrcX = srcX;
    int oldSrcY = srcY;
    int oldSrcW = srcW;
    int oldSrcH = srcH;

    if(srcW < 0){
        srcW = origSrcW - srcX;
    }

    if(srcH < 0){
        srcH = origSrcH - srcY;
    }

    if(!mathf::rectangleOverlapRegion(roiDst2SrcX, roiDst2SrcY, roiDst2SrcW, roiDst2SrcH, srcX, srcY, srcW, srcH)){
        srcW = oldSrcW; // restore if failed
        srcH = oldSrcH; // ...
        return false;
    }

    dstX += (srcX - oldSrcX);
    dstY += (srcY - oldSrcY);
    return true;
}

void imgf::blendImageBuffer(
        /* */ uint32_t *dstBuf, size_t dstBufW, size_t dstBufH,
        const uint32_t *srcBuf, size_t srcBufW, size_t srcBufH,

        int dstX, int dstY,
        int srcX, int srcY,
        int srcW, int srcH)
{
    fflassert(dstBuf);
    fflassert(dstBufW > 0);
    fflassert(dstBufH > 0);

    fflassert(srcBuf);
    fflassert(srcBufW > 0);
    fflassert(srcBufH > 0);

    if(!imgf::roiCrop(dstX, dstY, srcX, srcY, srcW, srcH, to_d(srcBufW), to_d(srcBufH))){
        return;
    }

    for(int srcDY = 0; srcDY < srcH; ++srcDY){
        for(int srcDX = 0; srcDX < srcW; ++srcDX){
            const auto &src = srcBuf[(srcX + srcDX) + (srcY + srcDY) * srcBufW];
            /* */ auto &dst = dstBuf[(dstX + srcDX) + (dstY + srcDY) * dstBufW];
            dst = colorf::renderRGBA(dst, src);
        }
    }
}

void imgf::blendImageBuffer(
        /* */ uint32_t *dstBuf, size_t dstBufW, size_t dstBufH,
        const uint32_t *srcBuf, size_t srcBufW, size_t srcBufH, int dstX, int dstY)
{
    imgf::blendImageBuffer(dstBuf, dstBufW, dstBufH, srcBuf, srcBufW, srcBufH, dstX, dstY, 0, 0, to_d(srcBufW), to_d(srcBufH));
}

bool imgf::saveImageBuffer(const void *imgBuf, size_t imgW, size_t imgH, const char *fileName)
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
                && imgBuf
                && imgW > 0
                && imgH > 0
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

    // Initialize header information for png, here the default RGBA doesn't match colorf::RGBA model:
    // RGBA means here: [0] : R  : [07:00] 0X000000FF
    //                  [1] : G  : [15:08] 0X0000FF00
    //                  [2] : B  : [23:16] 0X00FF0000
    //                  [3] : A  : [31:24] 0XFF000000
    //
    // I tried to call following functions and works
    //
    //     png_set_bgr(imgPtr);            // -> ARGB
    //     png_set_swap_alpha(imgPtr);     // -> RGBA
    //
    // but looks manual/examples uses transform when calling png_write_png:
    // https://www.roxlu.com/2015/050/saving-pixel-data-using-libpng

    png_set_IHDR(
            imgPtr,
            imgInfoPtr,
            imgW,
            imgH,
            8,
            PNG_COLOR_TYPE_RGBA,    // -> ABGR
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

    rowPtrBufLen = imgH * sizeof(png_byte *);
    rowPtrBuf =(png_byte **)(png_malloc(imgPtr, rowPtrBufLen)); // strict-aliasing issue ???

    CHECK_PNG_MALLOC_RESULT(imgPtr, rowPtrBuf);
    std::memset(rowPtrBuf, 0, rowPtrBufLen);

    for(size_t y = 0; y < imgH; ++y){
        rowPtrBuf[y] = (png_byte *)(imgBuf) + sizeof(uint8_t) * pixelSize * imgW * y; // const cast
    }

    png_init_io(imgPtr, fp);
    png_set_rows(imgPtr, imgInfoPtr, rowPtrBuf);
    png_write_png(imgPtr, imgInfoPtr, PNG_TRANSFORM_BGR | PNG_TRANSFORM_SWAP_ALPHA, nullptr);
    result = true;

pngf_saveRGBABuffer_failed:
    if(rowPtrBuf){
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
