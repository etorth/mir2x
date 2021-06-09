/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 04/03/2017 18:02:52
 *    Description: convert magic graphics res to png files, usage:
 *
 *                      magicwil2png mir2x_Data_path    # data path
 *                                   out-dir            # output dir
 *                                   N                  # prefix width, zero means no prefix
 *
 *                  i.e. if mir2x Data installed at: /mnt/c/Users/anhong/Desktop/Data
 *                  there are files:
 *
 *                      /mnt/c/Users/anhong/Desktop/Data/Magic.wil
 *                      /mnt/c/Users/anhong/Desktop/Data/MagicEx.wil
 *                      /mnt/c/Users/anhong/Desktop/Data/MagicEx2.wil
 *
 *                  then issue command:
 *
 *                      magicwil2png /mnt/c/Users/anhong/Desktop/Data out-dir 4
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
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cstdint>
#include <cinttypes>
#include <algorithm>

#include "totype.hpp"
#include "fflerror.hpp"
#include "pngf.hpp"
#include "alphaf.hpp"
#include "filesys.hpp"
#include "wilimagepackage.hpp"

const char *createOffsetFileName(char *fileNameBuf, const char *outDir, int fileIndex, int imgIndex,  int dx, int dy, int prefixIndex, int prefixWidth)
{
    if(!fileNameBuf){
        throw fflerror("invalid arguments");
    }

    char prefixBuf[64];
    if(prefixWidth > 0){
        std::sprintf(prefixBuf, "%0*d_", prefixWidth, prefixIndex);
    }
    else{
        prefixBuf[0] = '\0';
    }

    std::sprintf(fileNameBuf, "%s/%s%02llX%06llX%s%s%04X%04X.PNG",
            outDir,
            prefixBuf,
            to_llu(fileIndex),
            to_llu(imgIndex),
            ((dx > 0) ? "1" : "0"),
            ((dy > 0) ? "1" : "0"),
            std::abs(dx),
            std::abs(dy));

    return fileNameBuf;
}

void magicWil2PNG(const char *dataPath, const char *outDir, int prefixWidth)
{
    int prefixIndex = 0;
    for(int fileIndex = 0; const auto fileBodyName:
    {
        "Magic",
        "MagicEx",
        "MagicEx2",

        "MonMagic",
        "MonMagicEx",
        "MonMagicEx2",
        "MonMagicEx3",
        "MonMagicEx4",
    }){
        WilImagePackage imgPackage;
        if(!imgPackage.Load(dataPath, fileBodyName, "wil")){
            throw fflerror("load wil file failed: %s/%s.wil", dataPath, fileBodyName);
        }

        std::vector<uint32_t> pngBuf;
        for(int i = 0; i < imgPackage.IndexCount(); ++i){
            if(imgPackage.setIndex(i) && imgPackage.CurrentImageValid()){
                const auto imgInfo = imgPackage.CurrentImageInfo();
                pngBuf.resize(imgInfo.width * imgInfo.height);
                imgPackage.Decode(&(pngBuf[0]), 0XFFFFFFFF, 0XFFFFFFFF, 0XFFFFFFFF);
                alphaf::autoAlpha(pngBuf.data(), pngBuf.size());

                char saveFileName[256];
                createOffsetFileName(saveFileName, outDir, fileIndex, i, imgInfo.px, imgInfo.py, prefixIndex++, prefixWidth);

                if(!pngf::saveRGBABuffer((uint8_t *)(pngBuf.data()), imgInfo.width, imgInfo.height, saveFileName)){
                    throw fflerror("save PNG failed: %s", saveFileName);
                }
            }
        }
        fileIndex++;
    }
}

int main(int argc, char *argv[])
{
    if(argc != 4){
        throw fflerror("invalid arguments");
    }

    magicWil2PNG(argv[1], argv[2], std::stoi(argv[3]));
    return 0;
}
