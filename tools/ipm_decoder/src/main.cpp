/*
 * =====================================================================================
 *
 *       Filename: main.cpp
 *        Created: 04/03/2017 18:02:52
 *  Last Modified: 07/12/2017 18:27:59
 *
 *    Description: decode re_xue_sha_chen ipm file to all files it includes
 *
 *                      ipm_decoder magic_str full_ipm_path out_dir
 *
 *                 i.e.
 *                  
 *                      ipm_decoder codudu.ipm amp/login.apm home/you/out
 *
 *                 will print all needed information during extracting
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

#include <string>
#include <vector>
#include <cstdio>
#include <cstring>
#include <cstdint>
#include <iconv.h>

void printUsage()
{
    std::printf("%s",
            "Usage: decode re_xue_sha_chen ipm file to all files it includes\n"
            "\n"
            "           ipm_decoder magic_str full_ipm_path out_dir\n"
            "\n"
            "       magic_str should be cocodu.ipm, i.e.\n"
            "\n"
            "           ipm_decoder codudu.ipm amp/login.apm home/you/out\n"
            "\n"
            "       will print all needed information during extracting\n");
}

int ipm_Decode(const char *szMagicStr, const char *szFPMFileName, const char *szOutDir)
{
    if(auto fp = std::fopen(szFPMFileName, "rb")){
        uint32_t nU32;
        std::vector<uint8_t> stByteBuf;

        // 1. read magic string
        std::fread(&nU32, 4, 1, fp);
        stByteBuf.resize(nU32 + 1024);
        std::memset(&(stByteBuf[0]), 0, stByteBuf.size());
        std::fread(&(stByteBuf[0]), nU32, 1, fp);

        if(std::strcmp(szMagicStr, (char *)(&(stByteBuf[0])))){
            std::printf("Invalid package: %s, magic: %s, expecting: %s\n", szFPMFileName, (char *)(&(stByteBuf[0])), szMagicStr);
            return 1;
        }

        // 2. read file count
        uint32_t nFileCount = 0;
        std::fread(&nFileCount, 4, 1, fp);

        if(nFileCount == 0){
            std::printf("Empty package: %s, magic: %s, fileCount = 0\n", szFPMFileName, szMagicStr);
            return 1;
        }

        // OK we have fileCount
        // extract all files contained in this package
        uint32_t nFileSize;
        uint32_t nFileOffset;

        int32_t nXOffset;
        int32_t nYOffset;
        int32_t nWidth;
        int32_t nHeight;
        std::string szFileName;

        std::vector<uint32_t> stByteBufConv;
        for(uint32_t nIndex = 0; nIndex < nFileCount; ++nIndex){
            // 1. read file name
            std::fread(&nU32, 4, 1, fp);
            stByteBuf.resize(nU32 + 1024);
            std::memset(&(stByteBuf[0]), 0, stByteBuf.size());
            std::fread(&(stByteBuf[0]), nU32, 1, fp);

            // now size is nU32, data is &(stByteBuf[0])
            // call libiconv to convert to string with utf-8 encoding
            {
                stByteBufConv.resize(stByteBuf.size() + 128);

                auto pInBuf  = (char *)(&(stByteBuf[0]));
                auto pOutBuf = (char *)(&(stByteBufConv[0]));
                auto nInLen  = stByteBuf.size();
                auto nOutLen = stByteBufConv.size();

                auto icv = iconv_open("utf-8", "cp936");
                iconv(icv, &pInBuf, &nInLen, &pOutBuf, &nOutLen);
                iconv_close(icv);
            }

            szFileName = (char *)(&(stByteBufConv[0]));

            // x. read XOff, YOff, Width, Height
            std::fread(&nXOffset, 4, 1, fp);
            std::fread(&nYOffset, 4, 1, fp);
            std::fread(&nWidth,   4, 1, fp);
            std::fread(&nHeight,  4, 1, fp);

            // 2. read file size
            std::fread(&nFileSize, 4, 1, fp);

            // 3. read file offset
            std::fread(&nFileOffset, 4, 1, fp);

            if(false
                    || szFileName.empty()
                    || nFileSize == 0
                    || nFileOffset == 0){
                std::printf("Error in decoding file = %s, size = %d, offset = %d\n", szFileName.c_str(), (int)(nFileSize), (int)(nFileOffset));
                return 1;
            }

            // store current file offset for next file read
            auto nCurrOffset = std::ftell(fp);

            // 4. extracting it
            std::printf("Extracting [%04d/%04d] %s\n", (int)(nIndex) + 1, (int)(nFileCount), szFileName.c_str());

            stByteBuf.resize(nFileSize + 1024);
            std::memset(&(stByteBuf[0]), 0, stByteBuf.size());
            std::fseek(fp, nFileOffset, SEEK_SET);
            std::fread(&(stByteBuf[0]), nFileSize, 1, fp);

            if(auto fSave = std::fopen((std::string(szOutDir) + "/" + szFileName).c_str(), "wb")){
                std::fwrite(&(stByteBuf[0]), nFileSize, 1, fSave);
                std::fclose(fSave);
            }else{
                std::printf("Error in decoding file = %s, size = %d, offset = %d: save failed\n", szFileName.c_str(), (int)(nFileSize), (int)(nFileOffset));
                return 1;
            }

            std::fseek(fp, nCurrOffset, SEEK_SET);
        }
    }

    return 1;
}

int main(int argc, char *argv[])
{
    if(argc != 4){
        printUsage();
        return 1;
    }

    if(std::strcmp(argv[1], "codudu.ipm")){
        printUsage();
        return 1;
    }

    return ipm_Decode(argv[1], argv[2], argv[3]);
}
