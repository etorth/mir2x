/*
 * =====================================================================================
 *
 *       Filename: pngtexoffdb.hpp
 *        Created: 02/26/2016 21:48:43
 *  Last Modified: 03/17/2016 01:25:45
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

#pragma once
#include <utility>
#include <unordered_map>

#include <zip.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "hexstring.hpp"
#include "cachequeue.hpp"
#include "sdldevice.hpp"

// don't change this setting easily
// since it will affect hash key of linear cache
#define PNGTEXOFFDB_LINEAR_CACHE_SIZE   1024

template<int N>
class PNGTexOffDB
{
    private:

        // linear cache
        std::array<CacheQueue<std::tuple<SDL_Texture *,
            int, uint32_t>, N>, PNGTEXOFFDB_LINEAR_CACHE_SIZE> m_LCache;

        // main cache
        std::unordered_map<uint32_t, std::pair<int, SDL_Texture *>> m_Cache;

        // index cache
        std::unordered_map<uint32_t, std::tuple<zip_uint64_t, zip_uint64_t, int, int>> m_IndexCache;

        // time stamp queue
        std::queue<std::tuple<int, uint32_t>>  m_TimeStampQ;

        int      m_ResourceMaxCount;
        int      m_ResourceCount;
        zip_t   *m_ZIP;
        uint8_t *m_Buf;
        size_t   m_BufSize;
        int      m_CurrentTime;

    public:
        PNGTexOffDB()
            : m_ResourceMaxCount(N * 512 + 2000)
            , m_ResourceCount(0)
            , m_ZIP(nullptr)
            , m_Buf(nullptr)
            , m_BufSize(0)
            , m_CurrentTime(0)
        {
        }

        ~PNGTexOffDB()
        {
            Clear();
            delete m_Buf;

            if(m_ZIP){
                zip_close(m_ZIP);
            }
        }

    public:

        bool Valid()
        {
            return (m_ZIP != nullptr);
        }

        bool Load(const char *szPNGTexDBName)
        {
            int nErrorCode;

            m_ZIP = zip_open(szPNGTexDBName, ZIP_RDONLY, &nErrorCode);
            return (m_ZIP != nullptr);

            zip_uint64_t nNumEntries = zip_get_num_entries(m_ZIP, ZIP_FL_UNCHANGED);
            for(zip_uint64_t nIndex = 0; nIndex < nNumEntries; ++nIndex){
                zip_stat_t stStatus;
                if(zip_stat_index(m_ZIP, nIndex, ZIP_FL_ENC_RAW, &stStatus)){
                    Record(stStatus);
                }else{
                    m_IndexCache.clear();
                    zip_close(m_ZIP); m_ZIP = nullptr;
                    return false;
                }
            }
            return true;
        }

        void Clear()
        {
            if(N != 0){
                for(auto &stQN: m_LCache){
                    stQN.Clear();
                }
            }

            for(auto &stRecord: m_Cache){
                SDL_DestroyTexture(stRecord.second.second);
            }

            m_Cache.clear();
        }

    public:

        SDL_Texture *Retrieve(uint32_t nKey)
        {
            // everytime when call this function
            // there must be a pointer retrieved responding to nKey
            // even if it's nullptr, so update time stamp at each time
            //
            // overflow will be handled
            m_CurrentTime++;

            // if linear cache is enabled
            // then first try to retrieve from cache
            //
            int nLCacheKey;
            int nLocationInLC;

            if(N != 0){

                nLCacheKey = LinearCacheKey(nKey);

                if(LocateInLinearCache(nLCacheKey, nKey, nLocationInLC)){
                    // find resource in LC, good!
                    //
                    // 1. move the record in LC to its head
                    m_LCache[nLCacheKey].SwapHead(nLocationInLC);

                    // 2. update access time stamp in LC *only*
                    //    *important*:
                    //    didn't update the access time in Cache!
                    std::get<1>(m_LCache[nLCacheKey].Head()) = m_CurrentTime;

                    // 3. push the access stamp at the end of the time stamp queue
                    m_TimeStampQ.push({m_CurrentTime, nKey});

                    // 4. return the resource pointer
                    return std::get<0>(m_LCache[nLCacheKey].Head());
                }
            }

            // there is no LCache or failed to find it in LCache
            // try to retrieve from m_Cache
            //
            auto pTextureInst = m_Cache.find(nKey);
            if(pTextureInst != m_Cache.end()){
                // we find it in m_Cache, OK...

                // 1. Put a record in LC if necessary
                //
                //    but when LC is full, insertion of a record causes to drop another
                //    old record, we need to update the resource access time w.r.t the 
                //    to-drop record, since whenever there is a record in LC, access time
                //    in m_Cache won't be updated.
                if(N != 0){

                    // since the old record is in LC, it must exist in m_Cache
                    // update its time stamp
                    //
                    if(m_LCache[nLCacheKey].Full()){
                        m_Cache[std::get<2>(m_LCache[
                                nLCacheKey].Back())].first = std::get<1>(m_LCache[nLCacheKey].Back());
                    }

                    // now insert the record to LC
                    m_LCache[nLCacheKey].PushHead({pTextureInst->second.second, m_CurrentTime, nKey});
                }

                // 2. set access time in m_Cache
                //
                pTextureInst->second.first = m_CurrentTime;

                // 3. push the access stamp at the end of the time stamp queue
                //
                m_TimeStampQ.push({m_CurrentTime, nKey});

                // 4. return the resource pointer
                return pTextureInst->second.second;
            }

            // it's not in m_Cache, too bad ...
            // we need to load it from the hard driver
            //
            auto pTexture = LoadTexture(nKey);

            // 1. Put a record in LC if necessary
            //    same here, we need to handle when LC is full in one box
            //
            if(N != 0){

                // since the old record is in LC, it must exist in m_Cache
                // update its time stamp
                //
                if(m_LCache[nLCacheKey].Full()){
                    m_Cache[std::get<2>(m_LCache[
                            nLCacheKey].Back())].first = std::get<1>(m_LCache[nLCacheKey].Back());
                }

                // now insert the record to LC
                m_LCache[nLCacheKey].PushHead({pTexture, m_CurrentTime, nKey});
            }

            // 2. put the resource in m_Cache
            //
            m_Cache[nKey] = {m_CurrentTime, pTexture};

            // 3. push the access stamp at the end of the time stamp queue
            //
            m_TimeStampQ.push({m_CurrentTime, nKey});

            // 4. reset the size of the cache
            // 
            m_ResourceCount++;
            Resize();

            // 5. return the resource pointer
            return pTexture;
        }

    private:

        // keep the size under m_ResourceMaxCount
        void Resize()
        {
            while(m_ResourceCount > m_ResourceMaxCount){
                // won't happen
                if(m_TimeStampQ.empty()){ break; }

                // may or may not release one resource
                ReleaseResource(std::get<1>(m_TimeStampQ.front()), std::get<0>(m_TimeStampQ.front())); 
                // always pop front by one after trid to release resource
                m_TimeStampQ.pop();
            }
        }

        // return int with value in [0, 255)
        //
        // TODO
        // design it for better performance
        inline int LinearCacheKey(uint32_t nKey)
        {
            // for PNGTexDB, just take last 8 bits
            return nKey & 0X000000FF;
        }


        int SignedHexStringDecode(const char *szStringCode)
        {
            const int knvStringHexChk[] = {
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  0,  0,
                0,  0,  0,  0,  0, 10, 11, 12, 13, 14, 15,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                0,  0,  0,  0,  0,  0,  0, 10, 11, 12, 13, 14, 15,  0,  0};

            int nRes = (szStringCode[0] == '0' ? 1 : -1)
                * (knvStringHexChk[1] * 256 + knvStringHexChk[2] * 16 + knvStringHexChk[3]);
            
            return nRes;
        }

        void Record(const zip_stat_t &stStatus)
        {
            // A(12345678)B(1234)C(1234).PNG
            //
            //     A:1~8 image index for actor
            //           uint32_t = 4 * 8: defined in actor.cpp
            //
            //     B:1   sgn(dx)
            //     B:2~4 abs(dx)
            //     C:1   sgn(dy)
            //     D:2~4 abs(dy)

            uint32_t nKey = StringHex(stStatus.name, 8);

            zip_uint64_t nSize;

            m_IndexCache[nKey] = {
                stStatus.index,
                stStatus.size,
                SignedHexStringDecode(stStatus.name +  8),
                SignedHexStringDecode(stStatus.name + 12)};
        }

        void PNGFileName(uint32_t nKey, char *szPNGName)
        {
            // only use 24 ~ 1 bits
            const uint16_t knvHexStringChunk[] = {
                12336, 12592, 12848, 13104, 13360, 13616, 13872, 14128,
                14384, 14640, 16688, 16944, 17200, 17456, 17712, 17968,
                12337, 12593, 12849, 13105, 13361, 13617, 13873, 14129,
                14385, 14641, 16689, 16945, 17201, 17457, 17713, 17969,
                12338, 12594, 12850, 13106, 13362, 13618, 13874, 14130,
                14386, 14642, 16690, 16946, 17202, 17458, 17714, 17970,
                12339, 12595, 12851, 13107, 13363, 13619, 13875, 14131,
                14387, 14643, 16691, 16947, 17203, 17459, 17715, 17971,
                12340, 12596, 12852, 13108, 13364, 13620, 13876, 14132,
                14388, 14644, 16692, 16948, 17204, 17460, 17716, 17972,
                12341, 12597, 12853, 13109, 13365, 13621, 13877, 14133,
                14389, 14645, 16693, 16949, 17205, 17461, 17717, 17973,
                12342, 12598, 12854, 13110, 13366, 13622, 13878, 14134,
                14390, 14646, 16694, 16950, 17206, 17462, 17718, 17974,
                12343, 12599, 12855, 13111, 13367, 13623, 13879, 14135,
                14391, 14647, 16695, 16951, 17207, 17463, 17719, 17975,
                12344, 12600, 12856, 13112, 13368, 13624, 13880, 14136,
                14392, 14648, 16696, 16952, 17208, 17464, 17720, 17976,
                12345, 12601, 12857, 13113, 13369, 13625, 13881, 14137,
                14393, 14649, 16697, 16953, 17209, 17465, 17721, 17977,
                12353, 12609, 12865, 13121, 13377, 13633, 13889, 14145,
                14401, 14657, 16705, 16961, 17217, 17473, 17729, 17985,
                12354, 12610, 12866, 13122, 13378, 13634, 13890, 14146,
                14402, 14658, 16706, 16962, 17218, 17474, 17730, 17986,
                12355, 12611, 12867, 13123, 13379, 13635, 13891, 14147,
                14403, 14659, 16707, 16963, 17219, 17475, 17731, 17987,
                12356, 12612, 12868, 13124, 13380, 13636, 13892, 14148,
                14404, 14660, 16708, 16964, 17220, 17476, 17732, 17988,
                12357, 12613, 12869, 13125, 13381, 13637, 13893, 14149,
                14405, 14661, 16709, 16965, 17221, 17477, 17733, 17989,
                12358, 12614, 12870, 13126, 13382, 13638, 13894, 14150,
                14406, 14662, 16710, 16966, 17222, 17478, 17734, 17990
            };

            *(uint16_t *)(szPNGName + 0) = knvHexStringChunk[(nKey & 0X000000FF) >>  0];
            *(uint16_t *)(szPNGName + 2) = knvHexStringChunk[(nKey & 0X000000FF) >>  8];
            *(uint16_t *)(szPNGName + 4) = knvHexStringChunk[(nKey & 0X000000FF) >> 16];

            szPNGName[ 6] =  '.';
            szPNGName[ 7] =  'P';
            szPNGName[ 8] =  'N';
            szPNGName[ 9] =  'G';
            szPNGName[10] = '\0';

            return szPNGName;
        }

        // load texture from zip archive
        SDL_Texture *LoadTexture(uint32_t nKey)
        {
            char szPNGName;
            PNGFileName(nKey, szPNGName);

            zip_stat_t stZIPStat;
            if(zip_stat(m_ZIP, szPNGName, ZIP_FL_ENC_RAW, &stZIPStat)){
                if(true
                        && stZIPStat.valid & ZIP_STAT_INDEX
                        && stZIPStat.valid & ZIP_STAT_SIZE){

                    auto fp = zip_fopen_index(m_ZIP, stZIPStat.index, 0);
                    if(fp == nullptr){ return nullptr; }

                    ExtendBuf(stZIPStat.size);

                    if(stZIPStat.size != zip_fread(fp, m_Buf, stZIPStat.size)){
                        zip_fclose(fp);
                        zip_close(m_ZIP); m_ZIP = nullptr;
                        return nullptr;
                    }

                    extern SDLDevice *g_SDLDevice;
                    return g_SDLDevice->CreateTexture(m_Buf, stZIPStat.size);
                }
            }
            return nullptr;
        }

        // when calling this function, the nKey
        //
        // 1. shouldn't be in linear cache
        //
        // 2. it must be in m_Cache, only exception is m_CurrentTime overflowed
        //      a) when there is no overflow, everytime when free a resource, we compare the time
        //         if the time in m_Cache != time with the record in the queue, means there is 
        //         accessing later, so we keep it. other wise we free it.
        //
        //      b) when there is overflow, think about this scenario:
        //         current node in time stamp queue is at time k, but the last accessing time of the
        //         resource is at (k + max) = k, then it's equal, and we release it incorrectly
        //         
        void ReleaseResourceByTimeStamp(uint32_t nKey, int nTimeStamp)
        {
            auto pTextureInst = m_Cache.find(nKey);
            // to handle the overflow problem
            // otherwise we don't need this check
            if(pTextureInst != m_Cache.end()){
                if(pTextureInst->second.first == nTimeStamp){
                    SDL_DestroyTexture(pTextureInst->second.second);
                    m_Cache.erase(pTextureInst);

                    m_ResourceCount--;
                }
            }
        }

        void ReleaseResource(uint32_t nKey, int nTimeStamp)
        {
            // first make sure that the resource is not in LC
            // return directly if yes
            if(N != 0){
                int nLocationInLC;
                if(LocateInLinearCache(nKey, nLocationInLC)){ return; }
            }

            // not in LC, release resource if (most likely) in the cache
            ReleaseResourceByTimeStamp(nKey, nTimeStamp);
        }


        bool LocateInLinearCache(int nLCKey, uint32_t nKey, int &nLocationInLC)
        {
            for(m_LCache[nLCKey].Reset(); !m_LCache[nLCKey].Done(); m_LCache[nLCKey].Forward()){
                if(std::get<2>(m_LCache[nLCKey].Current()) == nKey){
                    nLocationInLC =  m_LCache[nLCKey].CurrentIndex();
                    return true;
                }
            }
            return false;
        }

        void ExtendBuf(size_t nSize)
        {
            if(nSize > m_BufSize){
                delete m_Buf;
                m_Buf = new uint8_t[nSize];
                m_BufSize = nSize;
            }
        }

};
