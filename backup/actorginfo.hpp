/*
 * =====================================================================================
 *
 *       Filename: actorginfo.hpp
 *        Created: 03/12/2016 01:18:31
 *  Last Modified: 03/12/2016 18:46:36
 *
 *    Description: actor global info shared by all actors
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

class ActorGInfo final
{
    public:
        ActorGInfo();
       ~ActorGInfo() = default;

    public:
       int FrameCount(int nState, int nDirection)
       {
           return m_FrameCount[nState][nDirection];
       }

    private:
        int m_FrameCount[32][8];


};
