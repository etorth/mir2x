#pragma once

#include <SDL.h>
#include <vector>
#include <tinyxml2.h>
#include <functional>
#include "section.hpp"
#include "tokenbox.hpp"
#include "devicemanager.hpp"
#include "texturemanager.hpp"

// SECTIONINFO      : static data
// SECTIONSTATE     : dynamic data

class TokenBoard
{
    public:
        TokenBoard(int, bool);
        ~TokenBoard() = default;
    private:
        bool              m_ShrinkageWidth;
    private:
        bool              m_HasEventText;
    public:
        int InSection(int, int);

    public:
        // bool    Add(const TOKENBOX &, int);
        bool    Add(TOKENBOX &, int);
        void    Clear();
        void    Update(Uint32);
        bool    Empty();
        int     MaxHeight();

    private:
        void GetTokenBoxMetrics(TOKENBOX &, int &, int &, int &, int &);
    private:
        void LoadUTF8CharBoxCache(TOKENBOX &, int);
        void LoadEmoticonCache(TOKENBOX &, int);

    private:
        int  DoPadding(int, int);
        bool InRect(int, int, int, int, int, int);
    private:
        void SetTokenBoxStartX(std::vector<TOKENBOX> &);
        void SetTokenBoxStartY(std::vector<TOKENBOX> &, int);

    public:
        int     SpacePadding(int);

    private:
        int m_MaxH1;
        int m_MaxH2;
        int m_CurrentWidth;
        int m_CurrentLineMaxH2;
        int          m_PW;
        int          m_H;
        int          m_W;
    private:
        void UpdateSection(SECTION &, Uint32);
    public:
        void AddEventHandler(const char *, std::function<void()>);

    public:
        int W();
        int H();

    private:
        const tinyxml2::XMLElement *NextObject(const tinyxml2::XMLElement *);
    private:
        bool IntervalOverlapped(int, int, int, int);
        void AddNewTokenBoxLine(bool);

    private:
        int  TokenBoxType(const TOKENBOX &);
    public:
        bool Load(const tinyxml2::XMLDocument &);

    private:
        bool ParseXMLContent(const tinyxml2::XMLElement *);

    private:
        bool ParseEmoticonObject(const tinyxml2::XMLElement *, int);
        bool ParseTextObject(const tinyxml2::XMLElement *, int);
        bool ParseEventTextObject(const tinyxml2::XMLElement *, int);

    private:
        SDL_Color   GetFontColor(const tinyxml2::XMLElement *);
        int         GetFontIndex(const tinyxml2::XMLElement *);
        int         GetFontStyle(const tinyxml2::XMLElement *);
        int         GetFontSize (const tinyxml2::XMLElement *);

    private:
        SDL_Color   GetTextCharColor(const tinyxml2::XMLElement *);
        SDL_Color   GetEventTextCharColor(const tinyxml2::XMLElement *, int);

    private:
        void        ParseTextContentSection(const tinyxml2::XMLElement *, int);


    private:
        int         GetEmoticonSet  (const tinyxml2::XMLElement *);
        int         GetEmoticonIndex(const tinyxml2::XMLElement *);
    private:
        FONTINFO    ParseFontInfo(const tinyxml2::XMLElement *);

    private:
        void        ParseTextObjectAttribute(const tinyxml2::XMLElement*, int);
        void        ParseEventTextObjectAttribute(const tinyxml2::XMLElement*, int);
        void        ParseEmoticonObjectAttribute(const tinyxml2::XMLElement*, int);

    private:
        int     m_LineWidth;

    public:
        bool ObjectEmocticon(const tinyxml2::XMLElement *);
        bool ObjectReturn(const tinyxml2::XMLElement *);
        bool ObjectText(const tinyxml2::XMLElement *);
        bool ObjectEventText(const tinyxml2::XMLElement *);


    private:
        void RedrawToken(int, int, TOKENBOX &, bool);
        bool DrawTextureCache();

    public:
        void Draw(int, int);

    private:
        // bool AddNewTokenBox(const TOKENBOX &, int);
        bool AddNewTokenBox(TOKENBOX &, int);

    public:

    private:
        int GetNthLineIntervalMaxH2(int, int, int);
        int GetNthLineTokenBoxStartY(int, int, int, int);
        int GetNthNewLineStartY(int);

        bool GetTokenBoxLocation(int, int, int &, int &, int &, int &);
        bool GetTokenBoxStartPoint(int, int, int &, int &);
        void SetDrawToTextureCache();
        void SetDrawToScreen();

    private:
        void ClearCurrentLine();

    private:

    private:
        SECTION    m_CurrentSection;

    public:
        bool HandleEvent(int, int, const SDL_Event &);

    private:
        SDL_Texture    *m_TextureCache;

    public:
        int Width();
        int Height();
    private:
        void RedrawSection(int);

    private:
        bool m_SetDrawToTextureCache;
    private:
        std::vector<std::vector<TOKENBOX>> m_LineV;
        std::vector<TOKENBOX>              m_CurrentLine;
        std::vector<SECTION>               m_SectionV;
        std::vector<int>                   m_LineStartY;
    private:
        std::vector<std::vector<std::vector<const TOKENBOX *>>> m_TokenBoxBitmap;

    private:
        void ResetCurrentLine();

    private:
        void MakeEventTextBitmap();
        void MarkEventTextBitmap(TOKENBOX &);
        int m_Resolution;
    private:
        int SectionTypeCount(const std::vector<TOKENBOX> &, int);

        void DrawRectLine(const SDL_Rect &);

    public:
};
