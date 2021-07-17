local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沙巴克城_3
{
    {
        name = '七点白蛇',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 5, time = 3600, cratio = 0},
        }
    },
    {
        name = '千年毒蛇',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 1200, cratio = 0},
        }
    },
    {
        name = '半兽人',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
        }
    },
    {
        name = '多角虫',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
        }
    },
    {
        name = '多角虫0',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '夜行鬼09',
        loc = {
            {x = 300, y = 300, w = 300, h = 300, count = 100, time = 1200, cratio = 0},
        }
    },
    {
        name = '威思而小虫',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
        }
    },
    {
        name = '栗子树',
        loc = {
            {x = 300, y = 300, w = 300, h = 300, count = 300, time = 300, cratio = 0},
        }
    },
    {
        name = '森林雪人',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
        }
    },
    {
        name = '森林雪人0',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '沙漠威思而小虫',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
        }
    },
    {
        name = '狼',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
        }
    },
    {
        name = '猎鹰',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
        }
    },
    {
        name = '猎鹰0',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '盔甲虫',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
        }
    },
    {
        name = '盔甲虫0',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '红蛇',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
        }
    },
    {
        name = '葛贰厘面0',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '虎蛇',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
        }
    },
    {
        name = '虎蛇0',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 1, time = 7200, cratio = 0},
        }
    },
    {
        name = '食人花',
        loc = {
            {x = 300, y = 300, w = 250, h = 250, count = 15, time = 600, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
