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

{},

{   .name = u8"物理攻击", // TODO ...
    .castRange
    {
        .type = CRT_DIR,
        .distance = 1,
    },
},

{   .name = u8"雷电术",
    .elem = u8"雷",
    .req
    {
        .level = {16, 18, 20},
        .train = {40, 50, 60},
        .job = u8"法师",
    },
    .coolDown = 80,
    .mp = 7,
    .mpInc = 6,
    .power = {15, 19},
    .addPower = {7, 11},
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

{   .name = u8"魔法盾",
    .elem = u8"风",
    .req
    {
        .level = {29, 31, 33},
        .train = {90, 100, 110},
        .job = u8"法师",
    },
    .coolDown = 10,
    .mp = 20,
    .mpInc = 30,
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

{   .name = u8"召唤骷髅",
    .elem = u8"幻影",
    .req
    {
        .level = {17, 19, 21},
        .train = {40, 50, 60},
        .job = u8"道士",
    },
    .coolDown = 200,
    .mp = 15,
    .mpInc = 10,
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

{   .name = u8"火球术",
    .elem = u8"火",
    .req
    {
        .level = {7, 9, 11},
        .train = {10, 20, 30},
        .job = u8"法师",
    },
    .coolDown = 50,
    .checkGround = 1,
    .mp = 3,
    .mpInc = 1,
    .power = {6, 10},
    .addPower = {0, 4},
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X0000071C,
            .frameCount = 8,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },

        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X000001A4,
            .frameCount = 5,
            .gfxIDCount = 10,
            .gfxDirType = 16, //special, has 16 directions
            .loop = 1,
            .speed = 200,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X00000244,
            .frameCount = 10,
        },
    }
},

{   .name = u8"大火球",
    .elem = u8"火",
    .req
    {
        .level = {15, 17, 19},
        .train = {40, 50, 60},
        .job = u8"法师",
    },
    .coolDown = 70,
    .checkGround = 1,
    .mp = 6,
    .mpInc = 6,
    .power = {15, 19},
    .addPower = {7, 11},
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X00000618,
            .frameCount = 9,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },

        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X00000668,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 16, //special, has 16 directions
            .loop = 1,
            .speed = 200,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X00000708,
            .frameCount = 10,
        },
    }
},

{   .name = u8"治愈术",
    .elem = u8"神圣",
    .req
    {
        .level = {7, 9, 11},
        .train = {10, 20, 30},
        .job = u8"道士",
    },
    .coolDown = 40,
    .mp = 7,
    .mpInc = 2,
    .power = {11, 15},
},

{   .name = u8"基本剑术",
    .req
    {
        .level = {7, 9, 11},
        .train = {10, 20, 30},
        .job = u8"战士",
    },
},

{   .name = u8"精神力战法",
    .req
    {
        .level = {8, 10, 12},
        .train = {10, 20, 30},
        .job = u8"道士",
    },
},

{   .name = u8"施毒术",
    .elem = u8"暗黑",
    .req
    {
        .level = {12, 14, 16},
        .train = {20, 30, 40},
        .job = u8"道士",
    },
    .coolDown = 60,
    .mp = 10,
    .mpInc = 5,
},

{   .name = u8"攻杀剑术",
    .req
    {
        .level = {14, 16, 18},
        .train = {30, 40, 50},
        .job = u8"战士",
    },
},

{   .name = u8"抗拒火环",
    .elem = u8"风",
    .req
    {
        .level = {12, 14, 16},
        .train = {20, 30, 40},
        .job = u8"法师",
    },
    .coolDown = 60,
    .mp = 8,
    .mpInc = 1,
    .power = {0, 2},
},

{   .name = u8"疾光电影",
    .elem = u8"雷",
    .req
    {
        .level = {21, 23, 25},
        .train = {50, 60, 70},
        .job = u8"法师",
    },
    .coolDown = 80,
    .mp = 12,
    .mpInc = 15,
    .power = {14, 14},
    .addPower = {12, 16},
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X000007B2,
            .frameCount = 10,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },

        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X0100049C,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },
    }
},

{   .name = u8"刺杀剑术",
    .req
    {
        .level = {19, 21, 23},
        .train = {40, 50, 60},
        .job = u8"战士",
    },
},

