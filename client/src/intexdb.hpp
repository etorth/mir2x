/*
 * =====================================================================================
 *
 *       Filename: intexdb.hpp
 *        Created: 02/26/2016 21:48:43
 *  Last Modified: 03/17/2016 19:06:08
 *
 *    Description: base of all Int->Tex map cache
 *                 this class load resources from a zip archieve
 *                 store it properly in a cache, additional linear cache is optional
 *
 *                 1. Load method depends
 *                 2. retrieve mehods differs
 *                 3. I'm also not 100% sure of my logic, so I added lot comments
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

template<typename T, size_t DeepN, size_t LenN, size_t ResM>
class IntexDB
{
    private:
        // linear cache
        std::array<CacheQueue<std::tuple<SDL_Texture *, unsigned long, T>, DeepN>, LenN> m_LCache;

        // main cache
        std::unordered_map<T, std::pair<unsigned long, SDL_Texture *>> m_Cache;

        // time stamp queue
        std::queue<std::tuple<unsigned long, T>>  m_TimeStampQ;

        size_t        m_ResourceMaxCount;
        size_t        m_ResourceCount;
        unsigned long m_CurrentTime;

    private:
        // libzip stuff
        //
        zip_t   *m_ZIP;
        size_t   m_BufSize;
        uint8_t *m_Buf;

    public:

        IntexDB()
            : m_ResourceMaxCount(ResM)
            , m_ResourceCount(0)
            , m_CurrentTime(0)
            , m_ZIP(nullptr)
            , m_BufSize(0)
            , m_Buf(nullptr)
        {
            static_assert(std::is_unsigned<T>::value,
                    "unsigned intergal type supported only please");
            static_assert(ResM > DeepN * LenN,
                    "maximal resource count must be larger than linear cache size please");
            static_assert(LenN < 8192,
                    "don't set linear cache size to be too large please");
            static_assert(DeepN < 16,
                    "don't set linear cache depth to be too large please");
        }

        ~IntexDB()
        {
            ClearLUT();
            delete m_Buf;

            if(m_ZIP){
                zip_close(m_ZIP);
            }
        }

    public:

        bool ValidZIP()
        {
            return (m_ZIP != nullptr);
        }

        bool LoadZIP(const char *szPNGTexDBName)
        {
            int nErrorCode;

            m_ZIP = zip_open(szPNGTexDBName, ZIP_RDONLY, &nErrorCode);
            return (m_ZIP != nullptr);
        }

        void ClearLUT()
        {
            if(UseLC()){
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

        bool UseLC() const
        {
            return LenN > 0 && DeepN > 0;
        }

        // internal retrieve function, for derived class use only
        // when retrieved successfully:
        //      when there is LC enabled, m_LCache[*pLCBucketIndex].Head() is the current result
        //      when no LC, nothing in nLCBucketIndex
        //
        // caller should define the LinearCacheKey() function
        SDL_Texture *InnRetrieve(T nKey,
                const std::function<size_t(T)> &fnLinearCacheKey,
                const std::function<zip_int64_t(T)> &fnZIPIndex, size_t *pLCBucketIndex)
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
            size_t nLCacheKey;
            size_t nLocationInLC;

            if(UseLC()){

                nLCacheKey = fnLinearCacheKey(nKey);

                if(LocateInLinearCache(nLCacheKey, nKey, nLocationInLC)){
                    // find resource in LC, good!
                    //
                    // 1. move the record in LC to its head
                    m_LCache[nLCacheKey].SwapHead(nLocationInLC);

                    // 2. update access time stamp in LC *only*
                    //    *important*:
                    //    didn't update the access time in hash map Cache!
                    std::get<1>(m_LCache[nLCacheKey].Head()) = m_CurrentTime;

                    // 3. push the access stamp at the end of the time stamp queue
                    m_TimeStampQ.emplace(m_CurrentTime, nKey);

                    // 4. return the bucket index
                    *pLCBucketIndex = nLocationInLC;

                    // 5. return the resource pointer
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
                if(UseLC()){

                    // since the old record is in LC, it must exist in m_Cache
                    // update its time stamp
                    //
                    if(m_LCache[nLCacheKey].Full()){
                        m_Cache[std::get<2>(m_LCache[nLCacheKey].Back())].first // in hashmap and LC
                            = std::get<1>(m_LCache[nLCacheKey].Back());         // update it's time stamp
                    }

                    // set pLCBucketIndex if necessary
                    *pLCBucketIndex = nLCacheKey;

                    // now insert the record to LC
                    // if it's full, the back() is overwrited as new head
                    //
                    m_LCache[nLCacheKey].PushHead(pTextureInst->second.second, m_CurrentTime, nKey);
                }

                // 2. set access time in m_Cache
                //
                pTextureInst->second.first = m_CurrentTime;

                // 3. push the access stamp at the end of the time stamp queue
                //
                m_TimeStampQ.emplace(m_CurrentTime, nKey);

                // 4. return the resource pointer
                return pTextureInst->second.second;
            }

            // it's not in m_Cache, too bad ...
            // we need to load it from the hard driver
            //

            zip_int64_t nZIPIndex = fnZIPIndex(nKey);
            SDL_Texture *pTexture = nullptr;
            if(nZIPIndex >= 0){
                pTexture = InnLoadTextureByZIPIndex((zip_uint64_t)nZIPIndex);
            }

            // 1. Put a record in LC if necessary
            //    same here, we need to handle when LC is full in one box
            //
            if(UseLC()){

                // since the old record is in LC, it must exist in m_Cache
                // update its time stamp
                //
                if(m_LCache[nLCacheKey].Full()){
                    m_Cache[std::get<2>(m_LCache[nLCacheKey].Back())].first
                        = std::get<1>(m_LCache[nLCacheKey].Back());
                }

                // set pLCBucketIndex if necessary
                *pLCBucketIndex = nLCacheKey;

                // now insert the record to LC
                m_LCache[nLCacheKey].PushHead(pTexture, m_CurrentTime, nKey);
            }

            // 2. put the resource in m_Cache
            //
            m_Cache[nKey] = {m_CurrentTime, pTexture};

            // 3. push the access stamp at the end of the time stamp queue
            //
            m_TimeStampQ.emplace(m_CurrentTime, nKey);

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

        // sizeof(T) is defined by declaration
        // nUseByteNum = 0: use all bytes
        // otherwise, use std::min(sizeof(T), nUseByteNum) from LSB
        //
        // 1. invocation should prepared enough buffer to szString
        // 2. no '\0' at the end, be careful

        template<size_t ByteN = 0> static const char *HexString(T nKey, char *szString)
        {
            const size_t nByteN = (ByteN) ? sizeof(T) : std::min(ByteN, sizeof(T));
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
                14406, 14662, 16710, 16966, 17222, 17478, 17734, 17990};

            for(size_t nIndex = 0; nIndex < nByteN; ++nIndex, (nKey >>= 8)){
                *(uint16_t *)(szString + 2 * (nByteN - nIndex - 1)) = knvHexStringChunk[(nKey & 0XFF)];
            }

            return szString;
        }

        // load texture from zip archive
        SDL_Texture *InnLoadTextureByZIPIndex(zip_uint64_t nIndex)
        {
            zip_stat_t stZIPStat;
            if(!zip_stat_index(m_ZIP, nIndex, ZIP_FL_ENC_RAW, &stZIPStat)){
                if(true
                        && stZIPStat.valid & ZIP_STAT_INDEX
                        && stZIPStat.valid & ZIP_STAT_SIZE){

                    auto fp = zip_fopen_index(m_ZIP, stZIPStat.index, 0);
                    if(fp == nullptr){ return nullptr; }

                    ExtendBuf((size_t)stZIPStat.size);

                    if(stZIPStat.size != (zip_uint64_t)zip_fread(fp, m_Buf, stZIPStat.size)){
                        zip_fclose(fp);
                        zip_close(m_ZIP); m_ZIP = nullptr;
                        return nullptr;
                    }

                    extern SDLDevice *g_SDLDevice;
                    return g_SDLDevice->CreateTexture((const uint8_t *)m_Buf, (size_t)stZIPStat.size);
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
        void ReleaseResourceByTimeStamp(T nKey, unsigned long nTimeStamp)
        {
            auto pTextureInst = m_Cache.find(nKey);
            // to handle the overflow problem, otherwise we don't need this check
            if(pTextureInst != m_Cache.end()){
                if(pTextureInst->second.first == nTimeStamp){
                    SDL_DestroyTexture(pTextureInst->second.second);
                    m_Cache.erase(pTextureInst);

                    m_ResourceCount--;
                }
            }
        }

        void ReleaseResource(T nKey,
                unsigned long nTimeStamp, const std::function<int(T)> &fnLinearCacheKey)
        {
            // first make sure that the resource is not in LC
            // return directly if yes
            if(DeepN != 0 && LenN != 0){
                int nLocationInLC;
                if(LocateInLinearCache(fnLinearCacheKey(nKey), nKey, &nLocationInLC)){ return; }
            }

            // not in LC, release resource if (most likely) in the cache
            ReleaseResourceByTimeStamp(nKey, nTimeStamp);
        }


        // assume everything is ok, parameters should be checked before invocation
        bool LocateInLinearCache(int nLCKey, T nKey, size_t *pLocationInLC)
        {
            for(m_LCache[nLCKey].Reset(); !m_LCache[nLCKey].Done(); m_LCache[nLCKey].Forward()){
                if(std::get<2>(m_LCache[nLCKey].Current()) == nKey){
                    *pLocationInLC = m_LCache[nLCKey].Index();
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
