local npc = {}
function npc.getUID(mapName, npcName)
    assertType(mapName, 'string')
    assertType(npcName, 'string')
end

function npc.runHandler(npcUID, playerUID, eventPath, event, value)
    assertType(npcUID, 'integer')
    assert(isNPChar(npcUID))

    assertType(playerUID, 'integer')
    assert(isPlayer(playerUID))

    assertType(eventPath, 'array')
    assertType(eventPath[1], 'string')

    assertType(event, 'string')
    assertType(value, 'string', 'nil')

    uidRemoteCall(npcUID, playerUID, eventPath, event, value,
    [[
        local playerUID, eventPath, event, value = ...
        runEventHandler(playerUID, eventPath, event, value)
    ]])
end
return npc