{   .name = u8"灵魂火符",
    .elem = u8"暗黑",
    .req
    {
        .level = {13, 15, 17},
        .train = {20, 30, 40},
        .job = u8"道士",
    },
    .coolDown = 50,
    .checkGround = 1,
    .mp = 6,
    .mpInc = 3,
    .power = {6, 10},
    .addPower = {3, 3},
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X00000820,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },

        {
            .stage = u8"运行",
            .type  = u8"跟随",
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

{   .name = u8"幽灵盾",
    .elem = u8"暗黑",
    .req
    {
        .level = {21, 23, 25},
        .train = {50, 60, 70},
        .job = u8"道士",
    },
    .coolDown = 70,
    .mp = 10,
    .mpInc = 5,
},

{   .name = u8"神圣战甲术",
    .elem = u8"暗黑",
    .req
    {
        .level = {25, 27, 29},
        .train = {70, 80, 90},
        .job = u8"道士",
    },
    .coolDown = 70,
    .mp = 10,
    .mpInc = 5,
},

{   .name = u8"困魔咒",
    .elem = u8"暗黑",
    .req
    {
        .level = {27, 29, 31},
        .train = {80, 90, 100},
        .job = u8"道士",
    },
    .coolDown = 100,
    .mp = 15,
    .mpInc = 10,
},

{   .name = u8"隐身术",
    .elem = u8"暗黑",
    .req
    {
        .level = {20, 22, 24},
        .train = {50, 60, 70},
        .job = u8"道士",
    },
    .coolDown = 70,
    .mp = 5,
    .mpInc = 5,
},

{   .name = u8"集体隐身术",
    .elem = u8"暗黑",
    .req
    {
        .level = {23, 25, 27},
        .train = {60, 70, 80},
        .job = u8"道士",
    },
    .coolDown = 70,
    .mp = 10,
    .mpInc = 5,
},

{   .name = u8"诱惑之光",
    .elem = u8"幻影",
    .req
    {
        .level = {13, 15, 17},
        .train = {20, 30, 40},
        .job = u8"法师",
    },
    .coolDown = 60,
    .mp = 3,
    .mpInc = 3,
},

{   .name = u8"瞬息移动",
    .elem = u8"幻影",
    .req
    {
        .level = {14, 16, 18},
        .train = {30, 40, 50},
        .job = u8"法师",
    },
    .coolDown = 70,
    .mp = 10,
    .mpInc = 10,
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X0000006E,
            .frameCount = 10,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X00000938,
            .frameCount = 10,
        },
    },
},

{   .name = u8"火墙",
    .elem = u8"火",
    .req
    {
        .level = {24, 26, 28},
        .train = {60, 70, 80},
        .job = u8"法师",
    },
    .coolDown = 100,
    .mp = 22,
    .mpInc = 30,
    .power = {2, 9},
    .addPower = {1, 6},
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X0000038E,
            .frameCount = 10,
            .motion = 1,
        },

        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X00000398,
            .frameCount = 5,
            .onGround = 1,
            .loop = 1,
            .speed = 50,
        },
    },
},

{   .name = u8"地狱火",
    .elem = u8"火",
    .req
    {
        .level = {20, 22, 24},
        .train = {50, 60, 70},
        .job = u8"法师",
    },
    .coolDown = 80,
    .mp = 11,
    .mpInc = 15,
    .power = {14, 18},
    .addPower = {12, 16},
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X0000071C,
            .frameCount = 8,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },

        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X0000076C,
            .frameCount = 30,
            .gfxDirType = 2,
            .onGround = 1,
            .speed = 200,
        },
    },
},

{   .name = u8"火焰灰烬", // magic effect, not for specific magic
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X00000992,
            .frameCount = 10,
            .gfxDirType = 5,
            .onGround = 1,
            .loop = 1,
        },
    },
},

{   .name = u8"冰刺", // magic effect, used for 冰沙掌 and 魄冰刺
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X0100005A,
            .frameCount = 20,
            .gfxDirType = 2,
            .onGround = 1,
            .speed = 200,
        },
    },
},

