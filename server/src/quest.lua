--, u8R"###(
--

function getNPCharUID(mapName, npcName)
    local mapUID = loadMap(mapName)
    assertType(mapUID, 'integer')

    if mapUID == 0 then
        return 0
    end

    local npcUID = uidExecute(mapUID, [[
        return getNPCharUID('%s')
    ]], npcName)

    assertType(npcUID, 'integer')
    return npcUID
end

--
-- )###"
