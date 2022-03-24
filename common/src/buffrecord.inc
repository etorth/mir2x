{},

{   .name = u8"治愈术",
    .favor = 1,
    .dispellable = 1,
    .icon
    {
        .show = 1,
        .gfxID = 0X18000083,
    },

    .actList
    {
        {   .name = u8"HP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(20),
            },
        },

        {   .name = u8"HP持续",
            .duration = 6000,
            .trigger
            {
                .on = BATGR_TIME,
                .tps = 2,
                .arg = buffArgWrapper<BuffValuePercentage>(20, 5),
            },
        },
    },
},

{   .name = u8"施毒术",
    .favor = -1,
    .dispellable = 1,
    .icon
    {
        .show = 1,
        .gfxID = 0X18000074,
    },

    .actList
    {
        {   .name = u8"HP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(-20),
            },
        },

        {   .name = u8"HP持续",
            .duration = 6000,
            .trigger
            {
                .on = BATGR_TIME,
                .tps = 2,
                .arg = buffArgWrapper<BuffValuePercentage>(0, -5),
            },
        },
    },
},

{   .name = u8"龙纹圣光",
    .favor = 1,
    .actList
    {
        {   .name = u8"恢复光环使能",
            .aura
            {
                .radius = 10,
                .linger
                {
                    .in = 500,
                    .out = 500,
                },
            },
        },
    },
},

{   .name = u8"恢复光环",
    .favor = 1,
    .icon
    {
        .show = 1,
        .gfxID = 0X1800000D,
    },

    .actList
    {
        {   .name = u8"HP恢复",
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(1, 5),
            },
        },
    },
},

{   .name = u8"死亡威慑",
    .favor = 1,
    .actList
    {
        {   .name = u8"战栗使能",
            .attackModifier
            {
                .prob = 100,
            },
        },
    },
},

{   .name = u8"战栗",
    .favor = 0,
    .icon
    {
        .show = 1,
        .gfxID = 0X18000065,
    },

    .actList
    {
        {   .name = u8"HP移动伤害",
            .duration = 6000,

            .trigger
            {
                .on = BATGR_MOVE,
                .arg = buffArgWrapper<int>(20),
            },
        },
    },
},

{   .name = u8"吸血鬼的诅咒",
    .favor = 1,
    .actList
    {
        {   .name = u8"吸血光环使能",
            .aura
            {
                .radius = 5,
                .linger
                {
                    .in = 500,
                    .out = 500,
                },
            },
        },
    },
},

{   .name = u8"吸血光环",
    .favor = 1,
    .icon
    {
        .show = 1,
        .gfxID = 0X1800011D,
    },

    .actList
    {
        {   .name = u8"吸血",
            .attackModifier
            {
                .prob = 100,
            },
        },
    },
},

{   .name = u8"幽灵盾",
    .favor = 1,
    .dispellable = 1,
    .icon
    {
        .show = 1,
        .gfxID = 0X18000200,
    },

    .actList
    {
        {   .name = u8"MAC下限",
            .duration = 30000,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(2, 5),
            },
        },

        {   .name = u8"MAC上限",
            .duration = 30000,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(2, 5),
            },
        },
    },
},

{   .name = u8"神圣战甲术",
    .favor = 1,
    .dispellable = 1,
    .icon
    {
        .show = 1,
        .gfxID = 0X18000232,
    },

    .actList
    {
        {   .name = u8"AC下限",
            .duration = 30000,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(2, 5),
            },
        },

        {   .name = u8"AC上限",
            .duration = 30000,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(2, 5),
            },
        },
    },
},

{   .name = u8"金创药（小）",
    .favor = 1,
    .stackReplace = 1,
    .actList
    {
        {   .name = u8"HP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(10),
            },
        },

        {   .name = u8"HP持续",
            .duration = 3000,
            .trigger
            {
                .on = BATGR_TIME,
                .tps = 2,
                .arg = buffArgWrapper<BuffValuePercentage>(20, 1),
            },
        },
    },
},

{   .name = u8"金创药（中）",
    .favor = 1,
    .stackReplace = 1,
    .actList
    {
        {   .name = u8"HP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(20),
            },
        },

        {   .name = u8"HP持续",
            .duration = 4000,
            .trigger
            {
                .on = BATGR_TIME,
                .tps = 2,
                .arg = buffArgWrapper<BuffValuePercentage>(20, 1),
            },
        },
    },
},

{   .name = u8"金创药（大）",
    .favor = 1,
    .stackReplace = 1,
    .actList
    {
        {   .name = u8"HP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(40),
            },
        },

        {   .name = u8"HP持续",
            .duration = 4000,
            .trigger
            {
                .on = BATGR_TIME,
                .tps = 2,
                .arg = buffArgWrapper<BuffValuePercentage>(20, 1),
            },
        },
    },
},

{   .name = u8"金创药（特）",
    .favor = 1,
    .stackReplace = 1,
    .actList
    {
        {   .name = u8"HP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(60),
            },
        },

        {   .name = u8"HP持续",
            .duration = 6000,
            .trigger
            {
                .on = BATGR_TIME,
                .tps = 2,
                .arg = buffArgWrapper<BuffValuePercentage>(20, 1),
            },
        },
    },
},

{   .name = u8"魔法药（小）",
    .favor = 1,
    .stackReplace = 1,
    .actList
    {
        {   .name = u8"MP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(10),
            },
        },

        {   .name = u8"MP持续",
            .duration = 3000,
            .trigger
            {
                .on = BATGR_TIME,
                .tps = 2,
                .arg = buffArgWrapper<BuffValuePercentage>(20, 1),
            },
        },
    },
},

{   .name = u8"魔法药（中）",
    .favor = 1,
    .stackReplace = 1,
    .actList
    {
        {   .name = u8"MP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(20),
            },
        },

        {   .name = u8"MP持续",
            .duration = 4000,
            .trigger
            {
                .on = BATGR_TIME,
                .tps = 2,
                .arg = buffArgWrapper<BuffValuePercentage>(20, 1),
            },
        },
    },
},

{   .name = u8"魔法药（大）",
    .favor = 1,
    .stackReplace = 1,
    .actList
    {
        {   .name = u8"MP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(40),
            },
        },

        {   .name = u8"MP持续",
            .duration = 4000,
            .trigger
            {
                .on = BATGR_TIME,
                .tps = 2,
                .arg = buffArgWrapper<BuffValuePercentage>(20, 1),
            },
        },
    },
},

{   .name = u8"魔法药（特）",
    .favor = 1,
    .stackReplace = 1,
    .actList
    {
        {   .name = u8"MP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(60),
            },
        },

        {   .name = u8"MP持续",
            .duration = 6000,
            .trigger
            {
                .on = BATGR_TIME,
                .tps = 2,
                .arg = buffArgWrapper<BuffValuePercentage>(20, 1),
            },
        },
    },
},

{   .name = u8"太阳水",
    .favor = 1,
    .stackReplace = 1,
    .actList
    {
        {   .name = u8"HP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(70),
            },
        },

        {   .name = u8"MP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(110),
            },
        },
    },
},

{   .name = u8"强效太阳水",
    .favor = 1,
    .stackReplace = 1,
    .actList
    {
        {   .name = u8"HP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(110),
            },
        },

        {   .name = u8"MP",
            .duration = 0,
            .attributeModifier
            {
                .arg = buffArgWrapper<BuffValuePercentage>(110),
            },
        },
    },
},
