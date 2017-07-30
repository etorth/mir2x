/*
 * =====================================================================================
 *
 *       Filename: dropitemconfig.hpp
 *        Created: 07/30/2017 00:11:10
 *  Last Modified: 07/30/2017 00:11:37
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
struct DropItemConfig
{
    const char *MonsterName;        // convert to id internally, monster name who drops this item
    const char *ItemName;           // convert to id internally, item name to drop

    int Group;                      // can only drop at most one item in the group, zero means no group
    int ProbRecip;                  // 1 / p, zero means disable this entry

    int Repeat;                     // try Repeat times for current item record, zero means disable this entry
    int Value;                      // how many items to drop, gold means value, zero means disable this entry

    DropItemConfig(
            const char *szMonsterName,
            const char *szItemName,

            int nGroup,
            int nProbRecip,
            
            int nRepeat,
            int nValue)
        : MonsterName(szMonsterName)
        , ItemName(szItemName)
        , Group(nGroup)
        , ProbRecip(nProbRecip)
        , Repeat(nRepeat)
        , Value(nValue)
    {}

    void Print() const;

    // check if current entry is valid
    // 1. check member variables
    // 2. check if the provided names are legal and practical
    operator bool() const;
};
