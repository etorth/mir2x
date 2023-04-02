function main()
    addQuestTrigger(SYS_ON_LEVELUP, function(playerUID, oldLevel, newLevel)
        uidExecute(playerUID,
        [[
            postString([=[恭喜你从%d升到了%d级！]=])
        ]], oldLevel, newLevel)
    end)
end
