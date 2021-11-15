{},

{   .name = u8"治愈术",
    .duration = 6000,
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
            .attributeModifier
            {
                .value = 20,
            },
        },

        {   .name = u8"HP持续",
            .attributeModifier
            {
                .value = 20,
                .percentage = 1,
            },
        },
    },
},

{   .name = u8"施毒术",
    .duration = 6000,
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
            .attributeModifier
            {
                .value = -20,
            },
        },

        {   .name = u8"HP持续",
            .attributeModifier
            {
                .value = -20,
                .percentage = -1,
            },
        },
    },
},

{   .name = u8"恢复光环使能",
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
        {   .name = u8"HP持续",
            .attributeModifier
            {
                .percentage = 5,
            },
        },
    },
},

{   .name = u8"幽灵盾",
    .duration = 30,
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
            .attributeModifier
            {
                .value = 2,
                .percentage = 5,
            },
        },

        {   .name = u8"MAC上限",
            .attributeModifier
            {
                .value = 2,
                .percentage = 5,
            },
        },
    },
},

{   .name = u8"神圣战甲术",
    .duration = 30,
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
            .attributeModifier
            {
                .value = 2,
                .percentage = 5,
            },
        },

        {   .name = u8"AC上限",
            .attributeModifier
            {
                .value = 2,
                .percentage = 5,
            },
        },
    },
},
