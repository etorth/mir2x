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
#include "toll.hpp"
#include "strf.hpp"
#include "xmltypeset.hpp"
#include "colorf.hpp"
#include "labelboard.hpp"

extern Log *g_log;

void LabelBoard::setText(const char8_t *szFormatString, ...)
{
    std::string szText;
    bool bError = false;
    {
        va_list ap;
        va_start(ap, szFormatString);

        try{
            szText = str_vprintf(to_cstr(szFormatString), ap);
        }catch(const std::exception &e){
            bError = true;
            szText = str_printf("Exception caught in labelBoard::setText(\"%s\", ...): %s", szFormatString, e.what());
        }

        va_end(ap);
    }

    if(bError){
        g_log->addLog(LOGTYPE_WARNING, "%s", szText.c_str());
    }

    // use the fallback values of m_tpset
    // don't need to specify the font/size/style info here
    m_tpset.loadXML(str_printf("<par>%s</par>", szText.c_str()).c_str());

    m_w = m_tpset.px() + m_tpset.pw();
    m_h = m_tpset.py() + m_tpset.ph();
}
