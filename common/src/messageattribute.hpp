/*
 * =====================================================================================
 *
 *       Filename: messageattribute.hpp
 *        Created: 04/23/2017 23:05:02
 *  Last Modified: 08/14/2017 23:08:54
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

#include <cstddef>
#include <cstdint>
#include <cassert>

struct MessageAttribute
{
    // define message type
    //
    //  0    :     empty
    //  1    : not empty,     fixed size,     compressed
    //  2    : not empty,     fixed size, not compressed
    //  3    : not empty, not fixed size, not compressed
    //
    // we don't have a "not empty, not fixed size, compressed" type message
    // if we need to send such a message, we should firstly compress it and send it in mode 3
    int Type;

    // define message length before compression
    //  0    : empty or not fixed
    //  x    : non-empty fixed size before compression
    //         since for fixed size compressed message we support maxCompLen = 256 + 255
    //         then for maxOrigLen > maxCompLen we may fail since compression may not save any bytes
    size_t DataLen;

    // message name
    // I treat (Name == "") as error
    std::string Name;

    MessageAttribute(int nType, size_t nDataLen, const char *pName)
        : Type(nType)
        , DataLen(nDataLen)
        , Name(pName ? pName : "")
    {
        // we can't use the log system here for error logging
        // since MessageAttribute will be used in both ClientMessage and ServerMessage
        assert(Name != "");

        switch(Type){
            case 0:
                {
                    // empty, shouldn't have any content
                    assert(!DataLen);
                    break;
                }
            case 1:
                {
                    // not empty, fixed size, compressed
                    // since for this mode we support maxCompLen = 256 + 255
                    // then for case maxOrigLen > maxCompLen it could fail since compression may not save any bytes

                    // how to solve this problem
                    // solution-1: force maxOrigLen <= maxCompLen for any message in mode 1
                    // solution-2: add a special message header code as MSG_LONGMSG which contains the long message
                    //             memory structure as:
                    //                    0          1234      5            6.....
                    //               MSG_LONGMSG    length     HC  (mask, compressed data)
                    // solution-3: never handle it, use at your own risk
                    //
                    // currently I am using solution-3
                    // when failed in compression I'll get informed in the log and redesign that message
                    assert(DataLen);
                    break;
                }
            case 2:
                {
                    // not empty, fixed size, not compressed
                    assert(DataLen);
                    break;
                }
            case 3:
                {
                    // not empty, not fixed size, not compressed
                    // DataLen should be zero to indicate it's not fixed size message
                    assert(!DoneLen);
                    break;
                }
            default:
                {
                    // abort without any message, really bad
                    std::abort();
                    break;
                }
        }
    }
};
