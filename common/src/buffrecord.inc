{},

{   .name = u8"治愈术",
    .time = 6000,
    .gfxID = 0X18000083,

    .modList
    {
        {   .type = BMOD_HP,
            .arg = 5,
        },
    },

    .tgrList
    {
        {   .name = u8"HP_MODIFIER",
            .arg = 5,
        },
    },
},

{   .name = u8"施毒术",
    .time = 6000,
    .gfxID = 0X18000074,

    .modList
    {
        {   .type = BMOD_HP,
            .arg = -5,
        },
    },

    .tgrList
    {
        {   .name = u8"HP_MODIFIER",
            .arg = -5,
        },
    },
},
