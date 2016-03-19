#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <tinyxml2.h>
#include <functional>
#include "section.hpp"
#include "tokenbox.hpp"
#include "widget.hpp"
#include <unordered_map>

// SECTIONINFO      : static data
// SECTIONSTATE     : dynamic data

class TokenBoard: public Widget
{
    public:
        TokenBoard(bool,    // wrap or not
                int,        // maximal width
                int,        // minimal text box margin
                int);       // minimal text box line space
        ~TokenBoard() = default;

    private:
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
        int     m_MaxH1;
        int     m_MaxH2;
        int     m_CurrentLineMaxH2;
        int     m_PW;
        int     m_W;
        int     m_H;
        bool    m_Wrap;
        int     m_CurrentWidth;
        bool    m_SkipEvent;


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
        // load the content and it's callback table
        bool Load(const tinyxml2::XMLDocument &,
                const std::unordered_map<std::string, std::function<void()>> &);

    private:
        bool ParseXMLContent(const tinyxml2::XMLElement *);

    private:
        bool ParseEmoticonObject(const tinyxml2::XMLElement &,
                int, const std::unordered_map<std::string, std::function<void()>> &);
        bool ParseEventTextObject(const tinyxml2::XMLElement &,
                int, const std::unordered_map<std::string, std::function<void()>> &);

    private:
        SDL_Color   GetFontColor(const tinyxml2::XMLElement &);
        int         GetFontIndex(const tinyxml2::XMLElement &);
        int         GetFontStyle(const tinyxml2::XMLElement &);
        int         GetFontSize (const tinyxml2::XMLElement &);

    private:
        SDL_Color   GetEventTextCharColor(const tinyxml2::XMLElement &);
        SDL_Color   GetEventTextCharColor(const tinyxml2::XMLElement &, int);

    private:
        void        ParseTextContentSection(const tinyxml2::XMLElement *, int);

    private:
        int         GetEmoticonSet  (const tinyxml2::XMLElement &);
        int         GetEmoticonIndex(const tinyxml2::XMLElement &);

    private:
        int     m_LineWidth;

    public:
        bool ObjectEmocticon(const tinyxml2::XMLElement &);
        bool ObjectReturn(const tinyxml2::XMLElement &);
        bool ObjectEventText(const tinyxml2::XMLElement &);


    private:
        void RedrawToken(int, int, TOKENBOX &, bool);
        bool DrawTextureCache();

    public:
        void Draw(int, int);
        void DrawEx(int, int, int, int, int, int);


    private:
        // bool AddNewTokenBox(const TOKENBOX &, int);
        bool AddNewTokenBox(TOKENBOX &);

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

    public:
        bool HandleEvent(int, int, const SDL_Event &);

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
        int  m_Resolution;
        int  m_MinTextMargin;
        int  m_MinTextLineSpace;

    private:
        // callbacks
        std::vector<std::function<void()>> m_IDFuncV;

    private:
        int SectionTypeCount(const std::vector<TOKENBOX> &, int);

        void DrawRectLine(const SDL_Rect &);

    public:
        const std::string &ContentExport();
};
