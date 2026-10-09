# Quest scripts

Each quest is a script of its own, converted from its legacy source. Keep the
source's flow, branches and texts in the script; do not fold quests into a
shared template. The framework lives in `server/src/quest.lua`, the remote API
in `server/script/api/quest.lua`, monster drops in `include/mondrop.lua`.

## States and state runners

`setQuestFSMTable{...}` declares the main fsm, `setQuestFSMTable(name, {...})`
a sub fsm. `setQuestState{uid=uid, state='x'}` writes the state and runs its
state function on a new thread, the state runner of `{uid, fsm}`:

- a switch closes the old state runner at once, even inside `pause()`,
  `uidRemoteCall()` or `waitNotify()`; a reply it waited for is dropped
- a state runner switching its own state never returns, nothing after the
  call runs, neither in the state function nor in its callers
- a switch by any other thread returns true, and false when `from=` refused it
- quest done, `state=SYS_DONE` of the main fsm, closes the state runners of all
  fsms of the player and drops their states, its row keeps the done state only
- a `[SYS_DONE]` function in the fsm table runs once after that, never replayed:
  the place to `setQuestDesp()` the closing description, which the login shows

## A login replays the current states

Every login runs the current state function of each fsm again, from the start.
So a state function gives no reward and takes nothing it can't take twice: do
that in the callback that switches into the state, after the switch, see below.

Before the replay, the world items the state installed but didn't commit are
removed again. `SYS_ON_ONLINE` fires after all states are replayed; trials
register their abandon function for `SYS_ON_ONLINE`, `SYS_ON_OFFLINE` and
`SYS_ON_DIE`, a trial doesn't survive a logout or a restart.

## World items are saved with the state

These install world changes as items of the quest context, saved with the
switch that ends the state installing them, and installed again at the first
login after a restart:

```lua
setupNPCQuestBehavior(mapName, npcName, uid, argstr, code)
clearNPCQuestBehavior(mapName, npcName, uid)

setupMapUIDGridTrigger{uid=uid, name='door', map=mapName, x=x, y=y, argstr=argstr, code=code}
clearMapUIDGridTrigger{uid=uid, name='door'}
```

- `argstr` runs again at every install, it must not capture anything of the
  script; pass values in by `string.format()` and `asInitString()`.
- A grid trigger has a name; installing the name again replaces the trigger,
  on another map it moves there. Its handler returning exactly true retires it.
- Quest done removes every saved item; an install into a quest done raises.
- `setupInstanceNPCBehavior()` and `setupInstanceUIDGridTrigger{}` work on a
  map copy and are never saved, a copy doesn't survive a restart.
- `setupMapGridTrigger()` installs a trigger for everyone, from the script
  load, not saved.

## Callbacks switch with `from=`

An NPC dialog stays installed until something replaces it, often for several
states; a grid trigger, a trigger on the player, a timer and a monster drop
stay until they fire. Every switch made outside the state runner of its fsm
names the state that installed it, or the states it is meant to stay in:

```lua
server.quest.setState(questUID, {uid=uid, from='quest_find_scholar', state='quest_find_guard'})
setQuestState{uid=playerUID, from={'quest_give_soju', 'quest_wait_guards'}, state='quest_wait_guards'}
```

A quest start uses `from = SYS_LUANIL`, no state yet, so a second accept
changes nothing. Every fsm of a quest done counts as `SYS_DONE` here. A `from`
naming a state the fsm doesn't have raises, a typo would refuse the switch for
good. A check made before something that yields, i.e. a remote call, goes into
`from=` too, the state can move on meanwhile.

## Rewards come after their switch

The switch is a compare-and-set in the quest actor. Make it first and give
only if it was made: a second click while the first one's remote calls are on
their way, or a dialog left from an earlier state, gets nothing.

```lua
npc_take_book = function(uid, value)
    if not server.quest.setState(questUID, {uid=uid, from='quest_trial_passed', state=SYS_DONE}) then
        return
    end

    server.player.addItem(uid, '...', 1)
end,
```

A payment, gold or an item the quest takes, goes before the switch and back if
the switch was refused: taken after it, it could be spent by then, and the
switch made for nothing.

```lua
npc_buy = function(uid, value)
    if not server.player.removeGold(uid, 3000) then
        return
    end

    if not server.quest.setState(questUID, {uid=uid, from='quest_buy', state='quest_bought'}) then
        server.player.addItem(uid, SYS_GOLDNAME, 3000)
        return
    end

    server.player.addItem(uid, '童子像', 1)
end,
```

From code sent to the quest, return the result: `return setQuestState{...}`.
A switch that runs a state function reading something the callback writes,
i.e. a quest flag, writes it first. A reward with no switch gets a quest flag,
checked and set in one remote call to the quest, nothing yields between:

```lua
if uidRemoteCall(questUID, uid,
[=[
    local playerUID = ...
    if hasQuestFlag(playerUID, 'gift') then
        return false
    end

    addQuestFlag(playerUID, 'gift')
    return true
]=]) then
    server.player.addItem(uid, '...', 1)
end
```

## Monster drops

`mondrop.addDropTrigger(uid, dropList)`, called from inside the state the drop
belongs to, is live in that state only: a kill after the quest left it removes
the drop and gives nothing, a drop with `setState` switches first, from that
state, then takes and gives. A logout removes the drops of the player, the
replay of the state at the next login adds them again.

## Values that don't survive a restart

Map copy uids and thread keys start over after a restart. Keep them with
`setQuestRuntimeVar(uid, key, value)`, never saved and dropped at quest done,
not in `dbSetQuestVar()`.

## Timers

`runQuestThread(func)` runs func on a thread of its own and returns its key;
`pause()` in it delays the rest, `closeThread(key)` cancels it. A timer that
switches the quest uses `from=` like any callback. Cancel the timers of a state
when it ends, i.e. with a `guardScope()` in the state function, or from the
state that follows.

`closeThread()` of a state runner raises: a state ends with `setQuestState()`.

## Errors

A raising state function rolls back the items it installed and didn't commit,
the error is logged, the quest stays in the state and the next login replays
it. To recover earlier:

- `stateWithFallback(func, fallback)` in the fsm table, the fallback runs for
  every run of the state, replays included, recommended
- `setQuestState{..., fallback=function(uid, args, err) ... end}`, for the run
  this switch starts only
- `setQuestState{..., fallback=[[ local uid, args, err = ... ]]}`, saved with
  the state, run by the replay too, and `server.quest.setState()` can send it

A fallback entering the same state again calls `pause()` first.

## Refused

Each of these raises before it changes anything:

- a switch of a player while another switch of that player runs, i.e. in a
  `<close>` handler of a state runner being closed
- a state runner switching itself from a coroutine it created, or where it
  can't yield, i.e. in a `table.sort()` comparator
- an unknown state, a `from` state the fsm doesn't have, a fallback for a
  state without state function, a function fallback by remote call
- states switching in a cycle without a yield, 64 deep
- `closeThread()` of a state runner

A `<close>` handler runs as plain C code in a close: no `pause()`, no remote
call, no switch there; `guardScope()` is for cleanup that doesn't yield.

## Tests

The framework is covered by the unit tests in `server/test/unit`
(`queststate`, `playerquest`, `gridtrigger`, `questdb`, `luarunner`), run by
`build.py --run-test`. No test loads the quest scripts: `luac -p` checks a
script, not the code strings in it, which compile only when they are sent.
