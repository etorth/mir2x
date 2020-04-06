/*
 * =====================================================================================
 *
 *       Filename: msgattribute.hpp
 *        Created: 04/23/2017 23:05:02
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

#include <cstddef>
#include <cstdint>
#include <string>
#include "fflerror.hpp"

struct msgAttribute
{
    //  0    :     empty
    //  1    : not empty,     fixed size,     compressed by xor
    //  2    : not empty,     fixed size, not compressed
    //  3    : not empty, not fixed size, not compressed
    //  4    : not empty, not fixed size,     compressed by lz4
    const int type;

    // define message length before compression
    //  0    : empty or not fixed
    //  x    : non-empty fixed size before compression
    //         since for fixed size compressed message we support maxCompLen = 256 + 255
    //         then for maxOrigLen > maxCompLen we may fail since compression may not save any bytes
    const size_t dataLen;
    const std::string name;

    msgAttribute(int t, size_t len, const char *nameStr)
        : type(t)
        , dataLen(len)
        , name(nameStr ? nameStr : "")
    {
        // we can't use the log system here for error logging
        // since msgAttribute will be used in both ClientMessage and ServerMessage

        if(name.empty()){
            throw fflerror("invalid message name");
        }

        switch(type){
            case 0:
                {
                    // empty, shouldn't have any content
                    if(dataLen){
                        throw fflerror("invalid dataLen: %zu", dataLen);
                    }
                    break;
                }
            case 1:
                {
                    // not empty, fixed size, compressed
                    // support maxCompLen = 256 + 255, abort if bigger than it

                    if(!(dataLen > 0 && dataLen < 256 + 255)){
                        throw fflerror("invalid dataLen: %zu", dataLen);
                    }
                    break;
                }
            case 2:
                {
                    // not empty, fixed size, not compressed
                    if(!dataLen){
                        throw fflerror("invalid dataLen: %zu", dataLen);
                    }
                    break;
                }
            case 3:
            case 4:
                {
                    // not empty, not fixed size, not compressed
                    // not empty, not fixed size,     compressed
                    // dataLen should be zero to indicate it's not fixed size message
                    if(dataLen){
                        throw fflerror("invalid dataLen: %zu", dataLen);
                    }
                    break;
                }
            default:
                {
                    throw fflerror("invalid message type: %d", type);
                }
        }
    }
};
