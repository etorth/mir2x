/*
 * =====================================================================================
 *
 *       Filename: fontexdb.hpp
 *        Created: 02/24/2016 17:51:16
 *  Last Modified: 03/18/2016 15:51:29
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

template<size_t LCDeepN, size_t LCLenN, size_t ResMaxN>
class FontexDB: public InresDB<uint32_t, FontexItem, LCDeepN, LCLenN, ResMaxN>
{
    private:
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
            : InresDB<uint32_t, FontexItem, LCDeepN, LCLenN, ResMaxN>()
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
        void RetrieveItem(uint32_t nKey, FontexItem *pItem,
                const std::function<size_t(uint32_t)> &fnLinearCacheKey)
        {
            // fnLinearCacheKey should be defined with LCLenN definition
            if(pItem){
                // InnRetrieve always return true;
                this->InnRetrieve(nKey, pItem, fnLinearCacheKey, nullptr);
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
            uint8_t  nFontIndex      = ((nKey & 0X00FF000000000000) >> 48);
            uint8_t  nPointSize      = ((nKey & 0X0000FF0000000000) >> 40);
            uint8_t  nFontStyle      = ((nKey & 0X000000FF00000000) >> 32);
            uint32_t nUTF8Code       = ((nKey & 0X00000000FFFFFFFF) >>  0);

            auto pZIPIndexInst = m_ZIPItemInfoCache.find(nFontIndex);
            if(pZIPIndexInst == m_ZIPItemInfoCache.end()){
                // no FontIndex supported in the DB
                // just return
                return stItem;
            }

            // supported FontIndex, try find SizedFont in the cache
            auto pSizeFontInst = m_SizedFontCache.find(nSizedFontIndex);
            if(pSizeFontInst == m_SizedFontCache.end()){
                // didn't find it
                // 1. may be we didn't load it yet
                // 2. previously loading ran into failure
                if(!pZIPIndexInst->second.Data){
                    if(pZIPIndexInst->second.Tried){
                        // ooops, can't help..
                        return stItem;
                    }

                    // didn't try yet
                    // so firstly load the ttf data

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
                        return stItem;
                    }

                    // 7. eventually we are done, now the buffer is for ttf file data
                    //    we can use it to create SizedTTF
                }

                // now the data buffer is well prepared
                extern SDLDevice *g_SDLDevice;
                m_SizedFontCache[nSizedFontIndex] = 
                    g_SDLDevice->CreateTTF((const uint8_t *)m_Buf, nSize, nFontSize);

                pSizeFontInst = m_SizedFontCache.find(nSizedFontIndex);
            }

            // now pSizeFontInst is well prepared
            auto pFont = pSizeFontInst->second;
            TTF_SetFontKerning(pFont, false);

            // set font style if necessary
            if(nFontStyle){
                int nTTFontStyle = 0;
                if(nFontStyle & FONTSTYLE_BOLD){
                    nTTFontStyle &= TTF_STYLE_BOLD;
                }

                if(nFontStyle & FONTSTYLE_ITALIC){
                    nTTFontStyle &= TTF_STYLE_ITALIC;
                }

                if(nFontStyle & FONTSTYLE_UNDERLINE){
                    nTTFontStyle &= TTF_STYLE_UNDERLINE;
                }

                if(nFontStyle & FONTSTYLE_STRIKETHROUGH){
                    nTTFontStyle &= TTF_STYLE_STRIKETHROUGH;
                }

                TTF_SetFontStyle(pFont, nTTFontStyle);
            }

            SDL_Surface *pSurface = nullptr;
            char szUTF8[8];

            *((uint32_t *)szUTF8) = nUTF8Code;
            szUTF8[4] = 0;

            if(nFontStyle & FONTSTYLE_SOLID){
                pSurface = TTF_RenderUTF8_Solid(pFont, szUTF8, {0XFF, 0XFF, 0XFF, 0XFF});
            }else if(nFontStyle & FONTSTYLE_SHADED){
                pSurface = TTF_RenderUTF8_Shaded(
                        pFont, szUTF8, {0XFF, 0XFF, 0XFF, 0XFF}, {0X00, 0X00, 0X00, 0X00});
            }else{
                // blended is by default by of lowest priority
                // means if we really need to set SOLID/SHADOWED/BLENDED
                // most likely we don't want the default setting
                pSurface = TTF_RenderUTF8_Blended(pFont, szUTF8, {0XFF, 0XFF, 0XFF, 0XFF});
            }

            if(pSurface){
                extern SDLDevice *g_SDLDevice;
                // TODO
                //
                // TBD
                // whethere make a SDLDevice::CreateTextureFromSurface() or
                // make a SDLDevice::CreateTextureFont(TTF_Font, UTF8Char)?
                //
                // 1. we want all data passed to SDLDevice to be builtin
                //    but both options are impossible
                //
                // 2. we can have CreateTextureFont(pSurface->Data, ...)
                //    but we need to handle endian by ourself
                //
                // 3. FONTSTYLE_XXX is defined in FontexDB, if we make this
                //    creation inside of SDLDevice, then SDLDevice need to 
                //    include this header file
                stItem.Texture = g_SDLDevice->CreateTextureFromSurface(pSurface);
                SDL_FreeSurface(pSurface);
            }

            return stItem;
        }

        void FreeResource(FontexItem &stItem)
        {
            if(stItem.Texture){
                SDL_DestroyTexture(stItem.Texture);
            }
        }
};
