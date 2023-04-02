function main()
    addQuestTrigger(SYS_ON_KILL, function(playerUID, monsterID)
        uidExecute(playerUID,
        [[
            postString([=[君子戒杀！又一只%s成为你的刀下亡魂！]=])
        ]], getMonsterName(monsterID))
    end)
end
