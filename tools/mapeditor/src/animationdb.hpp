/*
 * =====================================================================================
 *
 *       Filename: animationdb.hpp
 *        Created: 06/22/2016 18:19:55
 *  Last Modified: 06/22/2016 18:21:15
 *
 *    Description: db for testing animation
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

class AnimationDB
{
    private:
        std::vector<Animation>  m_AnimationV;

    public:
        AnimationDB();
        ~AnimationDB() = default;
};
