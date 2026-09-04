function dbExec(cmd, ...)
    dbExecString(cmd:format(...))
end

function dbQuery(query, ...)
    return dbQueryString(query:format(...))
end

-- in-game day and night
--
-- one full game day is two real hours: an hour of daylight then an hour of night. the server
-- comes up at the very start of a daytime, so uptime() alone says where in the cycle we are
--
-- uptime() is monotonic, so this does not jump if the system clock is adjusted. launchTime()
-- is the wall clock the cycle started from, for turning a game time back into a real one
SYS_DAYSECONDS   = 60 * 60
SYS_NIGHTSECONDS = 60 * 60

function getDaySeconds()
    return SYS_DAYSECONDS + SYS_NIGHTSECONDS
end

-- how far into the current game day we are, in seconds
function getDayTime()
    return uptime() % getDaySeconds()
end

-- which game day we are on, counting the one the server started in as 1
function getDayCount()
    return uptime() // getDaySeconds() + 1
end

function isDayTime()
    return getDayTime() < SYS_DAYSECONDS
end

function isNightTime()
    return not isDayTime()
end

-- seconds until it turns, whichever way it is about to turn
function getTimeToDayChange()
    if isDayTime() then
        return SYS_DAYSECONDS - getDayTime()
    else
        return getDaySeconds() - getDayTime()
    end
end
