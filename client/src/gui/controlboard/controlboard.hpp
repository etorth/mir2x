#pragma once
#include <string_view>
#include "widget.hpp"
#include "cbleft.hpp"
#include "cbright.hpp"
#include "cbmiddle.hpp"
#include "cbmiddleexpand.hpp"
#include "cbtitle.hpp"

// for texture 0X00000012 and 0X00000013
// I split it into many parts to fix different screen size
// for screen width is not 800 we build a new interface using these two
//
// 0X00000012 : 800 x 133:  left and right
// 0X00000013 : 456 x 131:  middle log
// 0X00000022 : 127 x 41 :  title
//
//                         +-----------+                           ---
//                          \  title  /                             ^
// +------+==----------------+       +----------------==+--------+  |  --- <-- left/right is 133, middle is 131
// |      $                                        +---+$        | 152  | ---
// |      |                                        |   ||        |  |  133 | 120 as underlay log
// |      |                                        +---+|        |  V   |  |
// +------+---------------------------------------------+--------+ --- -- ---
// ^      ^    ^           ^           ^          ^     ^        ^
// | 178  | 50 |    110    |   127     |    50    | 119 |   166  | = 800
//
// |---fixed---|-------------repeat---------------|---fixed------|

//
// 0X00000027 : 456 x 298: char box frame
//
//                         +-----------+                        ---
//                          \  title  /                          ^
//        +==----------------+       +----------------==+ ---    |  ---- <-- startY
//        $                                             $  ^     |   47
//        |                                             |  |     |  ----
//        |                                             |  |     |   |
//        |                                             |  |     |  196: use to repeat, as m_stretchH
//        |                                             | 298   319  |
//        +---------------------------------------+-----+  |     |  ----
//        |                                       |     |  |     |   55
//        |                                       |() ()|  |     |   |
//        |                                       |     |  v     v   |
// +------+---------------------------------------+-----+--------+ --- -- ---
// ^      ^    ^           ^           ^          ^     ^        ^
// | 178  | 50 |    110    |   127     |    50    | 119 |   166  | = 800
//
// |---fixed---|-------------repeat---------------|---fixed------|

enum
{
    CBLOG_DEF = 0,
    CBLOG_SYS,
    CBLOG_DBG,
    CBLOG_ERR,
};

class CBMiddle;
class ProcessRun;
class ControlBoard: public Widget
{
    private:
        friend class CBLeft;
        friend class CBRight;
        friend class CBTitle;
        friend class CBMiddle;
        friend class CBMiddleExpand;

    private:
        ProcessRun *m_processRun;

    private:
        bool m_expand   = false;
        bool m_maximize = false;
        bool m_minimize = false;

    private:
        LayoutBoard m_logBoard;
        LayoutBoard m_cmdBoard;

    private:
        CBLeft  m_left;
        CBRight m_right;

        CBMiddle       m_middle;
        CBMiddleExpand m_middleExpand;

        CBTitle m_title;

    public:
        ControlBoard(
                ProcessRun *,

                Widget * = nullptr,
                bool     = false);

    public:
        void addXMLLog(const char *);
        void addParLog(const char *);

    public:
        void addLog(int, const char *);

    public:
       TritexButton *getButton(const std::string_view &);

    private:
       void onClickSwitchModeButton(int);

    private:
       void onInputDone();

    public:
       int shiftHeight() const;
};
