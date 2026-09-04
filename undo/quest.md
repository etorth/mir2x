# HonChonDo / HonChonMado — the two legendary blade questlines

Everything found while surveying `Envir/QuestDiary/HonChonDo` and `Envir/QuestDiary/HonChonMado`
for conversion. **Nothing is converted.** This is not a transcription job like the 23 `MU_*`
skill quests were — it needs 6–8 new engine capabilities first, so it is a feature project.

Written 2026-09-04, against `Envir.utf8` as it stands and the `quest` branch.

---

## 1. What it is

Two halves of one story about a legendary sword and its shadow:

| half | blade | stages | files | bytes |
|---|---|---|---|---|
| `HonChonDo` | **寂幻之刃** (the shadow) | 25 | 15 | ~190 KB |
| `HonChonMado` | **影魅之刃** (the real blade) | 11 | 16 | ~79 KB |

31 files, ~270 KB total — comparable to everything converted in the skill quest pass.

`HonChonDo` is the prerequisite: you win 寂幻之刃 from it, and stage 1-22 says the mad warrior
lends it **for one week** as a test of whether you are fit to carry 影魅之刃. `HonChonMado` is
then the hunt for the real blade, ending against the fallen spirit of **诺玛教主**.

### HonChonDo stage list (`Convert_Def/QuestDiary/HonChonDo/HelperHonChonDoBook.txt`)

The in-game quest log; 25 sections `@Convert_HelperHonChonDoBook_0..24`. `_0` is "not started".

| stage | title | required items |
|---|---|---|
| 1-1 | 诺玛们的宗教活动 — the 毁灭之印 only surfaces during the Numa ceremony | – |
| 1-2 | 4条道路 — the ceremony is guarded, only four approach routes | – |
| 1-3 | 获取宝箱 — got the chest, take it to 夏柯 | – |
| 1-3 | 开启宝箱 — open it in front of 夏柯 | 宝箱 |
| 1-4 | 请求调查遗址 4楼 — 夏柯 asks you to survey ruins floor 4 | – |
| 1-5 | 用尸骨做成的项链 — the bone necklace is a dead Numa warrior's relic; ask 拉贝卡 | 尸骨项链 |
| 1-6 | 与拉贝卡的见面 — he stayed as the cult leader's man to solve 扎马尔's death; 帕斯卡 knows more | 尸骨项链 |
| 1-7 | 寻找帕斯卡 — the 地下神女 will only talk after a trial | 尸骨项链 |
| 1-8 | 击退护身符 — 梅山侠 sends a warding charm to get you through it | 击退护身符 |
| 1-9 | 与帕斯卡的对话 — she gives you his location | 击退护身符, 尸骨项链 |
| 1-10 | 与帕斯卡的对话 — 阿龙怪 interrupts | 击退护身符, 尸骨项链 |
| 1-11 | 勇者的儿子 — 阿龙怪 may be the warrior's son | 击退护身符, 尸骨项链 |
| 1-12 | 击退护身符 — the charm drives him off but you lose the necklace; report to 夏柯 | – |
| 1-13 | 夏柯的新请求 — go back to 帕斯卡 for more | – |
| 1-14 | 地下神女的安排 — she arranges the second meeting | – |
| 1-15 | 重新出现 — 阿龙怪 returns, 帕斯卡 fails to talk him down | – |
| 1-16 | 与帕斯卡的对话 — resume once he leaves | – |
| 1-17 | 听到秘密 — you hear the secret | – |
| 1-18 | 报告夏柯 | – |
| 1-19 | 诺玛圣地 — infiltrate the Numa holy ground for the 毁灭之印 | – |
| 1-20 | 永远的平行线 — you and 阿龙怪 will never cooperate | – |
| 1-21 | 发疯的诺玛勇士之魂 — speak with the warrior bound to 寂幻之刃 | 寂幻之刃 |
| 1-22 | 魔剑的影子 — he lends 寂幻之刃 **for one week** as the test | – |
| 1-23 | 寂幻之刃 — four eyes missing, only a shadow of 影魅之刃, still a powerful weapon | – |

