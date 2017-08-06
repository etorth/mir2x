/*
 * =====================================================================================
 *
 *       Filename: effectnode.hpp
 *        Created: 08/05/2017 22:58:20
 *  Last Modified: 08/06/2017 01:42:29
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
struct EffectNode
{
    // part-1: const fields
    //         description of this effect

    int Effect;

    // part-2 : mutable field
    //          always initialized as 0 and get updated later
    int Frame;

    EffectNode(int nEffect)
        : Effect(nEffect)
        , Frame(0)
    {}

    operator bool () const
    {
        return Effect > 0;
    }

    void Print() const
    {
    }
};
