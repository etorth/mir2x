/*
 * =====================================================================================
 *
 *       Filename: logtype.hpp
 *        Created: 02/13/2018 19:31:33
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

#pragma once
#include <array>
#include <string>
#include <functional>
#include <type_traits>

#define LOGV_INFO    0
#define LOGV_WARNING 1
#define LOGV_FATAL   2
#define LOGV_DEBUG   3

#define LOGTYPE_INFO    {std::to_string(LOGV_INFO   ), std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}
#define LOGTYPE_WARNING {std::to_string(LOGV_WARNING), std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}
#define LOGTYPE_FATAL   {std::to_string(LOGV_FATAL  ), std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}
#define LOGTYPE_DEBUG   {std::to_string(LOGV_DEBUG  ), std::string(__FILE__), std::to_string(__LINE__), std::string(__PRETTY_FUNCTION__)}

namespace LogStr
{
    inline void AddLog(const std::array<std::string, 4> &stLogType,
            const char *szLogFormat,
            va_list ap,
            const std::function<void(int, const char *)> &fnAddLog,
            const std::function<void(     const char *)> &fnAddError)
    {
        int nLogSize = 0;
        int nLogType = std::atoi(stLogType[0].c_str());

        // need to prepare this copy
        // in case parsing with static buffer failed

        va_list ap_copy;
        va_copy(ap_copy, ap);

        // 1. try static buffer
        //    give an enough size so we can hopefully stop here
        {
            char szSBuf[1024];
            nLogSize = std::vsnprintf(szSBuf, std::extent<decltype(szSBuf)>::value, szLogFormat, ap);

            if(nLogSize >= 0){
                if((size_t)(nLogSize + 1) < std::extent<decltype(szSBuf)>::value){
                    fnAddLog(nLogType, szSBuf);
                    goto __add_log_end;
                }else{
                    // do nothing
                    // have to try the dynamic buffer method
                }
            }else{
                fnAddError((std::string("Parse log info failed: ") + szLogFormat).c_str());
                goto __add_log_end;
            }
        }

        // 2. try dynamic buffer
        //    use the parsed buffer size above to get enough memory
        while(true){
            std::vector<char> szDBuf(nLogSize + 1 + 64);

            va_list ap_curr;
            va_copy(ap_curr, ap_copy);
            nLogSize = std::vsnprintf(&(szDBuf[0]), szDBuf.size(), szLogFormat, ap_curr);
            va_end(ap_curr);

            if(nLogSize >= 0){
                if((size_t)(nLogSize + 1) < szDBuf.size()){
                    fnAddLog(nLogType, &(szDBuf[0]));
                    goto __add_log_end;
                }else{
                    szDBuf.resize(nLogSize + 1 + 64);
                }
            }else{
                fnAddError((std::string("Parse log info failed: ") + szLogFormat).c_str());
                goto __add_log_end;
            }
        }

__add_log_end:
        // use jump to release ap_copy
        // don't use dtor or something to do it
        va_end(ap_copy);
    }
}
