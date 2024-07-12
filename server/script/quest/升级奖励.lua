addQuestTrigger(SYS_ON_LEVELUP, function(playerUID, oldLevel, newLevel)
    uidRemoteCall(playerUID, oldLevel, newLevel,
    [[
        local oldLevel, newLevel = ...
        postString([=[恭喜你从%d升到了%d级！]=], oldLevel, newLevel)
    ]])
end)
