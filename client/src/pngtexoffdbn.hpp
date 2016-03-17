/*
 * =====================================================================================
 *
 *       Filename: pngtexoffdbn.hpp
 *        Created: 03/17/2016 01:17:51
 *  Last Modified: 03/17/2016 01:22:31
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
#include "pngtexoffdb.hpp"

class PNGTexOffDBN: public PNGTexOffDB<0>
{
    public:
        PNGTexOffDBN()
            : PNGTexOffDB<0>()
        {
            extern PNGTexOffDBN *g_PNGTexOffDBN;
            if(g_PNGTexOffDBN){
                throw std::runtime_error("one instance for PNGTexDBN please");
            }
        }
};
