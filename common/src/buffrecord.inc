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
                .percentage = 1,
                .value = 20,
            },
        },

        {   .name = u8"HP恢复",
            .attributeModifier
            {
                .percentage = 1,
                .value = 20,
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
                .value = -10,
            },
        },
    },
},

{   .name = u8"恢复光环",
    .favor = 1,
    .dispellable = 1,
    .icon
    {
        .show = 1,
        .gfxID = 0X1800000D,
    },

    .actList
    {
        {   .name = u8"恢复光环",
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
