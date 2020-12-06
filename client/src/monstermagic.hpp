/*
 * =====================================================================================
 *
 *       Filename: monstermagic.hpp
 *        Created: 08/07/2017 21:19:44
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
#include "dbcomid.hpp"
#include "magicbase.hpp"
#include "fixedlocmagic.hpp"
#include "followuidmagic.hpp"

template<int magicID, int magicStage> class MonsterMagic;
template<> class MonsterMagic<DBCOM_MAGICID(u8"神兽-喷火"), magicStageID(u8"运行")>: public AttachMagic
{
    private:
        const std::unordered_map<int, std::array<int, 2>> *m_dirOffCPtr;

    public:
        MonsterMagic(int gfxDirIndex)
            : AttachMagic(u8"神兽-喷火", u8"运行", gfxDirIndex)
            , m_dirOffCPtr([]()
              {
                  const static std::unordered_map<int, std::array<int, 2>> s_dirOff
                  {
                      {DIR_UP,        { 0,  2}},
                      {DIR_UPRIGHT,   {-3, -3}},
                      {DIR_RIGHT,     {-2, -5}},
                      {DIR_DOWNRIGHT, { 0,  0}},
                      {DIR_DOWN,      { 0, -5}},
                      {DIR_DOWNLEFT,  { 0,  0}},
                      {DIR_LEFT,      { 2, -5}},
                      {DIR_UPLEFT,    { 3, -3}},
                  };
                  return &s_dirOff;
              }())
        {}

    public:
        void drawShift(int drawOffX, int drawOffY, bool alpha) override
        {
            const int dir = gfxDirIndex() + DIR_BEGIN;
            AttachMagic::drawShift(drawOffX + m_dirOffCPtr->at(dir).at(0), drawOffY + m_dirOffCPtr->at(dir).at(1), alpha);
        }
};
