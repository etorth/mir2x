local npc = {}
local g_eventHandler = {}

function npc.setEventHandler(eventHandler)
    g_eventHandler = eventHandler
end

function npc.checkEventHandler(verbose, event)
    if verbose ~= nil then
        assertType(verbose, 'boolean')
    else
        verbose = false
    end

    if event ~= nil then
        assertType(evetn, 'string')
    end
end

return npc
