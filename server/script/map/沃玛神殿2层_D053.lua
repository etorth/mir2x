local addmon = require('map.addmonster')
local addMonCo = addmon.monGener( -- 沃玛神殿2层_D053
{
    {
        name = '沃玛卫士',
        loc = {
            {x = 200, y = 200, w = 190, h = 190, count = 1, time = 7200, cratio = 0},
        }
    },
})

function main()
    while true do
        coroutine.resume(addMonCo)
        asyncWait(1000 * 5)
    end
end