### HonChonMado stage list (`Convert_Def/QuestDiary/HonChonMado/HelperHonChonMaDoBook.txt`)

11 sections `@Convert_HelperHonChonMaDoBook_0..10`.

| stage | title |
|---|---|
| 1-1 | 影魅之刃的诱惑 — collect the necklaces that become the sword's eyes |
| 1-2 | 艰难的出发 — 万里碧海 is lost and the journey stalls at the start |
| 1-3 | 魔达的帮助 — 魔达 explains it: break the mechanisms, get the tokens |
| 1-4 | 其他的动向 — unsigned letters hint at rivals |
| 1-5 | 神秘之印 — 紫霞神女 gives the seal, which opens the third guardian's gate |
| 1-6 | 守护石和守护者 — old secrets and bad omens as the blade nears completion |
| 1-7 | 怪人的人情 — got 大脚将的角笛 from the strange man; on to the 祭祀长 domain |
| 1-8 | 魔达的凶计 — 魔达 finally shows his true face at the last gate |
| 1-9 | 疯了的诺玛教主的灵魂 — beat the cult leader's fallen spirit; the trial ends |

### Cast

`夏柯` (Sahaka), `巴斯卡/帕斯卡` (Baska), `扎马尔/자하` (Jaha), `拉贝卡`, `玄` (Hyun),
`魔达` (Mado, the traitor), `紫霞神女`, `地下神女`, `梅山侠`, `诺玛大法老1..4`, `阿龙怪1..4`,
`诺玛教主` (final boss), `诺玛司令`.

---

## 2. It is a scheduled world event, not a player-driven quest

This is the single biggest structural difference from the skill quests, and the reason it cannot
be converted the same way.

`HonChonDo/HonMainControl.txt` reschedules itself forever and branches on the **real-world
calendar**:

```
[@MainHonChonRootin]
DelayGoto [Grobal] 180 @Honchonmadocontrol 1005      -- re-fire every 180s, forever

[@DAYOFWEEKCHECK]
DAYOFWEEK TUE  -> @tuesdayHonQuest
DAYOFWEEK WED  -> @wednesdayHonQuest
DAYOFWEEK THU  -> @thursdayHonQuest
DAYOFWEEK FRI  -> @fridayHonQuest
DAYOFWEEK SAT  -> @saturdayHonQuest
```

38 `DAYOFWEEK` checks and 144 `HOUR`/`MIN` windows across the two directories. Concrete schedule
from `HonMainControl.txt`:

- **Tue 23:01–23:05** — `@tuesdayMonGen`, the Numa ceremony spawns
- **Wed** — `@tuesdayMonGen` again at 01/03/05/07/09/11/13/15/17/19 :01–:05, then
  `@wednesdayMonGen` at 22:01 and 23:01
- **Thu** — `@wednesdayMonGen` at 03/08/14/19 :01–:05; `@wednesdayMonClear` at 20:55–20:59;
  `@thursdayMonGen` 22:00–23:50; `@thursdayMonClear` 23:50–23:59
- **Fri 00:01–00:05** — pharaohs move **out** of the pen into the ruins
- **Fri 20:55–20:59** — pharaohs move **back** to the pen
- **Fri 21:01–21:05** — 阿龙怪 ×4 move from the ruins to the pen
- **Fri 23:01–23:05** — pharaohs move to `D1604`
- **Sat 22:55–22:59** — pharaohs return to the pen

> The in-game day/night clock added in `b82f2bee` (`isDayTime`/`isNightTime`, 1 h day + 1 h night
> off `uptime()`) does **not** help here. That is a 2-hour game cycle; this needs real weekdays
> and real hours. Two different clocks — do not conflate them.

### NPC relocation

39 `Movenpc` calls shuttle NPCs between a holding pen and the dungeon:

