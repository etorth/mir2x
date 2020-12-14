/*
 * =====================================================================================
 *
 *       Filename: channpackq.cpp
 *        Created: 01/27/2018 11:22:45
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

#include "compress.hpp"
#include "monoserver.hpp"
#include "channpackq.hpp"

bool ChannPackQ::AddChannPack(uint8_t nHC, const uint8_t *pData, size_t nDataLen, std::function<void()> &&rstDoneCB)
{
    auto fnReportError = [nHC, pData, nDataLen](const char *pErrorMessage)
    {
        extern MonoServer *g_monoServer;
        g_monoServer->addLog(LOGTYPE_WARNING, "%s: (%d, %p, %d)", (pErrorMessage ? pErrorMessage : "Empty message"), (int)(nHC), pData, (int)(nDataLen));
    };

    ServerMsg smSG(nHC);
    switch(smSG.type()){
        case 0:
            {
                // empty body, head code only

                if(pData || nDataLen){
                    fnReportError("Invalid argument");
                    return false;
                }

                auto pDst = GetPostBuf(1);

                pDst[0] = nHC;
                AddPackMark(GetBufOff(pDst), 1, std::move(rstDoneCB));

                return true;
            }
        case 1:
            {
                // not empty, fixed size, comperssed

                if(!(pData && (nDataLen == smSG.dataLen()))){
                    fnReportError("Invalid argument");
                    return false;
                }

                // length encoding:
                // [0 - 254]          : length in 0 ~ 254
                // [    255][0 ~ 255] : length as 0 ~ 255 + 255

                // 1. most likely we are using 0 ~ 254
                // 2. if compressed length more than 254 we need two bytes
                // 3. we support range in [0, 255 + 255]

                auto pCompBuf = GetPostBuf(smSG.maskLen() + ((nDataLen + 7) / 8) + 16);
                auto nCompCnt = Compress::xorEncode(pCompBuf + 4, pData, nDataLen);

                if(nCompCnt < 0){
                    fnReportError("Compression failed");
                    return false;
                }else if(nCompCnt <= 254){
                    pCompBuf[2] = nHC;
                    pCompBuf[3] = (uint8_t)(nCompCnt);

                    return AddPackMark(GetBufOff(pCompBuf + 2), 2 + smSG.maskLen() + (size_t)(nCompCnt), std::move(rstDoneCB));
                }else if(nCompCnt <= (255 + 255)){
                    pCompBuf[1] = nHC;
                    pCompBuf[2] = 255;
                    pCompBuf[3] = (uint8_t)(nCompCnt - 255);

                    return AddPackMark(GetBufOff(pCompBuf + 1), 3 + smSG.maskLen() + (size_t)(nCompCnt), std::move(rstDoneCB));
                }else{
                    fnReportError("Compressed data too long");
                    return false;
                }
                return false;
            }
        case 2:
            {
                // not empty, fixed size, not compressed

                if(!(pData && (nDataLen == smSG.dataLen()))){
                    fnReportError("Invalid argument");
                    return false;
                }

                // for fixed size and uncompressed message
                // we don't need to send the length info since it's public known

                auto pDst = GetPostBuf(smSG.dataLen() + 1);
                pDst[0] = nHC;
                std::memcpy(pDst + 1, pData, nDataLen);
                return AddPackMark(GetBufOff(pDst), 1 + smSG.dataLen(), std::move(rstDoneCB));
            }
        case 3:
            {
                // not empty, not fixed size, not compressed

                if(pData){
                    if((nDataLen == 0) || (nDataLen > 0XFFFFFFFF)){
                        fnReportError("Invalid argument");
                        return false;
                    }
                }else{
                    if(nDataLen){
                        fnReportError("Invalid argument");
                        return false;
                    }
                }

                auto pDst = GetPostBuf(nDataLen + 4);

                // 1. setup the message length encoding
                {
                    auto nDataLenU32 = (uint32_t)(nDataLen);
                    std::memcpy(pDst, &nDataLenU32, sizeof(nDataLenU32));
                }

                // 2. copy data if there is
                if(pData){
                    std::memcpy(pDst + 4, pData, nDataLen);
                }

                return AddPackMark(GetBufOff(pDst), nDataLen + 4, std::move(rstDoneCB));
            }
        default:
            {
                fnReportError("Invalid argument");
                return false;
            }
    }
}

bool ChannPackQ::AddPackMark(size_t nLoc, size_t nLength, std::function<void()> &&rstDoneCB)
{
    if(!m_packMarkQ.empty()){
        if(nLoc < m_packMarkQ.back().Loc + m_packMarkQ.back().Length){
            return false;
        }
    }

    m_packMarkQ.emplace_back(nLoc, nLength, std::move(rstDoneCB));
    return true;
}

uint8_t *ChannPackQ::GetPostBuf(size_t nNextLen)
{
    // find the first byte of the new buffer to allocate
    // make sure current buffer can hold more nNextLen bytes for the new one

    size_t nNextLoc0 = 0;

    if(!m_packMarkQ.empty()){
        nNextLoc0 = m_packMarkQ.back().Loc + m_packMarkQ.back().Length;
    }

    m_packBuf.resize(nNextLoc0 + nNextLen + 16);
    return &(m_packBuf[0]) + nNextLoc0;
}
