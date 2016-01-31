/*
 * =====================================================================================
 *
 *       Filename: clientmessage.hpp
 *        Created: 01/11/2016 23:07:02
 *  Last Modified: 01/11/2016 23:22:09
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
#include <cstdint>

// message without content
// should be 1 ~ 255, 0 is prohibited

const uint16_t  CM_RIDEHORSE       = 1;

// message with fixed size content
// should be 256 ~ 65535
const uint16_t  CM_PING           = 256;
const uint16_t  CM_LOGIN          = 257;

