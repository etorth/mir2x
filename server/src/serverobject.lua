--, u8R"###(
--

function loadMap(mapName)
    assertType(mapName, 'string')
    local mapUID = _RSVD_NAME_callFuncCoop('loadMap', mapName)

    assertType(mapUID, 'integer', 'nil')
    return mapUID
end

--
-- )###"
