/*
 * =====================================================================================
 *
 *       Filename: tokenboard.hpp
 *        Created: 06/17/2015 10:24:27 PM
 *  Last Modified: 03/21/2016 23:16:12
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
 *                 Analysis: why we use select to support insert:
 *
 *                   for insert, we add new tokens to current board, we can everytime insert a new
 *                   token and re-padding, or we can return the underlying XML/plainText, and then
 *                   re-generate the whole board. Re-generatioin is better since re-padding every-
 *                   time is a re-generation. Even if we implement a new class EditBoard, it's still
 *                   a decision for re-padding and re-generation. So just use TokenBoard.
 *
 *                 For boards need to support insert, we need to supprt insert at anywhere we want,
 *                 this means we need to support select anywhere we want by click at the token of 
 *                 the place. So problem comes with eventtext, which already take click as event tr-
 *                 igger.
 *
 *                   But it's OK since for editable board, we don't need eventtext(hyperlinks), the
 *                 place we need hyperlinks is for case 2-a. This means at which we need to trigger
 *                 event, just trigger it, don't think about place location for insert. Since if we
 *                 really need to insert in current board, this board doesn't need trigger event.
 *
 *                   Another problem, how to support insert before current cursor? We can use func-
 *                 tion like GetXML() or GetPlainText(), but both just return formated text. inside
 *                 the tokenboard, cursor is something like (x, y) or (section, offset). we can't do
 *                 insert based on the returned text.
 *
 *                   Currently what I can get is inside the board class for each section I keep a
 *                 copy of its stinrg content, this helps to return GetXML() and GetPlainText() qui-
 *                 ckly and also we can maintain the mapping inside, and each time warpper class can
 *                 only ask for insert a token or a piece of text. Internal text mapping will handle
 *                 it properly, with update of string copy.
 * 
 *                 BUT, if we use this strategy, why not keep an external copy in the wrapper class
 *                 and just make TokenBoard only support case 1???
 *
 *                 No matter what, we need a string copy seems, since directly insert between token
 *                 cause more re-padding. So keep internal or external? Currently I prefer external,
 *                 and tokenboard is just a show board to reflect the text operation.
 *
 *                 Or just do directly tokenbox insert? because this affect only by para-graph, bet-
 *                 ween paragraphes maybe there is blank space left for the last line of the current
 *                 paragraph then we can just add Y for rest of paragraph? This logic will be much 
 *                 more complicated.
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

    public:
        int InSection(int, int);

    public:
        bool ProcessEvent(int, int, const SDL_Event &);

    public:
        bool    Add(TOKENBOX &);
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
        const TOKENBOX *m_LastTokenBox;

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
        void MarkEventTextBitmap(const TOKENBOX &);
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
        int GuessResoltion();
};
