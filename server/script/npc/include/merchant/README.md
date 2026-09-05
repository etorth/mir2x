# Merchant scripts

Each module owns its NPC menu, event handlers and services. There is no generic
merchant builder. Do not turn these modules back into wrappers around one, or
move service selection and event registration into a shared helper.

| Module | Responsibilities |
| --- | --- |
| `smith` | Weapon/mineral trading, ordinary and special repair, preliminary repair pages, weapon advice, bound-sword removal |
| `outfitter` | Clothing, helmet and shoe trading and ordinary repair, including preliminary repair pages |
| `jeweler` | Accessory trading, ordinary repair and the Numa rusty-accessory restoration service |
| `apothecary` | Potion/scroll trading and the two scripted special-potion sales, never equipment repair |
| `bookseller` | Skill-book trading and the bookseller's own class/book explanation pages |
| `grocer` | General-store trading; ordinary mending only when explicitly offered |
| `butcher` | Meat buying, optional stock and meat-gathering instructions |
| `buyer` | Material buying, without a stock or repair counter |
| `repairer` | Repair services, without trading |

`npc.include.invop` implements low-level inventory transactions.
`npc.include.dialog` only formats dialogue/links and guards individual
callbacks against red-name access. Neither chooses a merchant's services,
creates its menu or registers its handlers.

`dialog.link(id, label, opts)` accepts optional `prefix`, `suffix` and `close`
fields. If `close` is omitted, only `SYS_EXIT` closes the dialog; `true` or
`false` explicitly overrides that default.

## Converting a merchant

Read both `Envir/Market_Def/<name>.txt` (control flow, item categories and stock)
and its `Envir/Convert_Def/Market_Def/<name>.txt` includes (dialogue). Follow the
reachable branches, not just the list of section names.

Keep the provenance comment, stock, greetings, prompts, completion lines,
navigation labels and special conversations in the NPC script. Resolve
`#INCLUDE` stock lists and use the exact item names/types from `itemrecord.inc`.
Do not replace distinct source paragraphs with a summary or an invented answer.
The apothecary and material buyer also check `TRADE_ITEMS` against the legacy
`Stdmode` values in `King_StdItems.csv`, because the inventory type `道具` alone
would accept unrelated items. Keep those lists in sync when updating item data;
an NPC with a different source category can supply its own `tradeItems`.

Choose the matching type, then retain that NPC's exceptions explicitly. In
particular, ordinary repair and special repair are separate services; `false`
disables a type's default service, and a missing `goods` list means no stock
counter. Potion shops and material buyers do not acquire repair services from
unrelated fields.

Menus can override their type's operation labels/suffixes and exit/back labels.
Only NPCs with a reachable daily-task menu set `today`; it is the original
no-task fallback, not an implementation of the legacy daily-quest scheduler.
Use `preRepairText` separately from `repairText` where the source has both.
`repairDoneBack` preserves the completion-page caption; `false` omits that link.
Use a greeting function when text depends on live state instead of evaluating
that state when the script loads. Book help can retain NPC-specific prose in
`bookHelp`, rather than forcing every bookseller to say the same thing.

If a merchant cannot be expressed faithfully by its type, add a type-specific
branch or write its script directly. Do not weaken all other merchant types to
accommodate the exception. Quest state machines remain in the quest scripts;
an empty placeholder conversation is not a replacement for one.

For scripted exchanges, perform profession selection, material removal,
payment and rewards together in one player-side call. Return only serializable
results, not native containers with metatables. Closing a conversation
can cancel the NPC coroutine while an already-sent player operation still runs;
separate removal and reward calls can therefore lose the player's materials.

## Engine and source-data boundaries

The existing inventory operations still use mir2x pricing/durability rules,
not the legacy percentage price multipliers or Volume/Hour replenishment.
Do not infer buyback categories from the shop's stock.

The server does not expose castle-war state and `getSubukGuildName()` is still
a server-side placeholder. Sabuk merchants therefore use their original
peacetime dialogue, evaluated on entry, rather than claiming a perpetual siege.

The Pan Ye smith's item-gated conversation, profession-specific rewards and
exchange checks are retained locally. The current item database lacks
`潘夜天灵`, so that branch cannot be activated yet: it logs the missing record
and refuses exchange attempts without removing or granting anything. Do not
substitute another quest item or invent its missing item data.

The Oasis cursed-potion sale charges 5,000,000 gold; the RedZone rebirth-potion
sale charges 1,500,000. These are explicit script prices, independent of the
normal shop's price multiplier. Numa restoration charges 1,000,000 and preserves
the original nine-item priority and seven element choices. Checks, debits and
grants run together in a player-side call.

The 4,000,000-gold element-change option is commented out in the legacy script,
so it is not offered. The dark-element Shizun bracelet restoration branch uses
garbled inline source text with no close link instead of the clean, unused
dialogue include; that discrepancy is deliberately retained.

## Regression coverage

From the repository root, with Lua 5.4:

```sh
lua server/test/merchant.lua server/script/npc/*.lua
```

The harness uses the NPC dispatcher and the real inventory helper with a mocked
game world. It exercises type-specific services, navigation, stock/type names,
repair callbacks, red-name policy and converted NPC handlers. Scripted exchange
coverage includes stale requests, profession priority and cancellation before
the player-side reply arrives. It also loads the Wild Rush quest's real merchant
registrations and checks NPC entry through `server.player.hasJob()`, including
multiple professions, level requirements and completed quests. NPC clicks run
quest eligibility checks before entering the merchant's own dialogue.
