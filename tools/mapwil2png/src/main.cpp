/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 04/03/2017 18:02:52
 *    Description: convert map graphics res to png files, usage:
 *
 *                     mapwil2png map-wil-path map-png-output-dir
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

#include <vector>
#include <string>
#include <cstdlib>
#include <cstdint>
#include <cinttypes>

#include "strf.hpp"
#include "imgf.hpp"
#include "alphaf.hpp"
#include "imagemapdb.hpp"
#include "fflerror.hpp"

int main(int argc, char *argv[])
{
    if(argc != 3){
        throw fflerror("Usage: mapwil2png map-wil-path map-png-output-dir");
    }

    fflassert(str_haschar(argv[1])); // map-wil-path
    fflassert(str_haschar(argv[2])); // map-png-output-path

    std::string fileName;
    ImageMapDB(argv[1]).extract([argv, &fileName](uint8_t fileIndex, uint16_t imageIndex, const void *imgBuf, size_t imgW, size_t imgH)
    {
        fflassert(imgBuf);
        fflassert(imgW > 0);
        fflassert(imgH > 0);

        str_printf(fileName, "%s/%08llX.PNG", argv[2], (to_llu(fileIndex) << 16) + imageIndex);
        imgf::saveImageBuffer(imgBuf, imgW, imgH, fileName.c_str());
    },

    true); // remove shadow mosaic
    return 0;
}
