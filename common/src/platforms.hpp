/*
 * =====================================================================================
 *
 *       Filename: platforms.hpp
 *        Created: 02/06/2016 17:41:44
 *  Last Modified: 02/06/2016 17:56:46
 *
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

bool PlatformWindows()
{
#if !defined(SAG_COM) && \
    (defined(WIN64) || defined(_WIN64) || defined(__WIN64__) \
     || defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
    return true;
#else
    return false;
#endif
}

bool PlatformLinux()
{
#if defined(__linux__) || defined(__linux)
    return true;
#else
    return false;
#endif
}
