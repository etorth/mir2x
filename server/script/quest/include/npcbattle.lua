-- a quest NPC that drops the act and turns hostile
--
-- legacy made the NPC vanish and spawned a monster where it stood. mir2x NPCs cannot despawn
-- yet, so the NPC keeps standing there but stops talking: its per-player handler is removed,
-- and since these quest NPCs carry no default handler a click falls through to npchar.lua's
-- 我听不懂你在说什么。
--
-- TODO despawn the NPC for real once that is supported, then this is only the spawn

local npcbattle = {}

-- returns the monster uid, or nil when the map could not be loaded
--
--     npcbattle.turnHostile
--     {
--         map     = '连接通路_E402_001',
--         npc     = '署箭_1',
--         uid     = uid,
--         monster = '僧侣僵尸',            -- a name, or a list of them
--         x       = 13,
--         y       = 10,
--         say     = '署箭口中念念有词，一具僵尸从地里爬了出来！',
--     }
function npcbattle.turnHostile(args)
    assertType(args, 'table')
    assertType(args.map, 'string')
    assertType(args.npc, 'string')
    assertType(args.uid, 'integer')
    assertType(args.monster, 'string', 'table')
    assertType(args.x, 'integer')
    assertType(args.y, 'integer')
    assertType(args.say, 'string', 'nil')

    local monsterList = (type(args.monster) == 'string') and {args.monster} or args.monster
    for _, monster in ipairs(monsterList) do
        assertType(monster, 'string')
        if getMonsterID(monster) <= 0 then
            fatalPrintf('Can not spawn unknown monster %s', monster)
        end
    end

    -- it has nothing left to say to this player
    clearNPCQuestBehavior(args.map, args.npc, args.uid)

    local mapUID = getMapUID(args.map)
    if not mapUID then
        fatalPrintf('Can not load map %s', args.map)
        return nil
    end

    if args.say then
        server.player.postString(args.uid, args.say)
    end

    -- loose placement, so a pack does not pile onto one grid
    return uidRemoteCall(mapUID, monsterList, args.x, args.y,
    [[
        local monsterList, x, y = ...
        local uidList = {}
        for _, monster in ipairs(monsterList) do
            table.insert(uidList, addMonster(monster, x, y, false))
        end
        return uidList
    ]])
end

return npcbattle
