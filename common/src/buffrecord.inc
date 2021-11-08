{},

{   .name = u8"治愈术",
    .time = 6000,
    .gfxID = 0X18000083,

    .modList
    {
        {   .type = BMOD_HP,
            .arg = 20,
        },

        {   .type = BMOD_HPMAX,
            .arg = 200,
        },
    },

    .tgrList
    {
        {   .name = u8"HP_MODIFIER",
            .on = BTGR_TIME,
            .tps = 2,
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
            .arg = -20,
        },
    },

    .tgrList
    {
        {   .name = u8"HP_MODIFIER",
            .on = BTGR_TIME | BTGR_MOVE,
            .tps = 2,
            .arg = -5,
        },
    },
},
