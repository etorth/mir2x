/*
 * =====================================================================================
 *
 *       Filename: magicrecord.inc
 *        Created: 08/04/2017 23:00:09
 *  Last Modified: 08/06/2017 17:15:30
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

    GfxEntry {u8"启动", 0X00000596, 12, 0, 1},
    GfxEntry {u8"运行", 0X000005AA,  5, 0, 1},
},

{
    u8"魔法盾",

    GfxEntry {u8"启动", 0X0000033E,  9, 0, 1},
    GfxEntry {u8"开始", 0X00000347, 10, 0, 1},
    GfxEntry {u8"运行", 0X00000352,  3, 0, 1},
    GfxEntry {u8"挨打", 0X00000355,  3, 0, 1},
},
