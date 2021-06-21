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
                  -32,
                  -89,
                  {
                      #include "mon_12.wil.index_00040.inc"
                  },
              },

              {
                  -33,
                  -87,
                  {
                      #include "mon_12.wil.index_00041.inc"
                  },
              },

              {
                  -33,
                  -86,
                  {
                      #include "mon_12.wil.index_00042.inc"
                  },
              },

              {
                  -33,
                  -87,
                  {
                      #include "mon_12.wil.index_00043.inc"
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