{   .name = u8"爆裂火焰",
    .elem = u8"火",
    .req
    {
        .level = {32, 34, 36},
        .train = {100, 110, 120},
        .job = u8"法师",
    },
    .coolDown = 90,
    .mp = 15,
    .mpInc = 20,
    .power = {14, 18},
    .addPower = {14, 18},
},

{   .name = u8"地狱雷光",
    .elem = u8"雷",
    .req
    {
        .level = {33, 35, 37},
        .train = {100, 110, 120},
        .job = u8"法师",
    },
    .coolDown = 90,
    .mp = 17,
    .mpInc = 20,
    .power = {14, 18},
    .addPower = {14, 18},
},

{   .name = u8"半月弯刀",
    .req
    {
        .level = {24, 26, 28},
        .train = {60, 70, 80},
        .job = u8"战士",
    },
    .mpInc = 3,
},

{   .name = u8"烈火剑法",
    .req
    {
        .level = {32, 34, 36},
        .train = {100, 110, 120},
        .job = u8"战士",
    },
    .mp = 2,
    .mpInc = 7,
},

{   .name = u8"野蛮冲撞",
    .req
    {
        .level = {27, 29, 31},
        .train = {70, 80, 90},
        .job = u8"战士",
    },
    .mp = 20,
},

{   .name = u8"群体治愈术",
    .elem = u8"神圣",
    .req
    {
        .level = {31, 33, 35},
        .train = {100, 110, 120},
        .job = u8"道士",
    },
    .coolDown = 40,
    .mp = 10,
    .mpInc = 20,
    .power = {8, 12},
    .addPower = {4, 4},
},

{   .name = u8"召唤神兽",
    .elem = u8"幻影",
    .req
    {
        .level = {30, 32, 34},
        .train = {90, 100, 110},
        .job = u8"道士",
    },
    .coolDown = 200,
    .mp = 15,
    .mpInc = 15,
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

{   .name = u8"圣言术",
    .elem = u8"幻影",
    .req
    {
        .level = {26, 28, 30},
        .train = {70, 80, 90},
        .job = u8"法师",
    },
    .coolDown = 100,
    .mp = 30,
    .mpInc = 30,
},

{   .name = u8"冰咆哮",
    .elem = u8"冰",
    .req
    {
        .level = {34, 36, 38},
        .train = {100, 110, 120},
        .job = u8"法师",
    },
    .coolDown = 90,
    .mp = 19,
    .mpInc = 20,
    .power = {12, 16},
    .addPower = {14, 18},
},

{   .name = u8"莲月剑法",
    .req
    {
        .level = {38, 40, 42},
        .train = {100, 110, 120},
        .job = u8"战士",
    },
    .mp = 3,
    .mpInc = 9,
},

{   .name = u8"翔空剑法",
    .req
    {
        .level = {35, 37, 39},
        .train = {100, 110, 120},
        .job = u8"战士",
    },
    .mp = 2,
    .mpInc = 8,
},

{   .name = u8"空拳刀法",
    .req
    {
        .level = {28, 30, 32},
        .train = {90, 100, 110},
        .job = u8"道士",
    },
    .coolDown = 80,
    .mp = 20,
    .mpInc = 10,
},

{   .name = u8"月魂断玉",
    .elem = u8"神圣",
    .req
    {
        .level = {14, 16, 18},
        .train = {30, 40, 50},
        .job = u8"道士",
    },
    .coolDown = 50,
    .mp = 6,
    .mpInc = 3,
    .power = {6, 10},
    .addPower = {3, 3},
},

{   .name = u8"月魂灵波",
    .elem = u8"神圣",
    .req
    {
        .level = {24, 26, 28},
        .train = {60, 70, 80},
        .job = u8"道士",
    },
    .mp = 8,
    .mpInc = 4,
    .power = {8, 12},
    .addPower = {7, 7},
},

{   .name = u8"冰月神掌",
    .elem = u8"冰",
    .req
    {
        .level = {9, 11, 13},
        .train = {10, 20, 30},
        .job = u8"法师",
    },
    .coolDown = 50,
    .checkGround = 1,
    .mp = 4,
    .mpInc = 1,
    .power = {4, 8},
    .addPower = {0, 4},
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X00000A3C,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },

        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X00000A8C,
            .frameCount = 3,
            .gfxIDCount = 10,
            .gfxDirType = 16, //special, has 16 directions
            .loop = 1,
            .speed = 200,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X00000B2C,
            .frameCount = 10,
        },
    }
},

