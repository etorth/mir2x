/*
 * =====================================================================================
 *
 *       Filename: labelboard.cpp
 *        Created: 08/12/2015 09:59:15
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

#include "log.hpp"
#include "strfunc.hpp"
#include "xmlboard.hpp"
#include "colorfunc.hpp"
#include "labelboard.hpp"

extern Log *g_Log;

void LabelBoard::SetText(const char * szFormatString, ...)
{
    std::string szText;
    bool bError = false;
    {
        va_list ap;
        va_start(ap, szFormatString);

        try{
            szText = str_vprintf(szFormatString, ap);
        }catch(const std::exception &e){
            bError = true;
            szText = str_printf("Exception caught in LabelBoard::SetText(\"%s\", ...): %s", szFormatString, e.what());
        }

        va_end(ap);
    }

    if(bError){
        g_Log->AddLog(LOGTYPE_WARNING, "%s", szText.c_str());
    }

    // use the fallback values of m_Board
    // don't need to specify the font/size/style info here
    m_Board.LoadXML(str_printf("<par>%s</par>", szText.c_str()).c_str());

    m_W = m_Board.PX() + m_Board.PW();
    m_H = m_Board.PY() + m_Board.PH();
}
