/*
 * =====================================================================================
 *
 *       Filename: zcompf.hpp
 *        Created: 04/06/2020 10:50:49
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
#include "lz4.h"

namespace zcompf
{
    void lz4Encode();
    void lz4Decode();

    void xorEncode();
    void xorDecode();
}