{   .name = u8"冰月震天",
    .elem = u8"冰",
    .req
    {
        .level = {17, 19, 21},
        .train = {40, 50, 60},
        .job = u8"法师",
    },
    .coolDown = 70,
    .checkGround = 1,
    .mp = 7,
    .mpInc = 6,
    .power = {13, 17},
    .addPower = {7, 11},
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X00000B40,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },

        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X00000B90,
            .frameCount = 6,
            .loop = 1,
            .speed = 200,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X00000B9A,
            .frameCount = 10,
        },
    }
},

{   .name = u8"霹雳掌",
    .elem = u8"雷",
    .req
    {
        .level = {8, 10, 12},
        .train = {10, 20, 30},
        .job = u8"法师",
    },
    .coolDown = 50,
    .mp = 4,
    .mpInc = 1,
    .power = {6, 10},
    .addPower = {0, 4},
},

{   .name = u8"冰沙掌",
    .elem = u8"冰",
    .req
    {
        .level = {22, 24, 26},
        .train = {50, 60, 70},
        .job = u8"法师",
    },
    .coolDown = 80,
    .checkGround = 1,
    .mp = 12,
    .mpInc = 15,
    .power = {12, 16},
    .addPower = {12, 16},
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X01000000,
            .frameCount = 10,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },

        {
            .stage = u8"运行",  // gfx uses 冰刺
            .type  = u8"固定",
            .onGround = 1,
            .speed = 200,
        },
    }
},

{   .name = u8"风掌",
    .elem = u8"风",
    .req
    {
        .level = {10, 12, 14},
        .train = {10, 20, 30},
        .job = u8"法师",
    },
    .coolDown = 50,
    .mp = 3,
    .mpInc = 1,
    .power = {5, 9},
    .addPower = {0, 4},
},

{   .name = u8"龙卷风",
    .elem = u8"风",
    .req
    {
        .level = {35, 37, 39},
        .train = {100, 110, 120},
        .job = u8"法师",
    },
    .coolDown = 90,
    .mp = 18,
    .mpInc = 20,
    .power = {13, 17},
    .addPower = {14, 18},
},

{   .name = u8"风震天",
    .elem = u8"风",
    .req
    {
        .level = {23, 25, 27},
        .train = {60, 70, 80},
        .job = u8"法师",
    },
    .coolDown = 80,
    .mp = 13,
    .mpInc = 15,
    .power = {13, 17},
    .addPower = {12, 16},
},

{   .name = u8"击风",
    .elem = u8"风",
    .req
    {
        .level = {18, 20, 22},
        .train = {40, 50, 60},
        .job = u8"法师",
    },
    .coolDown = 80,
    .mp = 6,
    .mpInc = 6,
    .power = {14, 18},
    .addPower = {7, 11},
},

{   .name = u8"回生术",
    .elem = u8"神圣",
    .req
    {
        .level = {35, 37, 39},
        .train = {50, 60, 70},
        .job = u8"道士",
    },
    .coolDown = 500,
    .mp = 100,
    .mpInc = 100,
},

{   .name = u8"强魔震法",
    .elem = u8"暗黑",
    .req
    {
        .level = {29, 31, 33},
        .train = {90, 100, 110},
        .job = u8"道士",
    },
    .coolDown = 70,
    .mp = 10,
    .mpInc = 5,
},

{   .name = u8"猛虎强势",
    .elem = u8"暗黑",
    .req
    {
        .level = {34, 36, 38},
        .train = {100, 110, 120},
        .job = u8"道士",
    },
    .coolDown = 70,
    .mp = 10,
    .mpInc = 5,
},

{   .name = u8"铁布衫",
    .elem = u8"暗黑",
    .req
    {
        .level = {44, 47, 50},
        .train = {1000, 2000, 3000},
        .job = u8"战士",
    },
    .coolDown = 100,
    .mp = 40,
    .mpInc = 20,
    .power = {2, 4},
    .addPower = {3, 5},
},

