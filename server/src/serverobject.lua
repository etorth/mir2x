-- for map load/release, provide:
--
--     loadBaseMap(mapVar)
--     loadInstanceMap(mapVar)
--     closeInstanceMap(mapUID, exitMapVar, exitX, exitY)
--
-- we don't provide closeBaseMap(), base maps can not be closed by scripts
-- servicecore close all base maps when the service is shutting down, after all player are kicked offline

function loadBaseMap(mapVar)
    assertType(mapVar, 'integer', 'string')
    return assertType(_RSVD_NAME_callFuncCoop('loadMap', mapVar, true), 'integer', 'nil')
end

function loadInstanceMap(mapVar)
    assertType(mapVar, 'integer', 'string')
    return assertType(_RSVD_NAME_callFuncCoop('loadMap', mapVar, false), 'integer', 'nil')
end

-- must provide exit base map location
-- players in instance map requires safely exit to a exit base map
function closeInstanceMap(mapUID, exitMapVar, exitX, exitY)
    assertType(mapUID, 'integer')
    assertType(exitMapVar, 'integer', 'string')
    assertType(exitX, 'integer')
    assertType(exitY, 'integer')

    assert(isInstanceMap(mapUID))
    assert(validMapGLoc(exitMapVar, exitX, exitY))
    return assertType(_RSVD_NAME_callFuncCoop('closeInstanceMap', mapUID, exitMapVar, exitX, exitY), 'boolean')
end

function waitActivated()
    _RSVD_NAME_callFuncCoop('waitActivated')
end
