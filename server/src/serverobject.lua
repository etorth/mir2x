--, u8R"###(
--

function loadMap(mapName)
    assertType(mapName, 'integer', 'string')
    local mapUID = _RSVD_NAME_callFuncCoop('loadMap', mapName)

    assertType(mapUID, 'integer', 'nil')
    return mapUID
end

function waitActivated()
    _RSVD_NAME_callFuncCoop('waitActivated')
end

--
-- )###"