{   .name = u8"十方斩",
    .req
    {
        .level = {40, 43, 46},
        .train = {500, 800, 1000},
        .job = u8"战士",
    },
    .mpInc = 7,
},

{   .name = u8"异形换位",
    .elem = u8"幻影",
    .req
    {
        .level = {27, 29, 31},
        .train = {80, 90, 100},
        .job = u8"法师",
    },
    .coolDown = 250,
    .mp = 20,
    .mpInc = 20,
},

{   .name = u8"超强召唤骷髅",
    .elem = u8"幻影",
    .req
    {
        .level = {33, 35, 37},
        .train = {100, 110, 120},
        .job = u8"道士",
    },
    .coolDown = 200,
    .mp = 20,
    .mpInc = 25,
},

{   .name = u8"破血狂杀",
    .elem = u8"暗黑",
    .req
    {
        .level = {48, 51, 54},
        .train = {1000, 2000, 3000},
        .job = u8"战士",
    },
    .coolDown = 200,
    .mp = 100,
    .mpInc = 100,
    .power = {2, 4},
    .addPower = {3, 5},
},

{   .name = u8"乾坤大挪移",
    .req
    {
        .level = {42, 45, 48},
        .train = {500, 800, 1000},
        .job = u8"战士",
    },
    .coolDown = 20,
    .mp = 40,
    .mpInc = 10,
},

{   .name = u8"斗转星移",
    .req
    {
        .level = {46, 49, 52},
        .train = {500, 800, 1000},
        .job = u8"战士",
    },
    .coolDown = 20,
    .mp = 40,
    .mpInc = 20,
},

{   .name = u8"魄冰刺",
    .elem = u8"冰",
    .req
    {
        .level = {38, 41, 44},
        .train = {500, 800, 1000},
        .job = u8"法师",
    },
    .coolDown = 10,
    .mp = 20,
    .mpInc = 20,
    .power = {16, 20},
    .addPower = {16, 20},
},

{   .name = u8"怒神霹雳",
    .elem = u8"雷",
    .req
    {
        .level = {40, 42, 44},
        .train = {500, 800, 1000},
        .job = u8"法师",
    },
    .coolDown = 10,
    .mp = 40,
    .mpInc = 30,
    .power = {20, 40},
    .addPower = {20, 30},
},

{   .name = u8"凝血离魂",
    .elem = u8"暗黑",
    .req
    {
        .level = {46, 48, 50},
        .train = {1000, 2000, 3000},
        .job = u8"法师",
    },
    .coolDown = 10,
    .mp = 60,
    .mpInc = 10,
    .power = {6, 2},
    .addPower = {1, 1},
},

{   .name = u8"焰天火雨",
    .elem = u8"火",
    .req
    {
        .level = {43, 45, 47},
        .train = {1000, 2000, 3000},
        .job = u8"法师",
    },
    .coolDown = 10,
    .mp = 32,
    .mpInc = 20,
    .power = {32, 40},
    .addPower = {14, 22},
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X00000618,
            .frameCount = 9,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },

        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X00000668,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 16, //special, has 16 directions
            .loop = 1,
            .speed = 200,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X00000708,
            .frameCount = 10,
        },
    },
},

{   .name = u8"云寂术",
    .elem = u8"暗黑",
    .req
    {
        .level = {38, 41, 44},
        .train = {500, 800, 1000},
        .job = u8"道士",
    },
    .coolDown = 10,
    .mp = 20,
    .mpInc = 10,
},

{   .name = u8"妙影无踪",
    .elem = u8"暗黑",
    .req
    {
        .level = {43, 45, 47},
        .train = {1000, 2000, 3000},
        .job = u8"道士",
    },
    .coolDown = 10,
    .mp = 120,
    .mpInc = 80,
},

{   .name = u8"阴阳法环",
    .elem = u8"风",
    .req
    {
        .level = {46, 48, 50},
        .train = {1000, 2000, 3000},
        .job = u8"道士",
    },
    .coolDown = 20,
    .mp = 60,
    .mpInc = 50,
},

{   .name = u8"移花接玉",
    .elem = u8"暗黑",
    .req
    {
        .level = {40, 42, 44},
        .train = {500, 800, 1000},
        .job = u8"道士",
    },
    .coolDown = 20,
    .mp = 40,
    .mpInc = 30,
    .power = {4, 8},
    .addPower = {3, 4},
},

