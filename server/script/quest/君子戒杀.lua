addQuestTrigger(SYS_ON_KILL, function(playerUID, monsterUID)
    uidRemoteCall(playerUID, monsterUID,
    [[
        local monsterUID = ...
        postString([=[君子戒杀！又一只%s成为你的手下亡魂！]=], getMonsterName(getMonsterID(monsterUID)))
    ]])
end)
