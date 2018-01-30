/*
 * =====================================================================================
 *
 *       Filename: attachmagic.hpp
 *        Created: 08/10/2017 12:17:50
 *    Description: For attached magic we don't need its location info
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
#include "magicbase.hpp"
#include "magicrecord.hpp"
class AttachMagic: public MagicBase
{
    public:
        AttachMagic(int, int, int);
        AttachMagic(int, int, int, double);

    public:
        void Update(double);
        void Draw(int, int);

    public:
        bool Done() const;
};
