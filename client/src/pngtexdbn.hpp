/*
 * =====================================================================================
 *
 *       Filename: pngtexdbn.hpp
 *        Created: 03/17/2016 01:17:51
 *  Last Modified: 03/17/2016 01:29:13
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
#include "pngtexdb.hpp"

class PNGTexDBN: public PNGTexDB<0>
{
    public:
        PNGTexDBN()
            : PNGTexDB<0>()
        {
            extern PNGTexDBN *g_PNGTexDBN;
            if(g_PNGTexDBN){
                throw std::runtime_error("one instance for PNGTexDBN please");
            }
        }
};