```
Movenpc "npcroom1,10,7,诺玛大法老1"  D16031 61 354
Movenpc "npcroom1,10,8,诺玛大法老2"  D16032 374 371
Movenpc "npcroom1,10,9,诺玛大法老3"  D16033 369 46
Movenpc "npcroom1,10,10,诺玛大法老4" D16034 30 30
Movenpc "D16031,61,355,阿龙怪1"      npcroom1 9 7
```

**mir2x already ships the 8 placeholders at exactly these coordinates** — someone carried the
holding-pen design across deliberately and never wired it:

```
NPC屋_NPCROOM1.诺玛大法老_1.GLOC_10_7_0.LOOK_86.lua   <->  "npcroom1,10,7,诺玛大法老1"
NPC屋_NPCROOM1.诺玛大法老_2.GLOC_10_8_0.LOOK_86.lua   <->  "npcroom1,10,8,诺玛大法老2"
NPC屋_NPCROOM1.诺玛大法老_3.GLOC_10_9_0.LOOK_86.lua   <->  "npcroom1,10,9,诺玛大法老3"
NPC屋_NPCROOM1.诺玛大法老_4.GLOC_10_10_0.LOOK_86.lua  <->  "npcroom1,10,10,诺玛大法老4"
NPC屋_NPCROOM1.阿龙怪_1.GLOC_9_7_0.LOOK_90.lua        <->  "npcroom1,9,7,阿龙怪1"
NPC屋_NPCROOM1.阿龙怪_2.GLOC_9_8_0.LOOK_90.lua        <->  "npcroom1,9,8,阿龙怪2"
NPC屋_NPCROOM1.阿龙怪_3.GLOC_9_9_0.LOOK_90.lua        <->  "npcroom1,9,9,阿龙怪3"
NPC屋_NPCROOM1.阿龙怪_4.GLOC_9_10_0.LOOK_90.lua       <->  "npcroom1,9,10,阿龙怪4"
```

All 8 are **empty files** (they are among the 191 empty NPC placeholders in the repo — the same
class of file that let the U+F00D filename bug hide in `24a16db5`, so watch for that).

`ServerMap::loadNPChar` spawns NPCs from filenames at map load. There is no move-NPC-across-maps
operation, so either that gets built, or the pen design is dropped and each NPC gets a script that
answers only inside its schedule window.

---

## 3. World state and the unique blade