{   .name = u8"神兽_喷火",
    .elem = u8"火",
    .castRange
    {
        .type = CRT_DIR,
        .distance = 2,
    },

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

{   .name = u8"沙漠石人_石坑",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X0F0000C8,
            .frameCount = 1,
            .gfxDirType = 8,
            .onGround = 1,
            .loop = 1,
        },
    },
},

{   .name = u8"沙漠石人_死亡特效",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X03000132,
            .frameCount = 8,
            .gfxDirType = 1,
        },
    },
},

{   .name = u8"雷电僵尸_雷电",
    .elem = u8"雷",

    .castRange
    {
        .type = CRT_DIR,
        .distance = 8,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X03000258,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },
    },
},

{   .name = u8"僧侣僵尸_地洞",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X0F0000F0,
            .frameCount = 1,
            .gfxDirType = 8,
            .onGround = 1,
            .loop = 1,
        },
    },
},

{   .name = u8"沙漠树魔_喷刺",
    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 5,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X030003C0,
            .frameCount = 1,
            .gfxIDCount = 1,
            .gfxDirType = 1,
            .loop = 1,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X010003A2,
            .frameCount = 6,
        },
    },
},

{   .name = u8"楔蛾_喷毒",
    .castRange
    {
        .type = CRT_DIR,
        .distance = 1,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X03000370,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },
    },
},

{   .name = u8"掷斧骷髅_掷斧",
    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 5,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X03000320,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 8,
            .loop = 1,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X010003A2,
            .frameCount = 6,
        },
    },
},

{   .name = u8"诺玛法老_火球术",
    .elem = u8"火",

    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 8,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X000001A4,
            .frameCount = 5,
            .gfxIDCount = 10,
            .gfxDirType = 16, //special, has 16 directions
            .loop = 1,
            .speed = 200,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X00000244,
            .frameCount = 10,
        },
    }
},

{   .name = u8"诺玛大法老_雷电术",
    .elem = u8"雷",

    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 10,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X000005AA,
            .frameCount = 5,
        },
    },
},

{   .name = u8"稻草人_死亡特效",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着", // may also make it as 固定, magic of type 附着 only get draw when monster body is still visible
            .gfxID = 0X030002A8,
            .frameCount = 10,
        }
    },
},

{   .name = u8"轻甲守卫_死亡特效",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着", // may also make it as 固定, magic of type 附着 only get draw when monster body is still visible
            .gfxID = 0X0300003C,
            .frameCount = 18,
        }
    },
},

{   .name = u8"蚂蚁道士_治疗",
    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 8,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X03000064,
            .frameCount = 10,
        }
    },
},

{   .name = u8"红衣法师_魔法",
    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 8,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X03000118,
            .frameCount = 10,
        }
    },
},

{   .name = u8"红衣法师_死亡特效",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X0300003C,
            .frameCount = 18,
        }
    },
},

{   .name = u8"骷髅精灵_死亡特效",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X03000780,
            .frameCount = 10,
        }
    },
},

{   .name = u8"洞蛆_喷毒",
    .castRange
    {
        .type = CRT_DIR,
        .distance = 1,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X03000794,
            .frameCount = 5,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },
    },
},

{   .name = u8"火焰沃玛_喷火",
    .castRange
    {
        .type = CRT_DIR,
        .distance = 1,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X03000208,
            .frameCount = 7,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },
    },
},

{   .name = u8"沃玛教主_电光",
    .elem = u8"雷",

    .castRange
    {
        .type = CRT_DIR,
        .distance = 1,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X030001B8,
            .frameCount = 6,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },
    },
},

{   .name = u8"沃玛教主_雷电术",
    .elem = u8"雷",

    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 10,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X000005AA,
            .frameCount = 5,
        },
    },
},

{   .name = u8"暗黑战士_喷刺",
    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 8,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X030004D8,
            .frameCount = 1,
            .gfxIDCount = 10,
            .gfxDirType = 16,
            .loop = 1,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X010003A2,
            .frameCount = 6,
        },
    },
},

{   .name = u8"粪虫_喷毒",
    .castRange
    {
        .type = CRT_DIR,
        .distance = 1,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X030007EE,
            .frameCount = 5,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },
    },
},

