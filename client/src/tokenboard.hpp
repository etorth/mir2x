/*
 * =====================================================================================
 *
 *       Filename: tokenboard.hpp
 *        Created: 06/17/2015 10:24:27 PM
 *  Last Modified: 07/11/2017 23:39:52
 *
 *    Description: Design TBD.
 *
 *                 For scenario we need text-emoticon mixed boards:
 *
 *                 1. button like text terminal. which we only need to support emoticon
 *                    and eventtext, eventtext accept button click event only. This is
 *                    the simple case. no select/edit support needed.
 *
 *                 2. chatbox, two parts:
 *                      a. already sent chat messages.
 *                      b. editting chatting messages.
 *
 *                      +--------------------------+ 
 *                      | xxxxx :), xxxxxxxxxxxxxx |
 *                      | xxxxxxxxxxxxxxxxxxx.     |
 *                      | xx                       | <----- already sent messages
 *                      | xxxxxxxxxxxxxxxxxxxxxxxx |
 *                      | xxxxxxxxxxxxxxxxxxxxxxxx |
 *                      | xxxxxxxxxxxxxxxxxxxxxxxx |
 *                      +--------------------------+
 *                      | xxx :( xxxx |            |
 *                      |                          | <----- editting messages
 *                      +--------------------------+
 *
 *                      for 2-a: there is emoticon, normal text, eventtext, eventtext for hyperlinks
 *                               to trigger event, just like 1. 2-a should support select.
 *
 *                      for 2-b: there is emoticon and normal text. need to support select/edit. as
 *                               analyzed, edit is insert, and insert can be implemented by select.
 *                               so 2-b we need to support select.
 *                   
 *                 Event for handling or not:
 *
 *                 1. click, motion,  for sure we need to handle it for trigger event, for select.
 *                 2. ctrl+c/p/x, a/b/c..., Tokenboard won't handle this. alternatively, wrapper of
 *                    tokenboard handle it and query TokenBoard for proper behavor. IE:
 *                          ChatBox get event:
 *                          if it's "a":
 *                             1. if there is selected part, replace this part with ``a"
 *                             2. if not, put ``a" before the cursor.
 *                          if it's click:
 *                             1. directly pass it to tokenboard, to set cursor.
 *                          if it's ctrl-x:
 *                             1. query the tokenboard, if there is selected part, cut it.
 *                             2. otherwise do nothing.
 *
 *                 Some thing to think about:
 *
 *                 1. what's needed?
 *
 *                      What we need is to have class which can support case-1, case-2(a) and case-
 *                      2-(b), for case-1, we need support click event trigger, and the board is as
 *                      non-state class. For case-2(a) we need to support click event trigger, sele-
 *                      ct. For case-2(b) we need to support select, insert.
 *
 *                 2. how to support it?
 *                      Case-1 is simple and already supported. For select, we need to locate the
 *                      token under pointer. For insert we need to support builtin cursor. Both need
 *                      a (int, int) to support. Use (x, y) or (section, offset)?
 *
 *                      Let's use (x, y), since (x, y)->section is quick, but (section, offset)->
 *                      (x, y) is slow.
 *
 *                 3. let me add a (bSelectable, bEditable) to the class, when disable both, we have
 *                    the button-like text-terminal.
 *
 *                    (bSelectable, bEditable) with value:
 *
 *                    1. (0, 0): basic button-like text-terminal
 *                    2. (0, 1): no idea of this mode
 *                    3. (1, 0): sent-message box
 *                    4. (1, 1): input box
 *
 *                    or use (bSelectable, bWithCursor) with value:
 *                    1. (0, 0): basic button-like text-terminal
 *                    2. (0, 1): can't select, but can put cursor everywhere and insert, when push
 *                               and moving mouse, the cursor moving respectively
 *                    3. (1, 0): sent-message box, no cursor shown
 *                    4. (1, 1): classical input box
 *
 *                    Let's use (bSelectable, bWithCursor), having cursor means editable.
 *
 *                    Add another flag: bCanThrough, if true
 *
 *
 *                    +-----+  +---+
 *                    |     |  |   |
 *                    |     |  |   |  +----+
 *                L1  +-----+--+   +- |    |  <--- this is can through
 *                             |   |  |    |
 *                             +---+  |    |
 *                           +-----+  |    |
 *                           |     |  |    |
 *                           |     |  |    |
 *                L2  -------+-----+--+----+
 *
 *
 *                    if bCanThrough = false:
 *
 *                    +-----+  +---+
 *                    |     |  |   |
 *                    |     |  |   | 
 *                 L1 +-----+--+   +---------
 *                             |   |
 *                             +---+
 *                                    +----+  <--- this is can't through
 *                                    |    |
 *                                    |    |
 *                           +-----+  |    |
 *                           |     |  |    |
 *                           |     |  |    |
 *                 L2 -------+-----+--+----+
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

#include <limits>
#include <vector>
#include <SDL2/SDL.h>
#include <tinyxml2.h>
#include <functional>
#include <unordered_map>

#include "widget.hpp"
#include "section.hpp"
#include "tokenbox.hpp"
#include "xmlobjectlist.hpp"

enum XMLObjectType: int
{
    OBJECTTYPE_UNKNOWN      = 0,
    OBJECTTYPE_RETURN       = 1,
    OBJECTTYPE_PLAINTEXT    = 2,
    OBJECTTYPE_EVENTTEXT    = 3,
    OBJECTTYPE_EMOTICON     = 4,
};

class TokenBoard: public Widget
{
    // +-----------------------------+
    // |           0                 |
    // |  +---------------------+    |
    // |  |XXXXXXXX:)           |    |
    // |3 |XXXXXXXXXXXXXXX      | 1  |
    // |  |          XX         |    |
    // |  +---------------------+    |
    // |                             |
    // |           2                 |
    // +-----------------------------+

    // 1. Marg0 ~ Marg3 is for cursor blitting
    // 2. Internal box is for tokenbox *only*, |X X X  X|, no extra space
    //    any more in it beside the boundary
    // 3. The outer box as a whole to accept events

    public:
        // parameters
        //
        // bSelectable:       can select and copy
        // bWithCursor:       enable with: 1. can insert before cursor
        //                                 2. show a cursor
        //                                 3. select has nothing to do with current cursor
        // nMaxWidth  :       positive for wrap, non-positive for no-wrap
        // nWordSpace
        // nLineSpace
        //
        TokenBoard(
                int              nX,
                int              nY,
                bool             bSelectable,
                bool             bWithCursor,
                bool             bSpacePadding,
                bool             bCanThrough,
                int              nMaxWidth       = -1,
                int              nWordSpace      =  0,
                int              nLineSpace      =  0,
                uint8_t          nDefaultFont    =  0,
                uint8_t          nDefaultSize    =  10,
                uint8_t          nDefaultStyle   =  0,
                const SDL_Color &rstDefaultColor =  {0XFF, 0XFF, 0XFF, 0XFF},
                int              nMargin0        =  0,
                int              nMargin1        =  0,
                int              nMargin2        =  0,
                int              nMargin3        =  0,
                Widget          *pWidget         =  nullptr,
                bool             bFreeWidget     =  false)
            // don't rely on vim align
            // if it works terribly, just don't use it
            : Widget(nX, nY, 0, 0, pWidget, bFreeWidget)
            , m_WithCursor(bWithCursor)
            , m_Selectable(bSelectable)
            , m_SpacePadding(bSpacePadding)
            , m_CanThrough(bCanThrough)
            , m_PW(nMaxWidth)
            , m_SkipEvent(false)
            , m_Resolution(20)
            , m_WordSpace(nWordSpace)
            , m_LineSpace(nLineSpace)
            , m_CursorLoc(-1, -1)
            , m_DefaultFont(nDefaultFont)
            , m_DefaultSize(nDefaultSize)
            , m_DefaultStyle(nDefaultStyle)
            , m_DefaultColor(rstDefaultColor)
            , m_SkipUpdate(false)
            , m_Margin{ nMargin0, nMargin1, nMargin2, nMargin3 }
        {
            if(m_PW > 0){ m_W = m_Margin[1] + m_PW + m_Margin[3]; }
            Reset();
        }

        virtual ~TokenBoard() = default;

    public:
        bool ProcessEvent(const SDL_Event &, bool *);

    public:
        int XMLObjectType(const tinyxml2::XMLElement &);

    public:
        bool    Add(TOKENBOX &);
        void    Reset();
        void    Update(double);
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
        void SetTokenBoxStartX(int);
        void SetTokenBoxStartY(int, int);

    public:
        int  LinePadding(int);
        int  DoLinePadding(int, int, int);
        void ResetLine(int);

    private:
        bool    m_WithCursor;
        bool    m_Selectable;
        bool    m_SpacePadding;
        bool    m_CanThrough;
        int     m_MaxH1;
        int     m_MaxH2;
        int     m_CurrentLineMaxH2;
        int     m_PW;
        bool    m_SkipEvent;

    private:
        void UpdateSection(SECTION &, Uint32);

    private:
        const tinyxml2::XMLElement *NextObject(const tinyxml2::XMLElement *);
        void AddNewTokenBoxLine(bool);

    private:
        int  TokenBoxType(const TOKENBOX &);

    public:
        // using alias
        //      1. only use this in header file
        //      2. only use it when we have default parameters
        //      3. only use it when we can use initialization list for that parameter
        //
        using IDHandlerMap = std::unordered_map<std::string, std::function<void()>>;
        // load by XMLObjectList, failed then the tokenboard is undefined
        // 1. previous content will be destroyed
        // 2. failed then board is undefined
        // 3. TBD & TODO:
        //      if m_WithCursor is false, then last empty line will be deleted
        bool Load(XMLObjectList &rstXMLObjectList, const IDHandlerMap &rstMap = IDHandlerMap())
        {
            Reset();
            rstXMLObjectList.Reset();
            bool bRes = InnInsert(rstXMLObjectList, rstMap);
            if(bRes && !m_WithCursor){
                DeleteEmptyBottomLine();
            }
            return bRes;
        }

        bool LoadXML(const char *szXML, const IDHandlerMap &rstMap = IDHandlerMap())
        {
            XMLObjectList stXMLObjectList;
            if(stXMLObjectList.Parse(szXML, true)){
                stXMLObjectList.Reset();
                return Load(stXMLObjectList, rstMap);
            }

            return false;
        }


    private:
        bool ParseXMLContent(const tinyxml2::XMLElement *);

    private:
        bool ParseReturnObject();
        bool ParseEmoticonObject(const tinyxml2::XMLElement &);
        bool ParseTextObject(const tinyxml2::XMLElement &, int, const std::unordered_map<std::string, std::function<void()>> &);

    private:
        bool GetAttributeColor(SDL_Color *, const SDL_Color &, const tinyxml2::XMLElement &, const std::vector<std::string> &);
        bool GetAttributeAtoi(int *, int, const tinyxml2::XMLElement &, const std::vector<std::string> &);

    private:
        SDL_Color   GetFontColor(const tinyxml2::XMLElement &);
        int         GetFontIndex(const tinyxml2::XMLElement &);
        int         GetFontStyle(const tinyxml2::XMLElement &);
        int         GetFontSize (const tinyxml2::XMLElement &);

    private:
        void        ParseTextContentSection(const tinyxml2::XMLElement *, int);

    private:
        int         GetEmoticonSet  (const tinyxml2::XMLElement &);
        int         GetEmoticonIndex(const tinyxml2::XMLElement &);

    public:
        bool ObjectEmocticon(const tinyxml2::XMLElement &);
        bool ObjectReturn(const tinyxml2::XMLElement &);
        bool ObjectEventText(const tinyxml2::XMLElement &);

    private:
        bool MakeTokenBox(int, uint32_t, TOKENBOX *);

    private:
        void RedrawToken(int, int, TOKENBOX &, bool);
        bool DrawTextureCache();

    public:
        bool ParseXML(const char *);

    public:
        void Draw(int nX, int nY)
        {
            DrawEx(nX, nY, 0, 0, W(), H());
        }

    public:
        void DrawEx(int, int, int, int, int, int);

    private:
        bool AddNewTokenBox(TOKENBOX &);

    public:

    private:
        int GetLineIntervalMaxH2(int, int, int);
        int GetLineTokenBoxStartY(int, int, int, int);
        int GetNewLineStartY(int);

        bool GetTokenBoxLocation(int, int, int &, int &, int &, int &);
        bool GetTokenBoxStartPoint(int, int, int &, int &);
        void SetDrawToTextureCache();
        void SetDrawToScreen();

    private:
        void ClearCurrentLine();

    public:
        int Width();
        int Height();
    private:
        void RedrawSection(int);

    private:
        std::vector<std::vector<TOKENBOX>> m_LineV;
        std::unordered_map<int, SECTION>   m_SectionV;
        std::vector<int>                   m_LineStartY;
    private:
        std::vector<std::vector<std::vector<std::pair<int, int>>>> m_TokenBoxBitmap;

    private:
        void MakeTokenBoxEventBitmap();
        void MarkTokenBoxEventBitmap(int, int);
        int  m_Resolution;
        int  m_WordSpace;
        int  m_LineSpace;

    private:
        std::pair<int, int> m_CursorLoc;

    private:
        // this is used when insert words, empty lines, etc.
        uint8_t   m_DefaultFont;
        uint8_t   m_DefaultSize;
        uint8_t   m_DefaultStyle;
        SDL_Color m_DefaultColor;

        bool      m_SkipUpdate;

    private:
        // margins for the board, this is needed for balabala..
        //
        //  +-------------------------+
        //  |        M0               |
        //  |  +-------------+        |
        //  |  |             |        |
        //  |M3|             |  M1    |
        //  |  |             |        |
        //  |  +-------------+        |
        //  |        M2               |
        //  +-------------------------+
        //
        int  m_Margin[4];

        std::vector<bool> m_EndWithCR;

    private:
        // callbacks
        std::unordered_map<int, std::function<void()>> m_IDFuncV;

    private:
        int SectionTypeCount(int, int);

        void DrawRectLine(const SDL_Rect &);

    public:
        const std::string &ContentExport();
        int GuessResoltion();

        void TokenBoxGetMouseButtonUp(int, int, bool);
        void TokenBoxGetMouseButtonDown(int, int, bool);
        void TokenBoxGetMouseMotion(int, int, bool);

        void ProcessEventMouseButtonUp(int, int);
        void ProcessEventMouseButtonDown(int, int);
        void ProcessEventMouseMotion(int, int);

    public:
        bool AddTokenBoxV(const std::vector<TOKENBOX> &);
        bool Delete(bool);

    public:
        void GetCursor(int *pX, int *pY)
        {
            if(pX){ *pX = m_CursorLoc.first;  }
            if(pY){ *pY = m_CursorLoc.second; }
        }

        bool SetCursor(int nX, int nY)
        {
            if(CursorValid(nX, nY)){
                m_CursorLoc = {nX, nY};
                return true;
            }
            return false;
        }

        int GetWordSpace()
        {
            return m_WordSpace;
        }

        int GetLineSpace()
        {
            return m_LineSpace;
        }

        int GetLineTokenBoxCount(int nLine)
        {
            if(nLine >= 0 && nLine < (int)(m_LineV.size())){
                return (int)(m_LineV[nLine].size());
            }

            return -1;
        }

        int GetLineCount()
        {
            return (int)(m_LineV.size());
        }

        int BreakLine();

        // always we need an default environment
        void SetDefaultFont(uint8_t nFont, uint8_t nSize, uint8_t nStyle, const SDL_Color &rstColor)
        {
            m_DefaultFont  = nFont;
            m_DefaultSize  = nSize;
            m_DefaultStyle = nStyle;
            m_DefaultColor = rstColor;
        }

    public:
        bool  InnInsert(XMLObjectList &, const std::unordered_map<std::string, std::function<void()>> &);

        int LineFullWidth(int);
        int LineRawWidth(int, bool);
        int SetTokenBoxWordSpace(int);

        std::pair<int, int> m_LastTokenBoxLoc;

    private:
        bool CursorValid(int nX, int nY)
        {
            return true
                && nY >= 0
                && nY < (int)m_LineV.size()
                && nX >= 0
                && nX <= (int)m_LineV[nY].size();
        }

        bool CursorValid()
        {
            return CursorValid(m_CursorLoc.first, m_CursorLoc.second);
        }

        bool LineValid(int nLine)
        {
            return nLine >= 0 && nLine < (int)(m_LineV.size());
        }

        bool TokenBoxValid(int nX, int nY)
        {
            return true
                && nY >= 0
                && nY < (int)m_LineV.size()
                && nX >= 0
                && nX < (int)m_LineV[nY].size();
        }

        bool LastTokenBoxValid()
        {
            return TokenBoxValid(m_LastTokenBoxLoc.first, m_LastTokenBoxLoc.second);
        }

    private:
        int GetNewHBasedOnLastLine();

    public:
        int GetBlankLineHeight();

    public:
        std::string GetXML(bool);


        std::string InnGetXML(int, int, int, int);

        bool GetTokenBoxInfo(int, int, int *, int *, int *, int *, int *, int *, int *);

        void GetDefaultFontInfo(uint8_t *, uint8_t *, uint8_t *);

    public:
        bool AddUTF8Code(uint32_t);
        void ResetOneLine(int);
        void ResetLineStartY(int);
        void DeleteEmptyBottomLine();

    public:
        int GetLineStartY(int);
        int GetLineMaxH1(int);

    private:

        std::array<std::pair<int, int>, 2> m_SelectLoc;

    private:
        bool SectionValid(int nSectionID, bool bCheckSectionType = true)
        {
            // 1. invalid id
            if(nSectionID < 0){ return false; }

            // 2. id not found
            auto p = m_SectionV.find(nSectionID);
            if(p == m_SectionV.end()){ return false; }

            if(!bCheckSectionType){ return true; }

            switch(p->second.Info.Type){
                case SECTIONTYPE_PLAINTEXT:
                case SECTIONTYPE_EVENTTEXT:
                case SECTIONTYPE_EMOTICON:
                    return true;
                default:
                    return false;
            }
        }

        int CreateSection(const SECTION &rstSection, const std::function<void()> &fnCB = [](){})
        {
            // TODO
            // make it more efficient
            int nID = 0;
            while(nID <= std::numeric_limits<int>::max()){
                if(m_SectionV.find(nID) == m_SectionV.end()){
                    // good
                    m_SectionV[nID] = rstSection;
                    m_IDFuncV[nID]  = fnCB;
                    return nID;
                }
                nID++;
            }
            return -1;
        }

    public:
        std::string Print(bool);

    public:
        int Margin(int nIndex)
        {
            return (nIndex >= 0 && nIndex < 4) ? m_Margin[nIndex] : -1;
        }

    private:
        int     m_SelectState;  // 0: no selection
        // 1: selecting
        // 2: selection done with result available
};
