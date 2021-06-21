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
#include "imagedb.hpp"
#include "fflerror.hpp"

int main(int argc, char *argv[])
{
    if(argc != 3){
        throw fflerror("Usage: mapwil2png map-wil-path map-png-output-dir");
    }

    fflassert(str_haschar(argv[1])); // map-wil-path
    fflassert(str_haschar(argv[2])); // map-png-output-path

    std::string fileName;
    std::vector<uint32_t> convBuf;
    ImageDB(argv[1]).extract([argv, &fileName, &convBuf](uint8_t fileIndex, uint16_t imageIndex, const void *imgBuf, size_t imgW, size_t imgH)
    {
        fflassert(imgBuf);
        fflassert(imgW > 0);
        fflassert(imgH > 0);

        const auto p    = (const uint32_t *)(imgBuf);
        const auto pend = (const uint32_t *)(imgBuf) + imgW * imgH;

        convBuf.clear();
        convBuf.assign(p, pend);
        alphaf::autoShadowRemove(convBuf.data(), imgW, imgH, colorf::BLACK + colorf::A_SHF(0X80));

        str_printf(fileName, "%s/%08llX.PNG", argv[2], (to_llu(fileIndex) << 16) + imageIndex);
        imgf::saveImageBuffer(convBuf.data(), imgW, imgH, fileName.c_str());
    });
    return 0;
}
