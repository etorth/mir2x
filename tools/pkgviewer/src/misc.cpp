/*
 * =====================================================================================
 *
 *       Filename: misc.cpp
 *        Created: 7/24/2015 7:20:18 PM
 *  Last Modified: 07/25/2015 1:02:31 AM
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

#include <png.h>
#include <cstdio>
#include <cstdint>
#include <cstring>


bool SaveRGBABufferToPNG(const uint8_t *rgbaBuff, uint32_t nW, uint32_t nH, const char *fileFullName)
{
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
    for(int y = 0; y < nH; ++y){
        png_byte *row =(png_byte *)png_malloc(png_ptr, sizeof(uint8_t) * nW * nPixelSize);
        std::memcpy(row, rgbaBuff + nW * y * nPixelSize * sizeof(uint8_t), nW * nPixelSize * sizeof(uint8_t));
        row_pointers[y] = row;
    }

    /* Write the image data to "fp". */

    png_init_io(png_ptr, fp);
    png_set_rows(png_ptr, info_ptr, row_pointers);
    png_write_png(png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, nullptr);

    status = true;

    for(int y = 0; y < nH; y++){
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
