addQuestTrigger(SYS_ON_KILL, function(playerUID, monsterID)
    uidRemoteCall(playerUID, monsterID,
    [[
        local monsterID = ...
        postString([=[君子戒杀！又一只%s成为你的手下亡魂！]=], getMonsterName(monsterID))
    ]])
end)
