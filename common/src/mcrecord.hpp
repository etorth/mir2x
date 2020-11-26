/*
 * =====================================================================================
 *
 *       Filename: mcrecord.hpp
 *        Created: 08/04/2017 23:00:09
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
class MCRecord // magic category record
{
    public:
        const char8_t *name;

    public:
        constexpr operator bool () const
        {
            return name[0] != '\0';
        }
};
