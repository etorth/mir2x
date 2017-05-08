/*
 * =====================================================================================
 *
 *       Filename: humanskill.hpp
 *        Created: 05/06/2017 21:12:35
 *  Last Modified: 05/07/2017 23:06:17
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
#include <string>

struct HumanSkill
{
    int ID;
    int Job;
    int Type;
    int Delay;
    int Motion;
    int Effect;

    int Spell0;
    int Spell1;
    int Power0;
    int Power1;
    int MaxPower0;
    int MaxPower1;

    int NeedL0;
    int NeedL1;
    int NeedL2;
    int L0Train;
    int L1Train;
    int L2Train;

    std::string Name;
    std::string Description;

    HumanSkill(int nID,
               int nJob,
               int nType,
               int nDelay,
               int nMotion,
               int nEffect,
               int nSpell0,
               int nSpell1,
               int nPower0,
               int nPower1,
               int nMaxPower0,
               int nMaxPower1,
               int nNeedL0,
               int nNeedL1,
               int nNeedL2,
               int nL0Train,
               int nL1Train,
               int nL2Train,

               const char *szName,
               const char *szDescription)
        : ID(nID)
        , Job(nJob)
        , Type(nType)
        , Delay(nDelay)
        , Motion(nMotion)
        , Effect(nEffect)
        , Spell0(nSpell0)
        , Spell1(nSpell1)
        , Power0(nPower0)
        , Power1(nPower1)
        , MaxPower0(nMaxPower0)
        , MaxPower1(nMaxPower1)
        , NeedL0(nNeedL0)
        , NeedL1(nNeedL1)
        , NeedL2(nNeedL2)
        , L0Train(nL0Train)
        , L1Train(nL1Train)
        , L2Train(nL2Train)
        , Name(szName ? szName : "")
        , Description(szDescription ? szDescription : "")
    {}
};
