/*
 * =====================================================================================
 *
 *       Filename: sessiononhc.cpp
 *        Created: 02/28/2016 01:37:19
 *  Last Modified: 02/28/2016 01:48:55
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
#include <cstdint>

Session::OnPing()
{
    static uint32_t nTick;

    auto fnOperateData = [this, &nTick](){

    };

    Read(&nTick, 4, fnOperateData);
}