`TBL_HonChonMadoSeverFlag` (legacy's own typo — "Sever", not "Server") is keyed on a **single
named row**, not on a player:

```
FormatStr "FLD_MADOQUEST='%s'" "HonChon"
ReadValueSql "TBL_HonChonMadoSeverFlag" %A9 "FLD_MADOHAVE" [@callback]
```

`FLD_MADOHAVE=1` means **somebody in the world already holds 寂幻之刃**. From
`HonChondonpc.txt`, handing the blade over does:

```
TAKE 毁灭之印 1
Givew 1 寂幻之刃                                       -- straight into wear slot 1
UpdateValueSql "TBL_HonChonMadoQuest"     %A9 "FLD_MADOMAIN=7"
UpdateValueSql "TBL_HonChonMadoQuest"     %A9 "FLD_MADOCONTENT=1"
UpdateValueSql "TBL_HonChonMadoQuest"     %A9 "FLD_MADOQUESTCHECK=1"
UpdateValueSql "TBL_HonChonMadoSeverFlag" %A9 "FLD_MADOHAVE=1"
set [205] 0
Eventmsg grobal %A7 2000                               -- announce it to the whole server
```

So the blade is **world-unique with a one-week loan** (stage 1-22). Anyone arriving while
`FLD_MADOHAVE=1` gets the `@HonChondo_Questnpc_1_have` branch instead.

This is what the per-player `fld_*` quest model cannot express. It needs shared quest state plus
a real-time expiry.

---

## 4. The four SQL tables

**No schema ships with the data** — `grep -i "create table"` across all of `Envir` finds nothing.
The legacy server or an admin created these out of band, so column types are only inferable from
call sites.

`TBL_*` with `ReadValueSql` / `UpdateValueSql` is a **general legacy facility**, not something
these questlines invented — `QuestDiary/CastleWar/` uses `TBL_SnakeVallyWar` the same way. So the
mir2x gap is "no script-level SQL at all", and one binding pair plus an async-callback convention
serves every legacy questline that uses it.

### `TBL_HonChonMadoQuest` — per-player, HonChonDo half

Key `FLD_MADOUSER='<username>'`. Three-level progress: `FLD_MADOMAIN` (chapter) /
`FLD_MADOCONTENT` (stage) / `FLD_MADOQUESTCHECK` (substage).

| column | notes |
|---|---|
| `FLD_MADOUSER` | username, the key |
| `FLD_MADOMAIN` | chapter, seen set to 7 and 8 |
| `FLD_MADOCONTENT` | stage |
| `FLD_MADOQUESTCHECK` | substage |
| `FLD_MADOPATTERN` | in the insert column list — **check whether anything reads it** |
| `FLD_MADOTEMPQUEST` | in the insert column list — **check whether anything reads it** |

Row created once, `HonChonDo/MadoNpcSahaka.txt:37`. 323 references across 11
`QuestDiary/HonChonDo/*.txt`, heaviest in `Mon_extinction.txt` (61), `MadoNpcSahaka.txt` (57),
`MadoNpcBaska.txt` (51), `MadoNpcJaha.txt` (47). Also 5 references outside the directory:
`QuestDiary/NpcQuest_Def/17baska1-npcroom1.txt` … `17baska4-npcroom1.txt` and
`17Zaha_Numa-41.txt` — **do not miss these, they are the pen-side NPC scripts.**

### `TBL_MadoHaveUserQuest` — per-player, HonChonMado half

Key `FLD_MADOHAVEUSER='<username>'`. Columns `FLD_MADOUSERCONTENT` (stage 1..13+),
`FLD_MADOUSERCHECK` (substage), `FLD_MADOUSERPATTERN`.

Rows created at `HonChonMado/MadoNpc.txt:37` and `SahakaNpc.txt:28`. 165 references across all 16
`QuestDiary/HonChonMado/*.txt`, heaviest in `Mon_MadoHave.txt` (46), `MonsterNpc1.txt` (27),
`MadoNpc.txt` (21).

The Helper book branches on `FLD_MADOUSERCONTENT` with `Equal D0 n` / `Large D0 n` / `Small D0 n`,
reaching `Large D0 13` — so the stage counter runs past 13 even though the log only has 11 entries.

### `TBL_HonChonMadoSeverFlag` — server-wide singleton

Key `FLD_MADOQUEST='HonChon'`. Columns `FLD_MADOHAVE`, `FLD_MADOMAPEVENT`, `FLD_MADOQUESTSET`.
29 references: `MadoNpcJaha.txt` (10), `extinctionQuestEvent.txt` (9), `MadoNpcBaska.txt` (8),
`HonChondonpc.txt` (2).

### `TBL_HonChonMadoServer` — one reference

`HonChonDo/MadoextinctionNpc.txt`, single column `FLD_PASSWORDNAME`. Purpose unclear; possibly a
GM/admin gate. Investigate before building anything for it.

### Access pattern

Always string-built SQL, and reads are **asynchronous with a callback label**:

```
FormatStr "FLD_MADOHAVEUSER='%s'" %USERNAME
ReadValueSql "TBL_MadoHaveUserQuest" %A9 "FLD_MADOUSERCONTENT,FLD_MADOUSERCHECK,FLD_MADOUSERPATTERN" [@MadoNpc_Call_0]

[@MadoNpc_Call_0()]
mov D1 %ARG(1)
mov D2 %ARG(2)
mov D3 %ARG(3)
```

mir2x's coroutine lua (`bindCoop` + `LuaCoopResumer`) suits this well — a blocking-looking
`dbQuestRead(...)` that yields would read far better than the callback labels.

Verb totals across both directories: **119 `ReadValueSql`, 3 `WriteValueSql` (row insert),
401 `UpdateValueSql`**.

---

## 5. Complete legacy verb inventory

Every line-initial verb in the two directories, with what mir2x has. Counted with
`cat HonChonDo/*.txt HonChonMado/*.txt | grep -oE "^[A-Za-z_]+" | sort | uniq -c`.

| verb | n | mir2x | note |
|---|---|---|---|
| `Equal` / `Small` / `Large` | 812/16/14 | ✅ | plain lua comparison |
| `break` / `goto` | 742/581 | ✅ | control flow, becomes lua structure |
| `UpdateValueSql` | 401 | ❌ | **no script SQL** |
| `FormatStr` | 308 | ✅ | `string.format` |
| `LoadValue` | 288 | ✅ | string externalisation — inline the prose instead |
| `Eventmsg` | 270 | ❌ | broadcast; `Eventmsg grobal <msg> <ms>` is server-wide |
| `Delaygoto` / `DelayGoto` | 231/10 | ✅ | `runQuestThread` + `pause`, cancellable |
| `mov` / `Mov` / `movr` / `inc` / `AddStr` | 277/7/27/28/3 | ✅ | lua locals; `movr` is `math.random` |
| `Checkitemw` / `Checkitem` / `checkitem` | 155/49/88 | ✅ | `getWLItem`, `hasItem` |
| `HOUR` / `min` / `MIN` / `Min` | 144/23/6/2 | ❌ | **real wall-clock hour/minute** |
| `ReadValueSql` | 118 | ❌ | **no script SQL** |
| `set` | 66 | ✅ | quest flags |
| `take` / `Take` / `takew` | 47/2/1 | ✅ | `removeItem`, `removeWearItem` |
| `Mongenp` | 46 | ⚠️ | `addMonster` exists but **no per-spawn drop-table override** |
| `Movenpc` | 39 | ❌ | **move an NPC to another map** |
| `ApplyMonEx` | 38 | ❌ | **monster faction/title tag** (`[Monsterside]`) |
| `DAYOFWEEK` | 35 | ❌ | **real weekday** |
| `monclear` / `MonClear` | 35/22 | ✅ | `clearMonster()`, added `b82f2bee` |
| `give` / `Give` / `givew` | 35/5/1 | ⚠️ | `addItem` yes; `Givew <slot> <item>` (straight into a wear slot) has no binding — `Player::setWLItem` exists in C++ at `player.cpp:2146` but is unbound |
| `linemsg` / `Linemsg` | 11/6 | ❌ | broadcast to one map |
| `Checkmonmap` / `checkmonmap` | 11/6 | ✅ | `getMonsterCount()` |
| `Enter_fail` | 10 | ✅ | `setupMapDefaultGridTrigger` / `setupMapGridTrigger`, return `false` |
| `close` | 9 | ✅ | `close="1"` on an event |
| `mapting` | 8 | ❓ | `mapting D1606 41 148 84` — **4 args, purpose unknown**, investigate |
| `mapmove` / `MAPMOVE` / `map` | 5/3/5 | ✅ | `spaceMove`, takes a name, id or uid since `b82f2bee` |
| `return` | 4 | ✅ | |
| `GiveExp` | 4 | ⚠️ | `Player::gainExp` exists at `player.cpp:1822`, **unbound** |
| `gender` | 3 | ✅ | `getGender()` (was a broken `getGener`, fixed) |
| `WriteValueSql` | 3 | ❌ | **no script SQL** |
| `GroupMove` | 3 | ❌ | move the whole party: `GroupMove "D16062,37,48" %USERNAME` |
| `mapspell` | 2 | ⚠️ | `mapspell [firewall] "D16061,22,33,40,47, +d0, +t10, +r50, +h"` — `ServerMap::MapGrid::fireWallList` exists (`servermap.hpp:87`) with no lua binding |
| `checkjob` / `checkgold` / `Checkhum` / `Check` | 2/2/2/2 | ✅ | |
| `Checkweaponlevel` | – | ❌ | `!Checkweaponlevel 1` in `HonChondonpc.txt` |

### Summary of what must be built

1. **Real-time scheduler** — weekday + hour + minute windows, and a server-wide repeating tick
   (legacy: `DelayGoto [Grobal] 180`). Separate from the in-game day/night clock.
2. **Script-level SQL** — read/insert/update with a coroutine-style read. Benefits CastleWar too.
3. **Shared (non-player) quest state** — the `Sever`Flag singleton, plus real-time expiry for the
   one-week blade loan.
4. **`Movenpc`** — relocate an NPC between maps at runtime.
5. **Broadcast** — `Eventmsg grobal` (server-wide, with a duration) and `linemsg` (per-map).
6. **`Mongenp`** — spawn with a per-spawn drop-table override.
7. **`ApplyMonEx [Monsterside]`** — monster faction/title so event monsters read as a side.
8. **`GroupMove`** — party-wide teleport.
9. Small bindings for things already in C++: `gainExp`, `setWLItem` (`Givew`), a firewall
   binding for `mapspell`, and `Checkweaponlevel`.
10. Work out what `mapting` does.

---

## 6. Maps

| legacy | mir2x | id |
|---|---|---|
| `D1604` | 诺玛遗址4层_D1604 | 293 |
| `D16031` | 诺玛遗址3层_D16031 | 289 |
| `D16032` | 诺玛遗址3层_D16032 | 290 |
| `D16033` | 诺玛遗址3层_D16033 | 291 |
| `D16034` | 诺玛遗址3层_D16034 | 292 |
| `D1606` | 诺玛勇士墓穴_D1606 | 295 |
| `D16061` | **MISSING** | – |
| `D16062` | **MISSING** | – |
| `D16063` | 怪人屋_D16063 | 296 |
| `D16064` | 祭祀长领地_D16064 | 297 |
| `npcroom1` | NPC屋_NPCROOM1 | – |

`D16061` and `D16062` have no `maprecord.inc` entry. `D16061` is used 43 times (it is where the
`mapspell [firewall]` traps go and a `GroupMove` target); `D16062` 3 times. Check whether map data
exists for them before assuming they can be added.

Also referenced: `D16062 41 148 84` via `mapting`, and `mapting D1606 4 174 139`.

## 7. Items

21 named by `give` / `take` / `checkitem`. **15 are missing from `itemrecord.inc`.**

| item | in DB |
|---|---|
| 影魅之刃 | ✅ |
| 寂幻之刃 | ✅ |
| 万里碧海 | ✅ |
| 九宫云雾 | ✅ |
| 血花落照 | ✅ |
| 黑天暗云 | ✅ |
| 击退护身符 | ❌ |
| 大地石 | ❌ |
| 大族长角笛 | ❌ |
| 太阳石 | ❌ |
| 安息石 | ❌ |
| 尸骨项链 | ❌ |
| 心石 | ❌ |
| 无名书 | ❌ |
| 月光石 | ❌ |
| 机关零件 | ❌ |
| 毁灭之印 | ❌ |
| 活石 | ❌ |
| 神秘之印 | ❌ |
| 藏宝箱 | ❌ |
| 青空石 | ❌ |

The blades themselves exist; every quest token and the five element stones
(大地石/太阳石/心石/活石/青空石) do not. Note the log also mentions `宝箱` (exists) alongside
`藏宝箱` (missing) — check whether legacy means one item or two.

**Appending to `itemrecord.inc` is ID-safe** (IDs are positional, `s_itemRecordAssertor` enforces
name uniqueness at startup). Deleting or inserting is not.

## 8. Monsters

| monster | record | NPC placeholder |
|---|---|---|
| 诺玛司令 | ✅ | – |
| 诺玛教主 | ✅ | – |
| 诺玛大法老 | ✅ | 4 (empty) |
| 阿龙怪 | ❌ | 4 (empty) |

`阿龙怪` is both an NPC (in the pen) **and** a monster (`Mongenp "D16031,33,44,10" 阿龙怪 1`), so
it needs a monster record as well as its NPC scripts.

Spawn shape, from `extinctionMongen.txt`:

```
Mongenp   "D16031,33,44,10" 诺玛司令 20 "noitem,金创药（特）,魔法药（特）"
ApplyMonEx "D16031,33,44,30" 诺玛司令 [Monsterside] "毁灭之印大师"
Mongenp   "D16031,33,44,10" 阿龙怪  1  "noitem,金币,5000,金创药（特）,魔法药（特）"
ApplyMonEx "D16031,33,44,30" 阿龙怪  [Monsterside] "???"
```

`"noitem,..."` overrides the drop table for that spawn only. `ApplyMonEx [Monsterside] "<title>"`
tags them — `毁灭之印大师` is the title on the 诺玛司令 that carry the seal.

## 9. Flags

- `[205]` — HonChonDo active. Set 0 when the blade is handed over.
- `[207]` — HonChonMado active. Gates the Helper book.
- `[208]` — set 0 alongside `[207]` at `MadoNpc.txt`.

Progress itself lives in SQL, not flags — the flags are only "is this questline open".

## 10. File map

### `QuestDiary/HonChonDo` (15 files)

| file | bytes | what |
|---|---|---|
| `extinctionQuestEvent.txt` | 43536 | biggest; 29 `TBL_HonChonMadoQuest` + 9 `SeverFlag` refs, 38 `UpdateValueSql` |
| `MadoNpcBaska.txt` | 35981 | 帕斯卡/巴斯卡 dialogue, 51 + 8 refs |
| `MadoNpcSahaka.txt` | 20055 | 夏柯; **creates the `TBL_HonChonMadoQuest` row** (line 37) |
| `Mon_extinction.txt` | 15087 | monster-death hooks, 61 refs, 39 updates |
| `MadoNpcJaha.txt` | 13279 | 扎马尔/자하, 47 + 10 refs |
| `MadoNumaNpc.txt` | 11852 | Numa NPCs, 24 refs |
| `MadoextinctionNpc.txt` | 9149 | 25 refs; the only `TBL_HonChonMadoServer` user |
| `extinctionMongen.txt` | 8586 | the scheduled spawns (`@tuesdayMonGen` etc.) |
| `HelperHonChonDoBook.txt` | 7988 | the 25-stage quest log reader |
| `HelperHonChonDoBook.txt.utf8` | 8000 | **duplicate, all on one line**; nothing references it — ignore, it breaks greps |
| `HonMainControl.txt` | 4635 | the weekly scheduler |
| `HonChondonpc.txt` | 4352 | the entry NPC; hands over 寂幻之刃 |
| `MadoNpcHyun.txt` | 3012 | 玄 |
| `Map_extinctionRoom.txt` | 1976 | map script, 10 refs |
| `Map_honchonmadoRoom.txt` | 1901 | map script |

### `QuestDiary/HonChonMado` (16 files)

| file | bytes | what |
|---|---|---|
| `Mon_MadoHave.txt` | 18085 | monster hooks, 46 refs, 40 updates |
| `MadoNpc.txt` | 10987 | 魔达; **creates the `TBL_MadoHaveUserQuest` row** (line 37) |
| `MonsterNpc1.txt` | 8758 | 27 refs |
| `QuestMadoEvent.txt` | 6029 | event glue |
| `StoneNpc3.txt` | 4783 | guardian stone 3 |
| `MadoHaveJaha.txt` | 4678 | |
| `StoneNpc4.txt` | 4503 | guardian stone 4 |
| `SahakaNpc.txt` | 3851 | also creates the row (line 28) |
| `NumaNpcMagda.txt` | 3463 | |
| `NumaNpcMagda1.txt` | 2776 | |
| `StoneNpc2.txt` | 2535 | |
| `StoneNpc1.txt` | 2406 | |
| `StoneNpc5.txt` | 2263 | |
| `HelperHonChonMaDoBook.txt` | 1955 | the 11-stage quest log reader |
| `StoneNpcbox1.txt` | 1726 | |
| `ItemQuestMado.txt` | 957 | |

### Prose lives elsewhere

Every `#SAY` is `#INCLUDE [Convert_Def/QuestDiary/<dir>/<same-filename>] @Convert_<label>_n`.
So each of the above has a **paired file** in `Convert_Def/QuestDiary/HonChonDo/` (14 files —
no pair for the `.utf8` duplicate) and `Convert_Def/QuestDiary/HonChonMado/` (16 files, one each)
holding the actual text. 288 `LoadValue` calls
pull short strings the same way. When converting, inline the prose into the `.lua` the way the
skill quests do — do not reproduce the indirection.

### Outside the two directories

- `QuestDiary/NpcQuest_Def/17baska1-npcroom1.txt` … `17baska4-npcroom1.txt` — pen-side scripts,
  1 `TBL_HonChonMadoQuest` ref each
- `QuestDiary/NpcQuest_Def/17Zaha_Numa-41.txt` — 1 ref

## 11. Legacy data quirks to expect

Consistent with the rest of `Envir` (see the `legacy-data-is-buggy` note): expect unreachable
labels, self-referencing gotos, unsatisfiable checks and drifting NPC names. Already spotted:

- `TBL_HonChonMadoSeverFlag` — misspelled in the data itself. Keep the typo or the SQL will not
  match; do not "fix" it silently.
- `HelperHonChonDoBook.txt.utf8` — a one-line duplicate of the `.txt`, ~identical size. Not read
  by anything; it wrecks any `grep -n` that touches it.
- The HonChonDo log has **two stages both labelled 1-3** (获取宝箱 and 开启宝箱).
- Stage 1-23 is tagged `{FCOLOR/13}` where every other stage uses `{FCOLOR/10}`.
- `HelperHonChonDoBook` reaches `Large D0 13` while the log only defines 11 HonChonMado stages —
  the counter overruns the log.
- Several stage descriptions have literal `(???:???)` placeholders where coordinates should be,
  and one `ApplyMonEx ... "???"` title. Legacy shipped them unfilled.
- `FLD_MADOPATTERN` / `FLD_MADOTEMPQUEST` appear only in the insert column list — confirm anything
  reads them before designing a schema around them.

## 12. Suggested order

1. Script-level SQL (or decide on a different persistence model for multi-level + shared state).
   This unblocks CastleWar as well.
2. Real-time scheduler: weekday/hour/minute + a global repeating tick.
3. `Movenpc`, or decide the pen is dropped and NPCs self-gate on the schedule.
4. Broadcast (`Eventmsg grobal`, `linemsg`).
5. The 15 item records, the 阿龙怪 monster record, and `D16061`/`D16062` if map data exists.
6. `Mongenp` drop override + `ApplyMonEx` faction, then the spawn schedule.
7. Only then start transcribing dialogue — HonChonDo first, since 寂幻之刃 gates HonChonMado.

## 13. How to verify a conversion

The tooling built for the skill quests applies directly, and lives in the session scratchpad
(copy it somewhere permanent before relying on it):

- `questharness.lua <file>` — runs a quest plugin under stubbed globals, `load()`s every embedded
  code string, walks every FSM state and NPC handler, renders every layout and checks XML balance
- `npcharness.lua <file> <include-dir>` — the same for an NPC script. **The include dir is a
  required second argument**; without it `require` silently returns `{}`
- `coverage.lua <legacy-src.txt> <converted.lua>` — greps every legacy dialogue line against the
  converted file and reports what is missing. Format-substituted lines (`%s`) show as false
  misses; `goto`/`MonGen`/`give` command lines are noise

`luac -p` for syntax, but **copy to an ASCII temp name first** — `luac` cannot open CJK paths on
Windows. And after generating any file with a CJK name, scan for mangled filenames:

```
git ls-files -z | python -c "import sys;[print(repr(p)) for p in sys.stdin.buffer.read().decode().split(chr(0)) if any(0xE000<=ord(c)<=0xF8FF or ord(c)<0x20 for c in p)]"
```

That check is what caught 11 skill-teacher files named with a trailing U+F00D that the server
never loaded (`24a16db5`).
