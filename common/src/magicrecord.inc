/*
 * =====================================================================================
 *
 *       Filename: magicrecord.inc
 *        Created: 08/04/2017 23:00:09
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
    // magicID = 0
    // first empty record
},

{
    .name = u8"雷电术",
    .elem = u8"雷",
    .icon = 0X0500100A,

    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X00000596,
            .frameCount = 12,
            .motion = 1,
        },

        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X000005AA,
            .frameCount = 5,
        },
    },
},

{
    .name = u8"魔法盾",
    .elem = u8"风",
    .icon = 0X0500101D,

    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X0000033E,
            .frameCount = 9,
            .motion = 1,
        },

        {
            .stage = u8"开始",
            .type  = u8"附着",
            .gfxID = 0X00000347,
            .frameCount = 10,
        },

        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X00000352,
            .frameCount = 3,
            .loop = 1,
        },

        {
            .stage = u8"挨打",
            .type  = u8"附着",
            .gfxID = 0X00000355,
            .frameCount = 3,
        },
    },
},

{
    .name = u8"召唤骷髅",
    .elem = u8"幻影",
    .icon = 0X0500103B,

    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X000002E4,
            .frameCount = 10,
            .motion = 1,
            .speed = 80,
        },

        {
            .stage = u8"开始",
            .type  = u8"固定",
            .gfxID = 0X000002EE,
            .frameCount = 10,
            .speed = 200,
        },
    }
},

{
    .name = u8"火球术",
},

{
    .name = u8"治愈术",
},

{
    .name = u8"基本剑术",
},

{
    .name = u8"精神力战法",
},

{
    .name = u8"大火球",
},

{
    .name = u8"施毒术",
},

{
    .name = u8"攻杀剑术",
},

{
    .name = u8"抗拒火环",
},

{
    .name = u8"地狱火",
},

{
    .name = u8"疾光电影",
},

{
    .name = u8"雷电术",
},

{
    .name = u8"刺杀剑术",
},

{
    .name = u8"灵魂火符",
    .elem = u8"暗黑",
    .icon = 0X0500100C,

    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X00000820,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 8,
            .motion = 0,
        },

        {
            .stage = u8"运行",
            .type  = u8"射击",
            .gfxID = 0X000003D4,
            .frameCount = 3,
            .gfxIDCount = 10,
            .gfxDirType = 16, //special, has 16 directions
            .loop = 1,
            .speed = 200,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X00000474,
            .frameCount = 10,
        },
    }
},

{
    .name = u8"幽灵盾",
},

{
    .name = u8"神圣战甲术",
},

{
    .name = u8"困魔咒",
},

{
    .name = u8"隐身术",
},

{
    .name = u8"集体隐身术",
},

{
    .name = u8"诱惑之光",
},

{
    .name = u8"瞬息移动",
    .elem = u8"幻影",

    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X0000006E,
            .frameCount = 10,
            .motion = 0,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X00000938,
            .frameCount = 10,
        },
    },
},

{
    .name = u8"火墙",
},

{
    .name = u8"爆裂火焰",
},

{
    .name = u8"地狱雷光",
},

{
    .name = u8"半月弯刀",
},

{
    .name = u8"烈火剑法",
},

{
    .name = u8"野蛮冲撞",
},

{
    .name = u8"心灵启示",
},

{
    .name = u8"群体治愈术",
},

{
    .name = u8"召唤神兽",
    .elem = u8"幻影",
    .icon = 0X0500101C,

    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X00000A1E,
            .frameCount = 19,
            .motion = 1,
            .speed = 180,
        }
    }
},

{
    .name = u8"魔法盾",
},

{
    .name = u8"圣言术",
},

{
    .name = u8"冰咆哮",
},

{
    .name = u8"莲月剑法",
},

{
    .name = u8"翔空剑法",
},

{
    .name = u8"空拳刀法",
},

{
    .name = u8"月魂断玉",
},

{
    .name = u8"月魂灵波",
},

{
    .name = u8"冰月神掌",
},

{
    .name = u8"冰月震天",
},

{
    .name = u8"霹雳掌",
},

{
    .name = u8"冰沙掌",
},

{
    .name = u8"风掌",
},

{
    .name = u8"龙卷风",
},

{
    .name = u8"风震天",
},

{
    .name = u8"击风",
},

{
    .name = u8"回生术",
},

{
    .name = u8"强魔震法",
},

{
    .name = u8"猛虎强势",
},

{
    .name = u8"铁布衫",
},

{
    .name = u8"十方斩",
},

{
    .name = u8"异形换位",
},

{
    .name = u8"超强召唤骷髅",
},

{
    .name = u8"破血狂杀",
},

{
    .name = u8"乾坤大挪移",
},

{
    .name = u8"斗转星移",
},

{
    .name = u8"魄冰刺",
},

{
    .name = u8"怒神霹雳",
},

{
    .name = u8"凝血离魂",
},

{
    .name = u8"焰天火雨",
},

{
    .name = u8"云寂术",
},

{
    .name = u8"妙影无踪",
},

{
    .name = u8"阴阳法环",
},

{
    .name = u8"移花接玉",
},

{
    .name = u8"神兽_喷火",
    .elem = u8"火",

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X030003D4,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        }
    },
},
