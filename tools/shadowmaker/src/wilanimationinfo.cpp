/*
 * =====================================================================================
 *
 *       Filename: wilanimationinfo.cpp
 *        Created: 8/8/2015 4:41:53 AM
 *  Last Modified: 04/15/2016 21:44:16
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

#include "supwarning.hpp"
#include "wilanimationinfo.hpp"

int WilAnimationStartBaseIndex(int nFileIndex, int nAnimationIndex, int nStatus, int nDirection)
{
    UNUSED(nFileIndex);

    if(0){
    }else{
        // default
        return nAnimationIndex * 1000 + (nStatus * 8 + nDirection) * 10;
    }
}

int WilAnimationFrameCount(int nFileIndex, int nAnimationIndex, int nStatus, int nDirection)
{
    UNUSED(nFileIndex);
    UNUSED(nAnimationIndex);
    UNUSED(nDirection);

    if(0){
    }else{
        // default
        switch(nStatus){
            case 0:
                return 4;
            case 1:
                return 6;
            case 2:
                return 6;
            case 3:
                return 2;
            case 4:
                return 10;
            default:
                return 0;
        }
    }
}
