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

struct saveImageBufferHelperArgs
{
    // inputs
    const char *fileName;
    const void *imgBuf;

    size_t imgW;
    size_t imgH;

    // temp vars that go cross setjmp

    FILE         *fp           = nullptr;
    png_infop     imgInfoPtr   = nullptr;
    png_structp   imgPtr       = nullptr;
    png_byte    **rowPtrBuf    = nullptr;
    size_t        rowPtrBufLen = 0;
    bool          result       = false;
};

static bool saveImageBuffer_helper(saveImageBufferHelperArgs *args)
{
    if(!(true
                && args->imgBuf
                && args->imgW > 0
                && args->imgH > 0
                && str_haschar(args->fileName))){

        // use goto statement
        // then all variable declarations should be at the very beginning
        goto imgf_saveRGBABuffer_check_argument_failed;
    }

    if(!(args->fp = std::fopen(args->fileName, "wb"))){
        goto imgf_saveRGBABuffer_fopen_failed;
    }

    if(!(args->imgPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr))){
        goto imgf_saveRGBABuffer_png_create_write_struct_failed;
    }

    if(!(args->imgInfoPtr = png_create_info_struct(args->imgPtr))){
        goto imgf_saveRGBABuffer_png_create_info_struct_failed;
    }

    // after this line
    // any failure in following png_* calls jumps back here, triggered by png_error()

    if(setjmp(png_jmpbuf(args->imgPtr))){
        goto imgf_saveRGBABuffer_failed;
    }

    // Initialize header information for png, here the default uses RGBA model:
    // RGBA means here: [0] : R  : [07:00] 0X000000FF
    //                  [1] : G  : [15:08] 0X0000FF00
    //                  [2] : B  : [23:16] 0X00FF0000
    //                  [3] : A  : [31:24] 0XFF000000

    png_set_IHDR(
            args->imgPtr,
            args->imgInfoPtr,
            args->imgW,
            args->imgH,
            8,
            PNG_COLOR_TYPE_RGBA,
            PNG_INTERLACE_NONE,
            PNG_COMPRESSION_TYPE_DEFAULT,
            PNG_FILTER_TYPE_DEFAULT);

    //  be extremely careful for memory allocation
    //  I use this function to generate extremely large PNG files, i.e., render the whole map

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

    // rowPtrBufLen is not used after longjmp
    // but cannot define rowPtrBufLen here since goto can not cross variable definition

    args->rowPtrBufLen = args->imgH * sizeof(png_byte *);
    args->rowPtrBuf =(png_byte **)(png_malloc(args->imgPtr, args->rowPtrBufLen));

    CHECK_PNG_MALLOC_RESULT(args->imgPtr, args->rowPtrBuf);
    std::memset(args->rowPtrBuf, 0, args->rowPtrBufLen);

    for(size_t y = 0; y < args->imgH; ++y){
        args->rowPtrBuf[y] = (png_byte *)(args->imgBuf) + sizeof(uint8_t) * 4 * args->imgW * y; // const cast
    }

    png_init_io(args->imgPtr, args->fp);
    png_set_rows(args->imgPtr, args->imgInfoPtr, args->rowPtrBuf);
    png_write_png(args->imgPtr, args->imgInfoPtr, PNG_TRANSFORM_IDENTITY, nullptr);
    args->result = true;

imgf_saveRGBABuffer_failed:
    if(args->rowPtrBuf){
        png_free(args->imgPtr, args->rowPtrBuf);
    }

imgf_saveRGBABuffer_png_create_info_struct_failed:
    png_destroy_write_struct(&args->imgPtr, &args->imgInfoPtr);

imgf_saveRGBABuffer_png_create_write_struct_failed:
    if(args->fp){
        std::fclose(args->fp);
    }

imgf_saveRGBABuffer_fopen_failed:
imgf_saveRGBABuffer_check_argument_failed:
    return args->result;
}

bool imgf::saveImageBuffer(const void *imgBuf, size_t imgW, size_t imgH, const char *fileName)
{
    // libpng uses longjmp
    // requires initialization at beginning

    // result could be optimized into a register while setjmp only saves the current stack
    // then when jumping back, result may be lost or invalid

    saveImageBufferHelperArgs args
    {
        .fileName = fileName,
        .imgBuf   = imgBuf,
        .imgW     = imgW,
        .imgH     = imgH,
    };

    return saveImageBuffer_helper(&args);
}
