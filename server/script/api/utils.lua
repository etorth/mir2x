local utils = {}

function utils.peerCount()
end

function utils.peerIndex()
end

function utils.findPeerCore()
end

function utils.findServiceCore()
end

function utils.findNPChar(mapName, npcName)
    assertType(mapname, 'string')
    assertType(npcname, 'string')

    local mapUID = _RSVD_NAME_callFuncCoop('loadMap', mapName)
    assertType(mapUID, 'integer', 'nil')

    if (not mapUID) or (mapUID == 0) then
        return nil
    end

    local npcUID = uidRemoteCall(mapUID, npcName,
    [[
        local npcName = ...
        return getNPCharUID(npcName)
    ]])

    assertType(npcUID, 'integer', 'nil')

    if (not npcUID) or (npcUID == 0) then
        return nil
    end

    return npcUID
end

return utils