{   .name = u8"祖玛弓箭手_射箭",
    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 8,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X03000578,
            .frameCount = 1,
            .gfxIDCount = 10,
            .gfxDirType = 16,
            .loop = 1,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X010003A2,
            .frameCount = 6,
        },
    },
},

{   .name = u8"祖玛教主_石像碎片",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"固定",
            .gfxID = 0X0F0000D2,
            .frameCount = 1,
            .gfxDirType = 1,
            .onGround = 1,
            .loop = 1,
        },
    },
},

{   .name = u8"祖玛教主_施法",
    .gfxList
    {
        {
            .stage = u8"启动",
            .type  = u8"附着",
            .gfxID = 0X030002D0,
            .frameCount = 8,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },
    },
},

{   .name = u8"爆毒蚂蚁_喷毒",
    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 5,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X03000050,
            .frameCount = 6,
            .gfxDirType = 1,
            .loop = 1,
        },
    },
},

{   .name = u8"沙漠风魔_扇风",
    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 5,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X03000622,
            .frameCount = 10,
            .gfxDirType = 1,
        },
    },
},

{   .name = u8"神舰守卫_死亡特效",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着", // may also make it as 固定, magic of type 附着 only get draw when monster body is still visible
            .gfxID = 0X03000104,
            .frameCount = 6,
        },
    },
},

{   .name = u8"月魔蜘蛛_死亡特效",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着", // may also make it as 固定, magic of type 附着 only get draw when monster body is still visible
            .gfxID = 0X030008CA,
            .frameCount = 20,
        },
    },
},

{   .name = u8"爆裂蜘蛛_死亡特效",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着", // may also make it as 固定, magic of type 附着 only get draw when monster body is still visible
            .gfxID = 0X030008A2,
            .frameCount = 7,
        },
    },
},

{   .name = u8"霸王教主_火刃",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X03000708,
            .frameCount = 7, // skip last 3 frames
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },
    },
},

{   .name = u8"霸王教主_野蛮冲撞",
    .castRange
    {
        .type = CRT_DIR,
        .distance = 5,
    },
},

{   .name = u8"霸王教主_死亡特效",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着", // may also make it as 固定, magic of type 附着 only get draw when monster body is still visible
            .gfxID = 0X03000762,
            .frameCount = 18,
            .gfxDirType = 1,
        },
    },
},

{   .name = u8"潘夜右护卫_死亡特效",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X0400005A,
            .frameCount = 5,
            .gfxDirType = 1,
        },
    },
},

{   .name = u8"潘夜右护卫_电魔杖",
    .castRange
    {
        .type = CRT_DIR,
        .distance = 1,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X04000003,
            .frameCount = 3,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X0100033E,
            .frameCount = 6,
            .gfxDirType = 1,
        },
    },
},

{   .name = u8"潘夜右护卫_雷电术",
    .elem = u8"雷",

    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 10,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X000005AA,
            .frameCount = 5,
        },
    },
},

{   .name = u8"潘夜左护卫_死亡特效",
    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X040000C8,
            .frameCount = 5,
            .gfxDirType = 1,
        },
    },
},

{   .name = u8"潘夜左护卫_火魔杖",
    .castRange
    {
        .type = CRT_DIR,
        .distance = 1,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"附着",
            .gfxID = 0X04000071,
            .frameCount = 3,
            .gfxIDCount = 10,
            .gfxDirType = 8,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X01000316,
            .frameCount = 6,
            .gfxDirType = 1,
        },
    },
},

{   .name = u8"潘夜左护卫_火球术",
    .elem = u8"火",

    .castRange
    {
        .type = CRT_LIMITED,
        .distance = 8,
    },

    .gfxList
    {
        {
            .stage = u8"运行",
            .type  = u8"跟随",
            .gfxID = 0X000001A4,
            .frameCount = 5,
            .gfxIDCount = 10,
            .gfxDirType = 16, //special, has 16 directions
            .loop = 1,
            .speed = 200,
        },

        {
            .stage = u8"结束",
            .type  = u8"附着",
            .gfxID = 0X00000244,
            .frameCount = 10,
        },
    }
},
