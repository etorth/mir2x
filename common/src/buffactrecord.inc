{},

// 光环
{   .name = u8"恢复光环",
    .type = u8"光环",
    .aura
    {
        .self = 1,
    },
},

// 控制
{   .name = u8"眩晕",
    .type = u8"控制",
},

{   .name = u8"缠绕",
    .type = u8"控制",
    .controller
    {
        .can
        {
            .spell = 1,
        },
    },
},

{   .name = u8"束缚",
    .type = u8"控制",
},

{   .name = u8"妖术",
    .type = u8"控制",
},

{   .name = u8"龙卷风",
    .type = u8"控制",
},

{   .name = u8"隐藏",
    .type = u8"控制",
},

{   .name = u8"致盲",
    .type = u8"控制",
},

{   .name = u8"沉默",
    .type = u8"控制",
},

{   .name = u8"锁闭",
    .type = u8"控制",
},

{   .name = u8"破坏",
    .type = u8"控制",
},

{   .name = u8"缴械",
    .type = u8"控制",
},

{   .name = u8"减速",
    .type = u8"控制",
},

{   .name = u8"陷阱",
    .type = u8"控制",
},

{   .name = u8"嘲讽",
    .type = u8"控制",
},

{   .name = u8"恐惧",
    .type = u8"控制",
},

{   .name = u8"蛊惑",
    .type = u8"控制",
},

{   .name = u8"强制移动",
    .type = u8"控制",
},

// 触发
{   .name = u8"HP",
},

// 属性修改
{   .name = u8"HP_MODIFIER",
    .type = u8"属性修改",
},

{   .name = u8"MP_MODIFIER",
    .type = u8"属性修改",
},

{   .name = u8"HPMAX_MODIFIER",
    .type = u8"属性修改",
},

{   .name = u8"MPMAX_MODIFIER",
    .type = u8"属性修改",
},

{   .name = u8"DC下限",
    .type = u8"属性修改",
},

{   .name = u8"HP上限",
    .type = u8"属性修改",
},

// 攻击修改
{   .name = u8"重击",
    .type = u8"攻击修改",
    .attackModifier
    {
        .modifier = u8"重击",
    },
},

{   .name = u8"淬毒",
    .type = u8"攻击修改",
    .attackModifier
    {
        .modifier = u8"淬毒",
    },
},

// 施法修改
{   .name = u8"淬毒",
    .type = u8"攻击修改",
    .attackModifier
    {
        .modifier = u8"淬毒",
    },
},

{   .name = u8"传送",
},

{   .name = u8"隐身",
},

{   .name = u8"相位",
},

{   .name = u8"无敌",
    .controller
    {
        .can
        {
            .move    = 1,
            .attack  = 1,
            .spell   = 1,
            .hitted  = 1,
            .item    = 1,
        },
    },
},

{   .name = u8"技能免疫",
    .controller
    {
        .can
        {
            .move    = 1,
            .attack  = 1,
            .spell   = 1,
            .hitted  = 1,
            .item    = 1,
        },
    },
},

{   .name = u8"攻击免疫",
},

{   .name = u8"虚无",
},
