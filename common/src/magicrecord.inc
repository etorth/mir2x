/*
 * =====================================================================================
 *
 *       Filename: magicrecord.inc
 *        Created: 08/04/2017 23:00:09
 *  Last Modified: 08/10/2017 17:11:05
 *
 *    Description: description of magic
 *                 1. prayer
 *                 2. magic
 *                 3. explosion
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

{
    u8"",

    // empty record
    // won't put any GfxEntry here
},

{
    u8"雷电术",

    GfxEntry {u8"启动", u8"附着", 0X00000596, 12,  0, 100, 0, 1},
    GfxEntry {u8"运行", u8"附着", 0X000005AA,  5, -1, 100, 0, 1},
},

{
    u8"魔法盾",

    GfxEntry {u8"启动", u8"附着", 0X0000033E,  9,  1, 100, 0, 1},
    GfxEntry {u8"开始", u8"附着", 0X00000347, 10, -1, 100, 0, 1},
    GfxEntry {u8"运行", u8"附着", 0X00000352,  3, -1, 100, 1, 1},
    GfxEntry {u8"挨打", u8"附着", 0X00000355,  3, -1, 100, 0, 1},
},

{
    u8"召唤骷髅",

    GfxEntry {u8"启动", u8"附着", 0X000002E4, 10,  1,  80, 0, 1},
    GfxEntry {u8"开始", u8"固定", 0X000002EE, 10, -1, 100, 0, 1},
},
