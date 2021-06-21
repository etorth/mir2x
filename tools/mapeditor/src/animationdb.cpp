/*
 * =====================================================================================
 *
 *       Filename: animationdb.cpp
 *        Created: 06/22/2016 18:21:16
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

#include "mainwindow.hpp"
#include "animationdb.hpp"

extern MainWindow *g_mainWindow;
AnimationDB::AnimationDB()
    : m_animationList
      {
          { // animation 0
              {
                  8,
                  -47,
                  {
                      #include "animation/m_human.wil.index_09040.inc"
                  },
              },

              {
                  8,
                  -47,
                  {
                      #include "animation/m_human.wil.index_09041.inc"
                  },
              },

              {
                  8,
                  -47,
                  {
                      #include "animation/m_human.wil.index_09042.inc"
                  },
              },

              {
                  8,
                  -47,
                  {
                      #include "animation/m_human.wil.index_09043.inc"
                  },
              },
          },

          { // animation 1
              {
                  -32,
                  -89,
                  {
                      #include "animation/mon_12.wil.index_00040.inc"
                  },
              },

              {
                  -33,
                  -87,
                  {
                      #include "animation/mon_12.wil.index_00041.inc"
                  },
              },

              {
                  -33,
                  -86,
                  {
                      #include "animation/mon_12.wil.index_00042.inc"
                  },
              },

              {
                  -33,
                  -87,
                  {
                      #include "animation/mon_12.wil.index_00043.inc"
                  },
              },
          },

          { // animation 2
              {
                  -24,
                  -97,
                  {
                      #include "animation/mon_2.wil.index_0040.inc"
                  },
              },

              {
                  -21,
                  -96,
                  {
                      #include "animation/mon_2.wil.index_0041.inc"
                  },
              },

              {
                  -20,
                  -96,
                  {
                      #include "animation/mon_2.wil.index_0042.inc"
                  },
              },

              {
                  -22,
                  -96,
                  {
                      #include "animation/mon_2.wil.index_0043.inc"
                  },
              },
          },
      }
{}

Animation *AnimationDB::getAnimation()
{
    if(const auto index = g_mainWindow->getAnimationIndex(); index >= 0){
        return &(m_animationList.at(index));
    }
    return nullptr;
}
