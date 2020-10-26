/*
 * =====================================================================================
 *
 *       Filename: dropitemconfig.hpp
 *        Created: 07/30/2017 00:11:10
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
    const char8_t *monsterName;     // convert to id internally, monster name who drops this item
    const char8_t *itemName;        // convert to id internally, item name to drop

    int group;                      // can only drop at most one item in the group, zero means no group
    int probRecip;                  // 1 / p, zero means disable this entry

    int repeat;                     // try Repeat times for current item record, zero means disable this entry
    int value;                      // how many items to drop, gold means value, zero means disable this entry

    DropItemConfig(
            const char8_t *argMonsterName,
            const char8_t *argItemName,

            int argGroup,
            int argProbRecip,
            
            int argRepeat,
            int argValue)
        : monsterName(argMonsterName)
        , itemName(argItemName)
        , group(argGroup)
        , probRecip(argProbRecip)
        , repeat(argRepeat)
        , value(argValue)
    {}

    void print() const;

    // check if current entry is valid
    // 1. check member variables
    // 2. check if the provided names are legal and practical
    operator bool() const;
};
