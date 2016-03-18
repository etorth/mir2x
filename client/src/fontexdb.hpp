/*
 * =====================================================================================
 *
 *       Filename: fontexdb.hpp
 *        Created: 02/24/2016 17:51:16
 *  Last Modified: 03/18/2016 15:15:18
 *
 *    Description: this class only releases resource automatically
 *                 on loading new resources
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


enum FontStyle: uint8_t{
    FONTSTYLE_BOLD          = 0B0000'0001;
    FONTSTYLE_ITALIC        = 0B0000'0010;
    FONTSTYLE_UNDERLINE     = 0B0000'0100;
    FONTSTYLE_STRIKETHROUGH = 0B0000'1000;
    FONTSTYLE_SOLID         = 0B0001'0000;
    FONTSTYLE_SHADED        = 0B0010'0000;
    FONTSTYLE_BLENDED       = 0B0100'0000;
};

typedef struct{
    SDL_Texture *Texture;
}FontexItem;

template<int N = 4>
class FontexDB final
{
    private:
        // linear cache
        std::array<256,
            QueueN<N, std::tuple<
                SDL_Texture *, int, uint32_t, uint32_t>>> m_LCache;

        // main cache
        std::unordered_map<uint32_t,
            std::unordered_map<uint32_t, SDL_Texture *>> m_Cache;

        // time stamp queue
        std::queue<std::tuple<int, uint32_t, uint32_t>>  m_TimeStampQ;

        // font cache, no size control
        std::unordered_map<uint16_t, TTF_Font *> m_FontCache;

        int m_ResourceMaxCount;
        int m_ResourceCount;

    public:

    public:
        FontxDB()
            : m_ResourceMaxCount(N * 256 * 4)
              , m_ResourceCount(0)
    {
    }

        ~FontxDB()
        {
        }

    public:
        bool Load(const std::array<256, std::string> &);

    public:

        void Resize(int nMaxSize)
        {
            m_ResourceMaxCount = std::max(N * 256 * 4, nMaxSize);
            while(m_ResourceCount > m_ResourceMaxCount){
                if(m_TimeStampQ.empty()){
                    // this won't happen
                    // put it here for safity
                    break;
                }

                uint32_t nFontFaceKey = std::get<1>(m_TimeStampQ.front());
                uint32_t nUTF8Code    = std::get<2>(m_TimeStampQ.front());

                int     nTimeStamp = std::get<0>(m_TimeStampQ.front());
                uint8_t nLCKey     = LinearCacheKey(nFontFaceKey, nUTF8Code);

                // may or may not release one resource
                ReleaseResource(nLCKey, nFontFaceKey, nUTF8Code, nTimeStamp);
            }
        }

    public:

        SDL_Texture *Retrieve(uint8_t nFileIndex, uint8_t nSize, uint8_t nStyle, uint32_t nUTF8Code)
        {
            // everytime when call this function
            // there must be a pointer retrieved responding to (nFontFaceKey, nUTF8Code)
            // even if it's nullptr, so update time stamp at each time
            //
            // overflow will be handled
            m_CurrentTime++;

            uint32_t nFontFaceKey = ((uint32_t)nFileIndex << 16) + ((uint32_t)nSize << 8) + nStyle;
            uint8_t  nLCacheKey   = LinearCacheKey(nFontFaceKey, nUTF8Code);

            int nLocationInLC;

            if(LocateInLinearCache(nLCacheKey, nFontFaceKey, nUTF8Code, nLocationInLC)){
                // find resource in LC, good!
                //
                // 1. move the record in LC to its head
                m_LCache[nLCacheKey].SwapHead(nLocationInLC);

                // 2. update access time stamp in LC *only*
                //    *important*:
                //    didn't update the access time in Cache!
                std::get<1>(m_LCache[nLCacheKey].Head()) = m_CurrentTime;

                // 3. push the access stamp at the end of the time stamp queue
                m_TimeStampQ.push({m_CurrentTime, nFontFaceKey, nUTF8Code});

                // 4. return the resource pointer
                return std::get<0>(m_LCache[nLCacheKey].Head());
            }else{
                // didn't find it in LC, try to find it in m_Cache
                auto pFontFaceInst = m_Cache.find(nFontFaceKey);
                if(pFontFaceInst == m_Cache.end()){
                    // ok allocate the memory here
                    m_Cache[nFontFaceKey] = {};
                    // just in case of re-hash
                    pFontFaceInst = m_Cache.find(nFontFaceKey);
                }

                auto pTextureInst = pFontFaceInst.second.find(nUTF8Code);
                if(pTextureInst != pFontFaceInst.second.end()){
                    // we find it in m_Cache, OK...
                    //
                    // 1. Put a record in LC
                    //
                    //    but when LC is full, insertion of a record causes to drop another
                    //    old record, we need to update the resource access time w.r.t the 
                    //    to-drop record, since whenever there is a record in LC, access time
                    //    in m_Cache won't be updated.
                    if(m_LCache[nLCacheKey].Full()){
                        // since it's in LC, it must exist in m_Cache
                        int      nOldTimeStamp   = std::get<1>(m_LCache[nLCacheKey].Back());
                        uint32_t nOldFontFaceKey = std::get<2>(m_LCache[nLCacheKey].Back());
                        uint32_t nOldUTF8Key     = std::get<3>(m_LCache[nLCacheKey].Back());

                        m_Cache[nOldFontFaceKey][nOldUTF8Key].first = nOldTimeStamp;
                    }

                    // now insert the record to LC
                    m_LCache[nLCacheKey].PushHead(
                            {pTextureInst.second.second, m_CurrentTime, nFontFaceKey, nUTF8Code});

                    // 2. set access time in m_Cache
                    //
                    pTextureInst.second.first = m_CurrentTime;

                    // 3. push the access stamp at the end of the time stamp queue
                    //
                    m_TimeStampQ.push({m_CurrentTime, nFontFaceKey, nUTF8Code});

                    // 4. return the resource pointer
                    return pTexture.second.second;

                }else{

                    // it's not in m_Cache, too bad ...
                    // so externally load is required
                    //
                    auto pTexture = LoadTexture(nFileIndex, nSize, nStyle, nUTF8Code);

                    // 1. Put a record in LC
                    //    same here, we need to handle when LC is full in one box
                    if(m_LCache[nLCacheKey].Full()){
                        // since it's in LC, it must exist in m_Cache
                        int      nOldTimeStamp   = std::get<1>(m_LCache[nLCacheKey].Back());
                        uint32_t nOldFontFaceKey = std::get<2>(m_LCache[nLCacheKey].Back());
                        uint32_t nOldUTF8Key     = std::get<3>(m_LCache[nLCacheKey].Back());

                        m_Cache[nOldFontFaceKey][nOldUTF8Key].first = nOldTimeStamp;
                    }

                    // now insert the record to LC
                    //
                    m_LCache[nLCacheKey].PushHead({pTexture, m_CurrentTime, nFontFaceKey, nUTF8Code});

                    // 2. put the resource in m_Cache
                    //
                    pFontFaceInst.second[nUTF8Code] = {m_CurrentTime, pTexture};

                    // 3. push the access stamp at the end of the time stamp queue
                    //
                    m_TimeStampQ.push({m_CurrentTime, nFontFaceKey, nUTF8Code});

                    // 4. reset the size of the cache
                    // 
                    m_ReourceCount++;
                    Resize(m_ResourceMaxCount);

                    // 5. return the resource pointer
                    return pTexture;
                }
            }
        }

    private:

        // TODO
        // design it for better performance
        uint8_t LinearCacheKey(uint32_t nFontFaceKey, uint32_t nUTF8Code)
        {
            return (uint8_t)(nFontFaceKey + nUTF8Code);
        }

        TTF_Font *RetrieveFont(uint8_t nFileIndex, uint8_t nSize)
        {
            // we don't control # of TTF_Font
            // since it won't be a problem
            uint16_t nFontCode = ((uint16_t)nFileIndex << 8) + nSize;
            auto pFontInst = m_FontCache.find(nFontCode);
            if(pFontInst != m_FontCache.end()){
                // pFontInst.second may be nullptr
                // which means we tried to load it previously but faile
                // this helps to prevent trying to load it everytime
                return pFontInst.second;
            }

            auto pFont = TTF_OpenFont(m_FontFileNameV[nFileIndex].c_str(), nSize);
            m_FontCache[nFontCode] = pFont;
            return pFont;
        }

        SDL_Texture *LoadTexture(uint8_t nFileIndex, uint8_t nSize, uint8_t nStyle, uint32_t nUTF8Code)
        {
            auto *pFont = RetrieveFont(nFileIndex, nSize);
            if(pFont == nullptr){ return nullptr; }

            TTF_SetFontKerning(pFont, false);

            int nFontStyle = 0;
            if(nStyle & FONTSTYLE_BOLD){
                nFontStyle &= TTF_STYLE_BOLD;
            }

            if(nStyle & FONTSTYLE_ITALIC){
                nFontStyle &= TTF_STYLE_ITALIC;
            }

            if(nStyle & FONTSTYLE_UNDERLINE){
                nFontStyle &= TTF_STYLE_UNDERLINE;
            }

            if(nStyle & FONTSTYLE_STRIKETHROUGH){
                nFontStyle &= TTF_STYLE_STRIKETHROUGH;
            }

            TTF_SetFontStyle(pFont, nFontStyle);

            SDL_Surface *pSurface = nullptr;
            char szUTF8[8];

            *((uint32_t *)szUTF8) = nUTF8Code;
            szUTF8[4] = 0;

            if(nStyle & FONTSTYLE_SOLID){
                pSurface = TTF_RenderUTF8_Solid(pFont, szUTF8, {0XFF, 0XFF, 0XFF, 0XFF});
            }else if(stCharIdtor.FontInfo.Style & FONTSTYLE_SHADED){
                pSurface = TTF_RenderUTF8_Shaded(pFont, szUTF8, {0XFF, 0XFF, 0XFF, 0XFF}, {0X00, 0X00, 0X00, 0X00});
            }else{
                // blended is by default by of lowest priority
                // means if we really need to set SOLID/SHADOWED/BLENDED
                // most likely we don't want the default setting
                pSurface = TTF_RenderUTF8_Blended(pFont, szUTF8, {0XFF, 0XFF, 0XFF, 0XFF});
            }

            if(pSurface){
                SDL_Texture *pTexture = 
                    SDL_CreateTextureFromSurface(GetDeviceManager()->GetRenderer(), pSurface);
                SDL_FreeSurface(pSurface);
                return pTexture;
            }
            return nullptr;
        }

        // when calling this function, (nFontFaceKey, nUTF8Code):
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
        void ReleaseResourceByTimeStamp(uint32_t nFontFaceKey, uint32_t nUTF8Code, int nTimeStamp)
        {
            auto pFontFaceInst = m_Cache.find(nFontFaceKey);
            if(pFontFaceInst != m_Cache.end()){
                auto pTextureInst = pFontFaceInst.second.find(nUTF8Code);
                if(pTextureInst != pFontFaceInst.second.end()){
                    if(pTextureInst.second.first == nTimeStamp){
                        SDL_DestroyTexture(pTextureInst.second.second);
                        pFontFaceInst.erase(pTextureInst);

                        m_ResourceCount--;
                    }
                }
            }
        }

        void ReleaseResource(uint8_t nLCKey, uint32_t nFontFaceKey, uint32_t nUTF8Code, int nTimeStamp)
        {
            // first check whether the resource is in LC
            // return directly if yes since we always keep resrouce in LC
            int nLocationInLC;
            if(LocateInLinearCache(nLCKey, nFontFaceKey, nUTF8Code, nLocationInLC)){ return; }

            // not in LC, release resource if (most likely) in the cache
            ReleaseResourceByTimeStamp(nFontFaceKey, nUTF8Code, nTimeStamp);
        }


        bool LocateInLinearCache(uint8_t nLCKey,
                uint32_t nFontFaceKey, uint32_t nUTF8Code, int &nLocationInLC)
        {
            for(m_LCache[nLCKey].Reset(); !m_LCache[nLCKey].Done(); m_LCache[nLCKey].Forward()){
                if(std::get<1>(m_LCache[nLCKey].Current()) == nFontFaceKey
                        && std::get<2>(m_LCache[nLCKey].Current()) == nUTF8Code){
                    nLocationInLC =  m_LCache[nLCKey].CurrentIndex();
                    return true;
                }
            }
            return false;
        }
};



template<size_t LCDeepN, size_t LCLenN, size_t ResMaxN>
class FontexDB: public InresDB<uint32_t, PNGTexOffItem, LCDeepN, LCLenN, ResMaxN>
{
    private:
        size_t   m_BufSize;
        uint8_t *m_Buf;
        zip_t   *m_ZIP;

    private:
        typedef struct{
            zip_uint64_t Index;
            size_t       Size;
            // this is the data buffer for the font file in the ZIP
            // but we didn't allocate it when in Load(), instead
            // we load it if LoadResource() need it
            uint8_t     *Data;
            // this filed means we tried to load this ttf or not
            // to prevent load many times
            //
            // for PNGTexDB we don't need it since when return nullptr
            // it'll prevent load again
            //
            // but here one ttf contains many fontex, one null fontex
            // can prevent others to load again
            int          Tried;
        }ZIPItemInfo;

    private:
        std::unordered_map<uint32_t, ZIPItemInfo> m_ZIPItemInfoCache;

    public:
        FontexDB()
            : InresDB<uint32_t, PNGTexOffItem, LCDeepN, LCLenN, ResMaxN>()
              , m_BufSize(0)
              , m_Buf(nullptr)
              , m_ZIP(nullptr)
              , m_ZIPItemInfoCache()
    {}
        virtual ~FontexDB() = default;

    public:
        bool Valid()
        {
            return m_ZIP && !m_ZIPItemInfoCache.empty();
        }

        bool Load(const char *szFontexDBName)
        {
            int nErrorCode;
            m_ZIP = zip_open(szFontexDBName, ZIP_RDONLY, &nErrorCode);
            if(m_ZIP == nullptr){ return false; }

            zip_int64_t nCount = zip_get_num_entries(m_ZIP, ZIP_FL_UNCHANGED);
            if(nCount > 0){
                for(zip_uint64_t nIndex = 0; nIndex < (zip_uint64_t)nCount; ++nIndex){
                    zip_stat_t stZIPStat;
                    if(!zip_stat_index(m_ZIP, nIndex, ZIP_FL_ENC_RAW, &stZIPStat)){
                        if(true
                                && stZIPStat.valid & ZIP_STAT_INDEX
                                && stZIPStat.valid & ZIP_STAT_SIZE
                                && stZIPStat.valid & ZIP_STAT_NAME){
                            // file name inside is pretty simple
                            // just like
                            //
                            // 00.TTF
                            // 01.TTF
                            // ...
                            //
                            // FE.TTF
                            // FF.TTF

                            uint8_t nKey = StringHex<uint8_t, 1>(stZIPStat.name);
                            m_ZIPItemInfoCache[nKey] = {
                                stZIPStat.index, (size_t)stZIPStat.size, nullptr, 0};
                        }
                    }
                }
            }
            return Valid();
        }

    public:
        void RetrieveItem(uint32_t nKey, PNGTexOffItem *pItem,
                const std::function<size_t(uint32_t)> &fnLinearCacheKey)
        {
            // fnLinearCacheKey should be defined with LCLenN definition
            if(pItem){
                // InnRetrieve always return true;
                this->InnRetrieve(nKey, pItem, fnLinearCacheKey, nullptr);
            }
        }

        void ExtendBuf(size_t nSize)
        {
            if(nSize > m_BufSize){
                delete m_Buf;
                m_Buf = new uint8_t[nSize];
                m_BufSize = nSize;
            }
        }

    public:
        // for all pure virtual function required in class InresDB;
        //
        // if we need to load, means we need to find the font file handler
        // and don't care the speed
        // so we can put additional like font-setter or something here
        virtual FontexItem LoadResource(uint64_t nKey)
        {
            // null resource desc
            FontexItem stItem {nullptr};

            uint16_t nSizedFontIndex = ((nKey & 0X00FFFF0000000000) >> 40);
            uint8_t  nPointSize      = ((nKey & 0X0000FF0000000000) >> 40);
            uint8_t  nFontIndex      = ((nKey & 0X00FF000000000000) >> 48);

            auto pZIPIndexInst = m_ZIPItemInfoCache.find(nFontIndex);
            if(pZIPIndexInst == m_ZIPItemInfoCache.end()){
                // no FontIndex supported in the DB
                // just return
                return stItem;
            }

            if(!pZIPIndexInst->second.Data){
                if(pZIPIndexInst->second.Tried){
                    // ooops, can't help..
                    return stItem;
                }


                // first time, ok
                // 1. mark it as tried, any failure will prevent following loading
                pZIPIndexInst->second.Tried = 1;

                // 2. open the ttf in the zip
                auto pf = zip_fopen_index(m_ZIP, stZIPStat.index, ZIP_FL_UNCHANGED);
                if(pf){
                    return stItem;
                }

                // 3. allocate new buffer for the ttf file
                pZIPIndexInst->second.Data = new uint8_t[pZIPIndexInst->second.Size];

                // 4. read ttf file from zip archive
                auto nReadSize = (size_t)zip_fread(fp,
                        pZIPIndexInst->second.Data, pZIPIndexInst->second.Size);

                // 5. close the file handler anyway
                zip_fclose(fp);

                // 6. ran into failure, then free the buffer
                if(nReadSize != pZIPIndexInst->second.Size){
                    delete pZIPIndexInst->second.Data;
                    pZIPIndexInst->second.Data = nullptr;
                }

                // 7. eventually we are done, now the buffer is for ttf file data
                //    we can use it to create SizedTTF

                extern SDLDevice *g_SDLDevice;
                m_ZIPItemInfoCache[nSizedFontIndex] = 
                    g_SDLDevice->CreateTTF((const uint8_t *)m_Buf, nSize, nFontSize);
            }

            auto pZIPIndexInst = m_ZIPItemInfoCache.find(nSizedFontIndex);
            if(pZIPIndexInst == m_ZIPItemInfoCache.end()){



                extern SDLDevice *g_SDLDevice;
                m_ZIPItemInfoCache[nSizedFontIndex] = 
                    g_SDLDevice->CreateTTF((const uint8_t *)m_Buf, nSize, nFontSize);
            }




            stItem.DX = pZIPIndexInst->second.DX;
            stItem.DY = pZIPIndexInst->second.DY;


            stItem.Texture = g_SDLDevice->CreateTexture((const uint8_t *)m_Buf, nSize);

            return stItem;
        }

        void FreeResource(FontexItem &stItem)
        {
            if(stItem.Texture){
                SDL_DestroyTexture(stItem.Texture);
            }
        }
};
