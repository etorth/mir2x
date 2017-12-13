-- =====================================================================================
--
--       Filename: 道馆.lua
--        Created: 08/31/2015 08:52:57 PM
--  Last Modified: 12/13/2017 01:50:33
--
--    Description: lua 5.3
--
--        Version: 1.0
--       Revision: none
--       Compiler: gcc
--
--         Author: ANHONG
--          Email: anhonghe@gmail.com
--   Organization: USTC
--
-- =====================================================================================

if nLastTime == nil then
    nLastTime = getTime()
end

if getTime() - nLastTime > 1000 then

    -- mark current time
    -- then next time we start from here

    nLastTime = getTime()

    if getMonsterCount(3) < 3 then

        addMonster(3, 400, 120, true)

    end

end
