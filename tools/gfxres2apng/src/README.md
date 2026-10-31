# gfxres2apng

`gfxres2apng` converts a `mir2x_res` tree into a new, APNG based assets tree.
The per-frame texture PNGs and the emoji sprite sheets become one
[APNG](https://wiki.mozilla.org/APNG_Specification) per animation sequence.
Every other file, such as fonts, maps, sounds and the texture types that are
not converted, is copied unchanged, so the output is a complete mirror of the
input tree, see [Output tree](#output-tree). With `--skip-unconverted` the
copy is skipped and the output holds only the converted types.

The old format stores one PNG per frame. The frame index, the sequence identity
and the frame's draw offset are all encoded in the filename. The new format
stores the whole sequence in one APNG:

- the frame index becomes the APNG frame order,
- all frames share one canvas, so one x/y offset in the filename applies to
  every frame,
- the sequence identity is encoded in the filename's higher bits.

The script never modifies the source resources.

- [Requirements](#requirements)
- [Usage](#usage)
- [Output tree](#output-tree)
- [Texture types](#texture-types)
- [Input filename](#input-filename)
- [Output filename](#output-filename)
- [Old to new filename mapping](#old-to-new-filename-mapping)
- [Sequence grouping](#sequence-grouping)
- [Canvas, offset and placeholders](#canvas-offset-and-placeholders)
- [APNG encoding](#apng-encoding)
- [Verification](#verification)
- [Expected results](#expected-results)
- [Tests](#tests)
- [Adding a texture type](#adding-a-texture-type)

## Requirements

- Python 3.8 or newer.
- [Pillow](https://pypi.org/project/pillow/), tested with 12.3:
  `pip install pillow`.
- Optional: `apngopt` (from apng-optimizer) for smaller files, see `--apngopt`.

## Usage

```sh
python3 tools/gfxres2apng/src/gfxres2apng.py \
    --res-path 3rdparty/mir2x_res \
    --output-dir /tmp/mir2x_apng \
    --jobs 8
```

This converts `<res-path>/texture/<type>` to `<output-dir>/texture/<type>` and
`<res-path>/emoji` to `<output-dir>/emoji`, and copies every other file unless
`--skip-unconverted` is given. The
directory tree under `--res-path` is fixed, so it is the only input option.

| Option | Default | Meaning |
|---|---|---|
| `--res-path PATH` | required | `mir2x_res` root. |
| `--output-dir PATH` | required | Output root, a mirror of `--res-path`. Must be missing or empty except for `.git`, and must not overlap `--res-path`. |
| `--types a,b,...` | all | Comma-separated subset of `monster,npc,hero,hair,helmet,weapon,magic,equip,selectchar,proguse,emoji` to convert. Each type at most once. Types not listed are copied unchanged, or skipped with `--skip-unconverted`. |
| `--skip-unconverted` | copy | Don't copy any file outside the converted types. The output then holds only `texture/<type>` and `emoji` for the types in `--types`. |
| `--magic-record PATH` | `common/src/magicrecord.inc` | Magic sequence definitions, see [magic](#magic). The default path is relative to the script. |
| `--proguse-groups PATH` | `proguse_groups_v3.txt` | proguse animation and button groups, see [proguse](#proguse). The default path is next to the script. |
| `--delay MS` | `100` | Frame delay written into every frame, 1 to 65535 ms. proguse `anim` groups use their own delay, emoji use their fps. |
| `--jobs N` | CPU count | Worker processes. |
| `--apngopt PATH` | none | Run `apngopt -z2 -i15` on every output file. |
| `--no-verify` | verify | Skip decoding each output and comparing it with the source frames, and comparing each copied file with its source. |

Progress lines look like `gfxres2apng hero: 500/10816 sequences, 2652 frames`,
and `gfxres2apng copy: 20000/488743 files` for the copied files.

Notes:

- Types run one after another, then the remaining files are copied, unless
  `--skip-unconverted` is given. All steps share one worker pool.
- Any malformed name, out-of-range key, broken sequence or verification failure
  stops the run with an error. Nothing is clamped or skipped silently.

## Output tree

`--output-dir` becomes a mirror of `--res-path`:

- Each converted type's directory, `texture/<type>` or `emoji`, holds only the
  converted output. Source files of a converted type are never copied.
- Every other file is copied byte-for-byte (with its timestamps) to the same
  relative path. That includes the types not in `--types`, `texture/map`,
  `texture/item`, `font`, `map`, `sound`, `LICENSE` and `README.md`. Empty
  directories are created too, and symbolic links are copied as links.
- `<res-path>/.git` is not copied; the new tree has its own history.
- With `--skip-unconverted`, no file is copied and no directory is mirrored:
  the output holds only the converted types' directories. Use it to convert a
  few types quickly without copying the whole tree.
- `--output-dir` must be missing or empty. A `.git` directory in it is allowed
  and kept, so the output can be a checkout of the new assets repository. This
  rule means no stale file from an earlier run can remain.
- `--output-dir` must not be inside `--res-path` or contain it.

With the current `mir2x_res` and all types converted, the output tree is:

| Path | Content |
|---|---|
| `texture/monster`, `npc`, `hero`, `hair`, `helmet`, `weapon`, `magic`, `selectchar` | APNGs |
| `texture/equip` | renamed static PNGs |
| `texture/proguse` | APNGs and static PNGs |
| `emoji` | APNGs |
| `texture/map`, `texture/item` | copied, PNG |
| `font` | copied, TTF |
| `map` | copied, `.bin` |
| `sound/bgm`, `sound/seff` | copied |
| `LICENSE`, `README.md` | copied |

## Texture types

| Type | Supported | Notes |
|---|---|---|
| `monster` | yes | |
| `npc` | yes | Same layout as monster. |
| `hero` | yes | Body, with shadow and layer. |
| `hair` | yes | Has placeholder frames. |
| `helmet` | yes | Has placeholder frames. |
| `weapon` | yes | Also includes the weapon 44 effect from `texture/magic`. |
| `magic` | yes | Sequences come from `magicrecord.inc`, longer filename with head/tail points. |
| `equip` | yes | Static images, copied unchanged and renamed. |
| `selectchar` | yes | Characters in the select/create character screens. Hero layout, overlays padded to the body's length. |
| `proguse` | yes | UI images. Animations and button off/on/down states listed in `proguse_groups_v3.txt` become APNGs, everything else is copied unchanged. |
| `emoji` | yes | Reads `<res-path>/emoji`, not under `texture`. One sprite sheet per emoji becomes one APNG, see [emoji](#emoji). |
| `map`, `item` | no | Loaded by `PNGTexDB`, not animation sequences. Copied unchanged. |

## Input filename

Every source frame, except proguse (see [proguse](#proguse)) and emoji (see
[emoji](#emoji)), is named with 18
characters plus `.PNG`, the same format `client/src/pngtexoffdb.cpp` reads:

```text
KKKKKKKK S S XXXX YYYY
```

| Field | Chars | Meaning |
|---|---|---|
| `KKKKKKKK` | 0–7 | 32-bit key in hex. Layout depends on the type, see below. |
| `S` | 8 | Sign of dx: `0` negative, `1` positive. |
| `S` | 9 | Sign of dy: `0` negative, `1` positive. |
| `XXXX` | 10–13 | abs(dx) in hex. |
| `YYYY` | 14–17 | abs(dy) in hex. |

Example: `0000000510000C0012.PNG` is key `0x00000005`, dx = +12, dy = -18.

The frame is drawn at `anchor + (dx, dy)`, so the anchor is at `(-dx, -dy)`
relative to the frame's top-left pixel.

### Old key layouts

monster, npc (see `monsterwil2png`, `npcwil2png`, `client/src/monster.cpp`,
`client/src/clientnpc.cpp`), key must be below 2^24:

| Bits | Field |
|---|---|
| 0–4 | frame |
| 5–7 | direction |
| 8–11 | motion |
| 12–22 | look ID |
| 23 | shadow |

hero, hair, helmet, weapon (see `herowil2png`, `hairwil2png`, `helmetwil2png`,
`weaponwil2png`, `client/src/hero.cpp`):

| Bits | Field | Present in |
|---|---|---|
| 0–4 | frame | all |
| 5–7 | direction | all |
| 8–13 | motion | all |
| 14–21 | dress / hair / helmet / weapon ID | all |
| 22 | gender | all |
| 23 | shadow | hero, weapon |
| 24 | layer | hero |

Key limits: hero below 2^25, weapon below 2^24, hair and helmet below 2^23.

magic (see `magicwil2png`):

| Bits | Field |
|---|---|
| 0–23 | image index in the WIL file |
| 24–31 | WIL file index |

The magic WIL files are, in index order: `Magic`, `MagicEx`, `MagicEx2`,
`MonMagic`, `MonMagicEx`, `MonMagicEx2`, `MonMagicEx3` and `MonMagicEx4`.
`mir2x_res` also has images with file index 15 (`0F` prefix), three of which
`client/src/fixedlocmagic.cpp` uses directly.

## Output filename

The output filename is a fixed number of uppercase hex digits plus `.PNG`.
For every type except proguse and emoji, the lowest 24 bits are the sequence's
offset:

| Bits | Field |
|---|---|
| 0–11 | x offset, 12-bit two's complement, -2048 to 2047 |
| 12–23 | y offset, 12-bit two's complement, -2048 to 2047 |

The offset has the same meaning as the old dx/dy. Every APNG frame is drawn at
`anchor + (x, y)`. All bits not listed below are zero.

### monster, npc

14 hex digits, 56 bits:

| Bits | Field |
|---|---|
| 0–23 | offset |
| 24–26 | direction |
| 27–30 | motion |
| 31 | layer, always 0 |
| 32 | shadow |
| 33–43 | look ID |
| 44–55 | zero |

Example: `00000005FEEFF4.PNG` is look 0, motion 0, direction 5, no shadow,
offset (-12, -18).

### hero, hair, helmet, weapon

14 hex digits, 56 bits:

| Bits | Field |
|---|---|
| 0–23 | offset |
| 24–26 | direction |
| 27–32 | motion |
| 33 | layer: hero layer bit, or 1 for the weapon effect, 0 otherwise |
| 34 | shadow: hero and weapon only, 0 otherwise |
| 35 | gender |
| 36–43 | dress / hair / helmet / weapon ID |
| 44–55 | zero |

Examples:

- `00000005FD300E.PNG` is hero dress 0, gender 0, motion 0, direction 5,
  offset (14, -45).
- `0002C205FCBFD0.PNG` is weapon 44, layer 1 (effect), gender 0, motion 0,
  direction 5, offset (-48, -53).

#### Weapon 44 effect

The glow drawn over weapon 44 is stored in the magic resource,
`MonMagicEx4.wil` (file index 7), image indices 1900 to 5815
(old keys `0x0700076C` to `0x070016B7`). It uses the same anchor and slot
layout as the weapon resource:

```text
image index = base + motion * 80 + direction * 10 + frame
base        = 1900 for male (gender 1), 3900 for female (gender 0)
```

Each gender has 25 motion slots of 80 images. Slots 3820–3899 are empty
padding. The converter writes the effect into `texture/weapon` as weapon 44
with layer 1 and no shadow, so the client can draw it as an extra weapon layer.

The effect also has gfx motions 1 (ARROWATTACK, 6 frames) and 20 (ONHORSEDIE,
10 frames), which the weapon resource doesn't have. They are kept, since
`client/src/hero.cpp` expects these frame counts.

Each effect sequence's frames must be contiguous from 0. Every effect image in
the range must belong to a sequence. Otherwise the run fails.

### magic

26 hex digits, 104 bits:

| Bits | Field |
|---|---|
| 0–23 | offset |
| 24–35 | head x, 12-bit two's complement |
| 36–47 | head y, 12-bit two's complement |
| 48–59 | tail x, 12-bit two's complement |
| 60–71 | tail y, 12-bit two's complement |
| 72–75 | direction index, `[0, gfxDirType)` |
| 76–95 | image index of the sequence's base gfxID, same for every direction, below 2^20 |
| 96–103 | file index, 0 to 255 |

The first 8 digits, `FF IIIII D`, are the sequence identity: the 2-digit file,
the 5-digit image and the direction. The converted data uses files `00` to
`07` and `0F`, and images up to `0x16B7`. Magic added later, not converted
from `mir2x_res`, can use any other file number, so the client must accept
any value in all 8 digits.

Example: `00001A45000000001044FD1FF4.PNG` is file 0, image 420, direction 5,
head (68, 1), tail (0, 0), offset (-12, -47).

The offset, head and tail all use the same anchor:

- **Head** is the hit point of a flying magic. When it reaches the target, the
  magic hits. It is also the rotation center. Heads are seeded from
  `MagicGfxEntry::targetOffList` in `magicrecord.inc`, and are (0, 0) for
  sequences without one.
- **Tail** is a point in the tail area of the gfx (for example, the tail of a
  fireball). The `tail -> head` vector is the gfx's flying direction, so the
  client can rotate the gfx around the head to point at the target. Tails are
  (0, 0) for now and will be set in the editor (`followuidmagiceditor`).
- **head == tail** means the gfx is isotropic and is never rotated.

The direction index is not a real flying direction: it selects one of the
`gfxDirType` pre-rendered frame sequences of an entry. The client picks the
nearest pre-rendered direction and rotates the rest using `tail -> head`.

SDL can't read private APNG chunks, so head and tail are stored in the filename
and there is one head/tail pair per APNG.

### equip

Equip images are static UI images (the body, equipment and hair in the player
status board), not animations. A PNG is already a valid 1-frame APNG, so each
file is copied byte-for-byte and only renamed. That way the client parses one
filename format for every `PNGTexOffDB` type.

14 hex digits, 56 bits:

| Bits | Field |
|---|---|
| 0–23 | offset, the old dx/dy unchanged |
| 24–55 | the old 32-bit key, unchanged |

Example: `000000000000160079.PNG` (key 0, dx -22, dy -121) becomes
`00000000F87FEA.PNG`.

Keys must be unique, and the offset must fit in 12 bits. Equip images are not
trimmed or re-encoded, and `--apngopt` and `--delay` don't apply to them.

### selectchar

The characters drawn by `ProcessSelectChar` and `ProcessCreateChar`. Old key,
see `charGfxBaseID()` in both files, key must be below 2^15:

| Bits | Field |
|---|---|
| 0–4 | frame |
| 5–8 | motion |
| 9 | gender, 1 is male |
| 10–12 | job gfx index, `jobf::jobGfxIndex()` |
| 13 | magic overlay |
| 14 | shadow |

The new name uses the [hero layout](#hero-hair-helmet-weapon), 14 hex digits:

| Bits | Field |
|---|---|
| 0–23 | offset |
| 24–26 | direction, always 0 |
| 27–32 | motion |
| 33 | layer: 1 for the magic overlay, like the weapon effect |
| 34 | shadow |
| 35 | gender, 1 is male |
| 36–43 | job gfx index |
| 44–55 | zero |

Example: `00000A08FA1FA5.PNG` is the magic overlay of job 0, male, motion 1,
offset (-91, -95).

The client draws the body, then the shadow and the magic overlay at the same
frame index as the body. So:

- Every body sequence (no overlay, no shadow) must be contiguous from frame 0.
  Its frame counts match `client/src/selectcharframecount.inc`.
- Shadow and overlay sequences are padded with blank frames to the body's frame
  count, both before their first and after their last real frame, so APNG frame
  `i` of every layer lines up with body frame `i`. For example, an overlay with
  frames 4–19 over a 20-frame body becomes a 20-frame APNG with frames 0–3
  blank.
- The run fails if a shadow or overlay has no body, or has a frame index
  beyond the body's frame count.

In the current data, 5 of the 14 overlays start after frame 0 and most end
before the body's last frame. Shadows always match their body.

### proguse

proguse holds the client's UI images, loaded by `PNGTexDB`
(`client/src/progusedb.hpp`). The old name is the 8-hex key plus `.PNG`, with
no offset; one file, `00000590.png`, has a lowercase extension.

Most images are static. The animations and the buttons whose off/on/down
states are separate images are listed, one APNG per line, in
`proguse_groups_v3.txt` (`--proguse-groups`). The list was generated from the
client code and image sizes, then reviewed by hand:

```text
anim    FIRST-LAST  DELAY  [name=KEY]       frames FIRST, FIRST+1, ..., LAST, DELAY ms each
button  OFF  ON  DOWN      [name=KEY]       3 frames: frame 0/1/2 = off/on/down
```

- Keys are 8 hex digits. Everything after `#` is a comment.
- Every state has an image. A key may repeat inside a group, so states can
  share an image. A button with no off image of its own uses its on image as
  off: `0000001C 0000001C 0000001D`.
- States don't need to be consecutive or ascending: `00000033 00000033 00000032`.
- `name=KEY` sets the APNG name; by default it's the smallest key in the group.
  KEY must be one of the group's keys.
- A key may be in more than one group, but every APNG name must be unique.
  `00000300` and `00000302` are in two groups, because `queststateboard` uses
  `300` as off while `auctionboard` and `pullmenu` use `301` as off:

  ```text
  button  00000301  00000300  00000302  name=00000301
  button  00000300  00000300  00000302
  ```

- All images of a group must have the same size.

The new name is the 8-hex key plus `.PNG`:

| Bits | Field |
|---|---|
| 0–31 | the APNG name key for a group, or the old key for a static image |

- APNGs are not trimmed: every frame keeps the full source size and there is no
  offset, because widgets take their size from the texture.
- Button frames are selected by state, not played, so their delay (`--delay`)
  is unused. `anim` frames use the group's DELAY.
- Every key not in any group is a static image, copied byte-for-byte to
  `KKKKKKKK.PNG` (the extension is normalized to `.PNG`). Static names never
  clash with APNG names, because an APNG name is one of its group's keys.

### emoji

emoji is loaded by `EmojiDB` (`client/src/emojidb.hpp/.cpp`) from
`<res-path>/emoji`, not from `texture`. Each old file is a sprite sheet holding
all frames of one emoji. Its name is 24 hex digits plus `.PNG`:

```text
SS UUUU FF CC PP WWWW HHHH hhhh
```

| Field | Chars | Meaning |
|---|---|---|
| `SS` | 0–1 | emoji set |
| `UUUU` | 2–5 | emoji subset, the emoji in its set |
| `FF` | 6–7 | frame index, always `00` in the files; the client adds the frame index here at lookup |
| `CC` | 8–9 | frame count |
| `PP` | 10–11 | fps |
| `WWWW` | 12–15 | frame width |
| `HHHH` | 16–19 | frame height |
| `hhhh` | 20–23 | `frameH1`, the frame's height above the text baseline |

The sheet is a grid of `WWWW × HHHH` cells, `sheet width / WWWW` cells per
row. Frame `i` is cell `i`, row-major: column `i % cols`, row `i / cols`.

Each sheet becomes one APNG:

- Frames are cells 0 to `CC - 1`, each `WWWW × HHHH`, not trimmed, no offset.
- Cells after `CC - 1` are spare grid slots and are dropped. The client never
  draws them, because it plays frame `n % CC`. In the current data 3 sheets
  have spare cells with a few stray edge pixels bled from the neighbouring
  frame: `010020000F0A006400640064` (cell 15), `010021000E0A006400640064`
  (cells 14, 15) and `01002A00120A006400640064` (cells 18, 19).
- Blank frames inside `CC` are kept: `01001900150A006400640064` frame 1 and
  `01003E000E0A006400640064` frame 0.
- Every frame has its own delay in its `fcTL` chunk, written as exactly
  `1/PP` s (`delay_num = 1`, `delay_den = PP`), so 6 fps is exactly 1/6 s.
  All frames of an emoji have the same delay today, but the client must read
  the delay per frame, so later APNGs can have a different delay per frame.
- Frame count, frame width and frame height are read from the APNG and are not
  in the new name.

The new name is 11 hex digits plus `.PNG`, `00SSUUUUhhh`:

| Bits | Field |
|---|---|
| 0–11 | `frameH1`, 12-bit two's complement, -2048 to 2047 |
| 12–43 | emoji key `0x00SSUUUU` = set << 16 \| subset |

The key is the same 24-bit value as the `<emoji id="...">` tag's id in the
client's XML layout, see [emoji client changes](#emoji-client-changes).

## Old to new filename mapping

This section is the reference for rewriting the client's texture loading: for
every type it says which APNG, and which frame in it, an old frame went to.
All examples are real files from the current `mir2x_res`.

### Common rules

Every old key is `sequence fields | frame index` (except magic and equip, see
below). The conversion splits it:

1. **Sequence fields → new identity.** The fields are moved to new bit
   positions above the offset. The new identity is what the client should use
   as its lookup key, because the full filename also contains the offset:

   | Types | Filename | New identity | As hex digits |
   |---|---|---|---|
   | monster, npc, hero, hair, helmet, weapon, selectchar | 14 digits | `value >> 24` | first 8 digits |
   | equip | 14 digits | `value >> 24`, equals the old key | first 8 digits |
   | magic | 26 digits | `value >> 72` = `dir \| image << 4 \| file << 24` | first 8 digits, `FF IIIII D` |

2. **Frame index → APNG frame index.** Old frame `f` is APNG frame `f`. For
   magic, `f` counts from the first frame of the sequence. selectchar shadow
   and overlay APNGs have extra blank frames that have no old file.

3. **Per-frame dx/dy → one offset (X, Y).** The old dx/dy of each frame are
   not stored anymore. For a sequence with frames `i`, old offsets
   `(dx_i, dy_i)` and visible bounding boxes `(l_i, t_i, r_i, b_i)` (alpha >
   0, relative to the frame's top-left, right/bottom exclusive), over the
   non-placeholder frames:

   ```text
   X = min(dx_i + l_i)          W = max(dx_i + r_i) - X
   Y = min(dy_i + t_i)          H = max(dy_i + b_i) - Y
   ```

   `W × H` is the APNG size. Old frame `i`'s top-left pixel is at
   `(dx_i - X, dy_i - Y)` in APNG frame `i`; this can be negative, because the
   frame's own transparent margin is trimmed. So drawing changes from

   ```text
   old: draw(texture of frame i, x + dx_i, y + dy_i)
   new: draw(APNG frame i,        x + X,    y + Y)
   ```

   and produces exactly the same pixels. If every frame is a placeholder,
   `(X, Y) = (0, 0)` and the APNG is 1×1. equip is not trimmed: `(X, Y)` is
   the old `(dx, dy)`.

4. **Hex digits.** Names are uppercase, zero-padded to a fixed width, with a
   `.PNG` extension. The offset is bits 0–23, so it is always the last 6 hex
   digits, `YYYXXX`, each 12-bit two's complement (`0xFFF` = -1, `0x800` =
   -2048). The identity starts at bit 24, a hex-digit boundary, so it can be
   read directly from the leading digits.

### monster, npc

| Field | Old key bits | New value bits | New identity bits |
|---|---|---|---|
| frame | 0–4 | APNG frame index | |
| direction | 5–7 | 24–26 | 0–2 |
| motion | 8–11 | 27–30 | 3–6 |
| layer | | 31, always 0 | 7 |
| shadow | 23 | 32 | 8 |
| look | 12–22 | 33–43 | 9–19 |

```text
frame    = oldKey & 0x1F
identity = ((oldKey >>  5) & 0x07)                # direction
         | ((oldKey >>  8) & 0x0F) <<  3          # motion
         | ((oldKey >> 23) & 0x01) <<  8          # shadow
         | ((oldKey >> 12) & 0x7FF) << 9          # look
name     = "%014X.PNG" % (identity << 24 | (Y & 0xFFF) << 12 | (X & 0xFFF))

# inverse, old key of APNG frame f
oldKey   = f | (identity & 0x07) << 5 | ((identity >> 3) & 0x0F) << 8
             | ((identity >> 9) & 0x7FF) << 12 | ((identity >> 8) & 0x01) << 23
```

Example, monster look 21, motion 3, direction 1, shadow:

| Old file | Frame | dx, dy | Visible box |
|---|---|---|---|
| `008153201100030004.PNG` | 0 | 3, 4 | (0, 0, 89, 23) |
| `008153211000020007.PNG` | 1 | 2, -7 | (0, 0, 85, 33) |

- identity = 1 | 3 << 3 | 1 << 8 | 21 << 9 = `0x2B19`
- X = min(3 + 0, 2 + 0) = 2, Y = min(4 + 0, -7 + 0) = -7
- W = max(3 + 89, 2 + 85) - 2 = 90, H = max(4 + 23, -7 + 33) + 7 = 34
- new file `00002B19FF9002.PNG`: identity `00002B19`, Y `FF9`, X `002`,
  90×34, 2 frames.

npc uses the same mapping. Example: `0003300000001A0057.PNG` (look 51,
motion 0, direction 0, dx -26, dy -87, visible box starts at x 1) becomes
`00006600FA9FE7.PNG`, offset (-25, -87).

### hero, hair, helmet, weapon

| Field | Old key bits | New value bits | New identity bits |
|---|---|---|---|
| frame | 0–4 | APNG frame index | |
| direction | 5–7 | 24–26 | 0–2 |
| motion | 8–13 | 27–32 | 3–8 |
| layer | 24, hero only | 33 | 9 |
| shadow | 23, hero and weapon | 34 | 10 |
| gender | 22 | 35 | 11 |
| dress / hair / helmet / weapon ID | 14–21 | 36–43 | 12–19 |

Fields a type doesn't have are 0. The weapon effect sets layer to 1, see
[below](#weapon-44-effect-1).

```text
frame    = oldKey & 0x1F
identity = ((oldKey >>  5) & 0x07)                # direction
         | ((oldKey >>  8) & 0x3F) <<  3          # motion
         | ((oldKey >> 24) & 0x01) <<  9          # layer
         | ((oldKey >> 23) & 0x01) << 10          # shadow
         | ((oldKey >> 22) & 0x01) << 11          # gender
         | ((oldKey >> 14) & 0xFF) << 12          # ID
name     = "%014X.PNG" % (identity << 24 | (Y & 0xFFF) << 12 | (X & 0xFFF))

# inverse, old key of APNG frame f, layer 0 (or hero layer 1)
oldKey   = f | (identity & 0x07) << 5 | ((identity >> 3) & 0x3F) << 8
             | ((identity >> 12) & 0xFF) << 14 | ((identity >> 11) & 1) << 22
             | ((identity >> 10) & 1) << 23 | ((identity >> 9) & 1) << 24
```

Since the frame index and direction occupy the low bits in both layouts,
`identity & 0x07` is the direction and `identity >> 3` is
`motion | layer << 6 | shadow << 7 | gender << 8 | ID << 9`.

Example, hero dress 1, motion 4, direction 1, layer 1, one frame: old
`01004420100007002B.PNG` (dx 7, dy -43, visible box (1, 9, 38, 70)) becomes
`00001221FDE008.PNG`:

- identity = 1 | 4 << 3 | 1 << 9 | 1 << 12 = `0x1221`
- X = 7 + 1 = 8, Y = -43 + 9 = -34, size 37×61, the 1-pixel left and 9-pixel
  top margins of the old 40×74 image are trimmed.

Example, hair 4, male, motion 19 (DIE), direction 5, frames
`004113A0...` to `004113A9...` become `0000489DFCA00D.PNG`, 45×53, 10 frames,
offset (13, -54). Old frame 5, `004113A51125E7262F.PNG` (+9703, +9775, fully
transparent), is a placeholder: APNG frame 5 is blank and doesn't affect the
offset or size.

Example, weapon 44, male, motion 0, direction 2, shadow: `00CB0040...` to
`00CB0043...` become `0002CC02001009.PNG`.

helmet example: helmet 5, male, motion 19, direction 4, `00415380...` to
`00415389...` become `0000589CFCC00D.PNG`.

#### Weapon 44 effect

The source is a magic key, `0x07000000 | image`, with image in 1900–5815.
It maps to a weapon identity with ID 44, layer 1, shadow 0:

```text
image     = oldKey & 0xFFFFFF
gender    = 1 if image < 3900 else 0          # male base 1900, female base 3900
rel       = image - (1900 if gender else 3900)
motion    = rel // 80
direction = rel % 80 // 10
frame     = rel % 10
identity  = direction | motion << 3 | 1 << 9 | gender << 11 | 44 << 12

# inverse, magic key of APNG frame f
oldKey    = 0x07000000 | (1900 if gender else 3900) + motion * 80 + direction * 10 + f
```

Example: `07000780000020001B.PNG` to `07000783...` (images 1920–1923, male,
motion 0, direction 2) become `0002CA02FE1FDF.PNG`: identity `0x2CA02`,
offset (-33, -31). It is drawn over the weapon APNG `0002C802...`, which has
the same ID, gender, motion and direction with layer 0 and shadow 0.

### magic

The identity is not a function of the old key's bits: it comes from
`magicrecord.inc`. Each old key `fileIndex << 24 | image` maps as follows,
first match wins:

1. **Referenced by an entry.** For an entry with `gfxID`, `gfxIDCount` and
   `gfxDirType`, if `oldKey = gfxID + d * gfxIDCount + f` with
   `0 <= d < gfxDirType` and `f` less than the APNG's frame count:

   ```text
   file      = gfxID >> 24
   image     = gfxID & 0xFFFFFF      # the entry's gfxID, not the first frame of direction d
   direction = d
   frame     = f
   head      = targetOffList[d], or (0, 0) if the entry has no targetOffList
   ```

   So the old client call
   `retrieve(gfxID + gfxDirIndex * gfxIDCount + frame)` becomes APNG
   `(gfxID, gfxDirIndex)`, frame `frame`. The APNG holds every contiguous
   frame on disk up to `gfxIDCount`, which can be more than `frameCount`. If
   several entries share the same gfxID and direction, the APNG is the
   longest of their sequences, and the shorter ones are its prefixes. An
   entry whose gfxID is in `MAGIC_RECORD_OVERRIDES` keeps exactly the
   override's frame count instead.
2. **Hardcoded sequence** in `MAGIC_HARDCODED_SEQUENCES`, if
   `oldKey = gfxID + d * gfxIDCount + f` with `0 <= d < gfxDirType` and
   `f < frameCount`: `(gfxID >> 24, gfxID & 0xFFFFFF, d)`, frame `f`.
3. **Unreferenced run**, first old key `first`:
   `(first >> 24, first & 0xFFFFFF, 0)`, frame `oldKey - first`. If every
   frame of the run is fully transparent, the run is dropped and its keys
   have no APNG.

Keys `0x0700076C`–`0x070016B7` don't map to magic, they are the
[weapon 44 effect](#weapon-44-effect-1).

The name packs every field on hex-digit boundaries, so it can be read directly:

```text
FF IIIII D TTT TTT HHH HHH YYY XXX
│  │     │ │   │   │   │   │   └─ X offset   (bits  0-11)
│  │     │ │   │   │   │   └───── Y offset   (bits 12-23)
│  │     │ │   │   │   └───────── head x     (bits 24-35)
│  │     │ │   │   └───────────── head y     (bits 36-47)
│  │     │ │   └───────────────── tail x     (bits 48-59)
│  │     │ └───────────────────── tail y     (bits 60-71)
│  │     └─────────────────────── direction  (bits 72-75)
│  └───────────────────────────── image      (bits 76-95)
└──────────────────────────────── file       (bits 96-103)
```

```text
identity = direction | image << 4 | file << 24     # 8 hex digits, FF IIIII D
name     = "%026X.PNG" % (identity << 72
                         | (tailY & 0xFFF) << 60 | (tailX & 0xFFF) << 48
                         | (headY & 0xFFF) << 36 | (headX & 0xFFF) << 24
                         | (Y & 0xFFF) << 12 | (X & 0xFFF))
```

Examples:

- **Entry with direction and head.** `magicrecord.inc` line 60, gfxID
  `0x000003D4` (image 980), `frameCount` 3, `gfxIDCount` 10, `gfxDirType` 16,
  `targetOffList[2]` = (62, -45). Direction 2 starts at 980 + 2 × 10 = 1000:
  `000003E81000310035.PNG`, `000003E9...`, `000003EA...` become
  `00003D42000000FD303EFC8031.PNG`, read as file `00`, image
  `003D4`, direction `2`, tail (0, 0), head y `FD3` = -45, head x `03E` = 62,
  Y `FC8` = -56, X `031` = 49.
- **Longest prefix and shared frames.** Two entries use gfxID `0x010007BC`
  with `frameCount` 10 and 15: one APNG `01007BC0000000000000F20FCE.PNG` with
  15 frames (`010007BC` to `010007CA`) serves both. Another entry uses gfxID
  `0x010007C6` with 5 frames, so `010007C6` to `010007CA` are also APNG
  `01007C60...`, frames 0–4. These 5 are the only old frames stored in two
  APNGs. With 74 dropped blank frames and 3 padded blank frames, 6,093
  source frames become 6,027 APNG frames.
- **Hardcoded image.** `0F0000DC00001C0015.PNG` becomes
  `0F000DC0000000000000FEBFE4.PNG`, file 15, image 220, offset (-28, -21).
- **Teleport.** Images `0x956`–`0x97D` of file 0 are one contiguous run of
  40 images, but two animations: someone arriving, then someone leaving.
  `0000095600000600B6.PNG` to `0000096900004C001D.PNG` become
  `00009560000000000000F4AFB4.PNG`, and `0000096A00004C001D.PNG` to
  `0000097D00000600B6.PNG` become `000096A0000000000000F4AFB4.PNG`, 20
  frames each.
- **Directions with blank slots.** Images 3470–3565 (`0xD8E`–`0xDED`) of
  file 0 are one contiguous run of 96 images:
  - 3470–3549: 8 directions, 10 slots apart. Each has 3 frames, then 7 fully
    transparent slots. Direction `d`, `0x00000D8E + d * 10` to `+ 2`, becomes
    APNG `(0, 0xD8E, d)`, for example `0000D8E0000000000000FE2FF0.PNG`, 3
    frames.
  - 3550–3559: fully transparent, not part of any animation.
  - 3560–3565: another animation, `0000DE80000000000000FFB00D.PNG`, 6 frames.

  The 7 blank slots after each direction are unreferenced runs; the last
  direction's 7 and the 10 at 3550–3559 form one 17-frame run. All 8 runs
  are fully transparent, so they are dropped.
- **Unreferenced run.** `010004F6...`, `010004F7...`, `010004F8...` (images
  1270–1272 of file 1, no entry uses them) become
  `01004F60000000000000FE0016.PNG`, offset (22, -32).

### magic client changes

Not done yet. This section lists the migration order and everything the client
needs to change for magic. File:line references are to `client/src` at
commit `f05d2151` unless another directory is given.

#### Migration order

1. **Converter emits the new names. Done.** `gfxres2apng.py` writes
   `FF IIIII D ...` names. It still reads the current `magicrecord.inc`, where
   `gfxID` is the old key of the first frame, `file << 24 | image`, and
   directions are `gfxIDCount` images apart.
2. **Convert the resources. Done** for the test tree
   (`--types magic --skip-unconverted`): 990 APNGs, 6,027 frames. The magic
   directory of the real resource repository is converted when the client
   change below lands.
3. **Change the client magic loader and every use site, and rewrite
   `magicrecord.inc`, in one commit.** The client can't read both formats:
   the old `texture/magic` has one PNG per frame, keyed by old key, and the
   new one has one APNG per sequence, keyed by identity.
4. **Convert the real resource repository** and rebuild with it. Old
   resources don't work with the new client, and new resources don't work
   with the old client.
5. **Later, in `followuidmagiceditor`:** set the tails and fix the heads, and
   write them back into the APNG names.

After step 3, `magicrecord.inc` no longer has the old `gfxID` and
`gfxIDCount`. To re-run the converter on the original WIL-derived PNGs, use
the `magicrecord.inc` from before step 3 (from git), or change the converter
to read the new fields; the new `gfxID` is enough to find the base image, but
not the direction stride.

#### magicrecord.inc

- Rewrite every `gfxID` from the old key `file << 24 | image` to 7 hex
  digits, `file << 20 | image`. Then the APNG identity of direction `d` is
  `gfxID << 4 | d`, the first 8 digits of the name. For example `0x000003D4`
  stays `0x00003D4`, `0x010007BC` becomes `0x01007BC`, `0x0F0000DC` becomes
  `0x0F000DC`. `SYS_U32NIL` stays the "no gfx" marker; it is not a valid
  28-bit `gfxID`.
- Drop `gfxIDCount`. It was only the stride between directions in the old
  key space; each direction is its own APNG now.
- `frameCount` is still needed where an entry plays a prefix of its APNG
  (for example two entries on `0x01007BC`, 10 and 15 frames, share one
  15-frame APNG). Make it optional: when it is 0, use the APNG's frame count.
  Keep the existing assert as `frameCount <= APNG frame count`.
- `gfxDirType` stays: it is the number of direction APNGs, `[0, gfxDirType)`.
  Some directions may have no APNG (see `MAGIC_OMITTED_DIRECTIONS`); the
  client still looks them up, and when the lookup returns null it skips
  drawing that gfx instead of failing.
- `targetOffList` is now in the names as the head. Keep it until the client
  reads heads from the database, then remove it.
- Fix 阴阳法环 (MagicEx2 images 290–305, like 魔法盾): 290–298 is the
  spawn, 300–302 the sustain loop, 303–305 the hit. Today 运行 loops only
  the last spawn frame `0x0200012A` (298), and 挨打 plays `0x0200012C` with 6
  frames (300–305). Change 运行 to `0x0200012C`, 3 frames, `loop = 1`,
  change 挨打 to `0x0200012F`, 3 frames, and drop the `0x0200012A` entry.
  Then remove `0x0200012C` from `MAGIC_RECORD_OVERRIDES`, `0x0200012A` from
  `MAGIC_RECORD_SKIPPED` and `0x0200012F` from `MAGIC_HARDCODED_SEQUENCES`
  in the converter.
- Fix 风震天 (MagicEx images 1980–2009, verified in the original game). 击风
  and 风震天 share 启动 `0x010007B2`, attached to the caster. 击风 运行
  plays `0x010007BC`, all 15 frames (1980–1994), at the target, then 裂解
  `0x010007D0` (2000–2009). 风震天 运行 is only `0x010007C6` (1990–1994):
  it starts at the caster's location and moves straight along one of the 8
  directions, then after several grids becomes 裂解 `0x010007D0`. Today
  风震天 has two 运行 entries, `0x010007BC` (10 frames) and `0x010007C6`,
  and `DBCOM_MAGICGFXENTRY` returns the first one, so it plays 1980–1989
  instead. Drop the `0x010007BC` entry from 风震天. The converter needs no
  change: `01007BC0…` (15 frames) and `01007C60…` (5 frames) both exist,
  and 1990–1994 are in both on purpose.
- `MagicGfxEntryRef` and `modColor`, `speed`, `loop`, sound and the other
  fields don't change.
- `common/src/buffrecord.hpp:211` `icon.gfxID` is not a magic key; it does
  not change.

#### Loader

- `g_magicDB` is a `PNGTexOffDB` (`main.cpp:29`, `main.cpp:75`), loaded from
  `texture/magic.zsdb` in `initview.cpp:59`. Replace it with a magic DB that
  loads one APNG per entry with `SDLDevice::loadPNGAnimation`, the same way
  as `EmojiDB`.
- Database key: the first 8 hex digits of the name, the identity
  `file << 24 | image << 4 | dir`. Accept any value in all 8 digits, not only
  the converted files `00`–`07` and `0F`.
- Parse from the name, each a 12-bit two's complement with
  `hexstr::to_2comp(name + pos, 3)`: tail y at chars 8–10, tail x 11–13,
  head y 14–16, head x 17–19, Y offset 20–22, X offset 23–25.
- Element: one texture per frame, the frame count, the offset, head and tail.
  All frames of an APNG share one offset: the converter aligned and trimmed
  them to one canvas. Per-frame delays are in the file (`--delay`, 100 ms by
  default) but magic timing comes from `magicrecord.inc` `speed`, so the
  client ignores them.
- `retrieve(identity, frame)` returns the frame's texture and the shared
  offset, `nullptr` texture if `frame >= frameCount`.
- Frames that were missing on disk (`MAGIC_PADDED_KEYS`) or were 4×2 /
  huge-offset placeholders are fully transparent frames in the APNG. The old
  client got `nullptr` or drew the placeholder off screen; the new client
  draws a transparent frame, so nothing visible changes and no special case
  is needed.
- An APNG read by a plain PNG decoder shows only frame 0.

#### Use sites

Every site computes `gfxID + frame + dirIndex * gfxIDCount`. Each becomes
`retrieve(gfxID << 4 | dirIndex, frame)`, with `dirIndex = 0` when
`gfxDirType` is 1. The drawing code, `drawTexture(tex, x + offX, y + offY)`,
doesn't change: the new offset keeps the old sign convention.

| Site | Today | New |
|---|---|---|
| `attachmagic.cpp:16–27` `AttachMagic::drawShift` | `gfxID + frame()` or `+ m_gfxDirIndex * gfxIDCount` | `(gfxID << 4 \| m_gfxDirIndex, frame())` |
| `attachmagic.cpp:40–51` `Thunderbolt::drawShift` | same | same |
| `followuidmagic.cpp:60–75` `FollowUIDMagic::frameTexID` | same, returns one `uint32_t` | return `(identity, frame)` |
| `followuidmagic.cpp:111–112` `drawViewOff` | `retrieve(texID)` | `retrieve(identity, frame)` |
| `followuidmagic.hpp:59–60` `targetOff` | `targetOffList[m_gfxDirIndex]` | the head from the DB |
| `fixedlocmagic.cpp:19–20` | `gfxID + gfxDirOff + frame()` | `(gfxID << 4 \| max(m_gfxDirIndex, 0), frame())` |
| `fixedlocmagic.cpp:30` `FireAshEffect_RUN` | `0X0F0000DC` | `(0x0F000DC0, 0)` |
| `fixedlocmagic.cpp:86` `IceThorn_RUN` | `0X0F000104` | `(0x0F001040, 0)` |
| `fixedlocmagic.cpp:95` `IceThorn_RUN` | `0X0F000105` | `(0x0F001050, 0)` |
| `motioneffect.cpp:40–46` `MotionEffect::frameTexID` | `gfxID + gfxIndex (+ (direction - DIR_BEGIN) * gfxIDCount)` | `(gfxID << 4 \| dir, gfxIndex)` |
| `motioneffect.cpp:56–57` | `retrieve(texID)` | `retrieve(identity, frame)` |
| `motioneffect.cpp:26–27`, `basemagic.hpp:62–63` | assert `frameCount <= gfxIDCount` | assert `frameCount <= APNG frame count`, or drop |
| `tools/followuidmagiceditor/src/magicdrawarea.cpp:114` | `gfxID + m_frame + gfxDirIndex * gfxIDCount` | `(gfxID << 4 \| gfxDirIndex, m_frame)` |
| `tools/followuidmagiceditor/src/magicframedb.hpp` | one PNG per old key | one APNG per identity |

`basemagic.hpp:75` (`gfxDirIndex() < gfxDirType`) and the frame timing in
`basemagic.hpp:165–196` (`frame()`, `absFrame()`, `done()`) don't change.

#### Weapon 44 glow

Images `0x0700076C`–`0x070016B7` are no longer in `texture/magic`. They are
[weapon 44, layer 1](#weapon-44-effect-1) in `texture/weapon`. No client code
draws them from `g_magicDB` today. To show the glow, draw the layer-1 weapon
APNG over weapon 44 with the same motion, direction and frame, as part of the
weapon change.

#### Rotation

New, not in the client today. Each APNG has a head and a tail in its name
(see [magic](#magic)). With `head == tail` (every APNG today, tails are all
(0, 0) and heads only come from `targetOffList`), draw as now. Otherwise,
`FollowUIDMagic` picks the nearest of the `gfxDirType` directions and rotates
the frame around the head by the angle between `tail -> head` and the vector
to the target, with `SDL_RenderTextureRotated`, and uses the head as the hit
point instead of `targetOffList`. This needs the editor to set the tails
first.

### equip

```text
identity = oldKey
name     = "%014X.PNG" % (oldKey << 24 | (dy & 0xFFF) << 12 | (dx & 0xFFF))
```

The file content is unchanged, so the offset is the old dx/dy, not trimmed.
Example: `01000B720000180010.PNG` (dx -24, dy -16) becomes
`01000B72FF0FE8.PNG`, the first 8 digits are the old key.

### selectchar

| Field | Old key bits | New value bits | New identity bits |
|---|---|---|---|
| frame | 0–4 | APNG frame index | |
| direction | | 24–26, always 0 | 0–2 |
| motion | 5–8 | 27–32 | 3–8 |
| magic overlay | 13 | 33 (layer) | 9 |
| shadow | 14 | 34 | 10 |
| gender | 9 | 35 | 11 |
| job gfx index | 10–12 | 36–43 | 12–19 |

```text
frame    = oldKey & 0x1F
identity = ((oldKey >>  5) & 0x0F) <<  3          # motion
         | ((oldKey >> 13) & 0x01) <<  9          # magic overlay
         | ((oldKey >> 14) & 0x01) << 10          # shadow
         | ((oldKey >>  9) & 0x01) << 11          # gender
         | ((oldKey >> 10) & 0x07) << 12          # job gfx index

# inverse, old key of APNG frame f, if that frame is not padding
oldKey   = f | ((identity >> 3) & 0x0F) << 5 | ((identity >> 11) & 1) << 9
             | ((identity >> 12) & 0x07) << 10 | ((identity >> 9) & 1) << 13
             | ((identity >> 10) & 1) << 14
```

The old client draws, for `frame = absFrame % frameCount`:

```text
old: base | frame,  base | shadowMask | frame,  base | magicMask | frame
new: APNG (layer 0, shadow 0), (layer 0, shadow 1), (layer 1, shadow 0), all frame `frame`
```

`frameCount` from `selectcharframecount.inc` equals the body APNG's frame
count, and the shadow and overlay APNGs have the same count. Old code skipped
missing overlay frames because `retrieve()` returned null; in the new APNG
those frames exist but are blank.

Examples, job 0, male, motion 1 (20-frame body):

- Overlay: `000022240100340011.PNG` to `00002233...` (frames 4–19) become
  `00000A08FA1FA5.PNG`, identity `0xA08`, 20 frames, frames 0–3 blank,
  offset (-91, -95).
- Shadow: `0000422011000A0071.PNG` to `00004233...` become
  `00000C0805FFD6.PNG`, identity `0xC08`, 20 frames, offset (-42, 95).

### proguse

`proguse_groups_v3.txt` is the mapping. For each line, in output order:

```text
anim    FIRST-LAST DELAY [name=KEY]  : old key FIRST + f  ->  APNG "%08X.PNG" % name, frame f
button  OFF ON DOWN [name=KEY]       : off/on/down        ->  APNG "%08X.PNG" % name, frame 0/1/2
name = KEY if given, else min(all keys of the line)
any other old key k                  ->  "%08X.PNG" % k, the unchanged static image
```

Client update notes:

- A `TritexButton` with `.texIDList{.off = A, .on = B, .down = C}` becomes one
  APNG, and the button state is the frame index. Find the line with the same
  OFF ON DOWN, and load its APNG.
- A button without `.off` maps to the line with `OFF == ON`, e.g.
  `.texIDList{.on = 0X1C, .down = 0X1D}` maps to `0000001C 0000001C 0000001D`.
  Its off state now draws the on image instead of nothing.
- Code that builds states itself, `texIDFunc` returning
  `std::array{A, B, C}[state]`, or three `ImageBoard`s as in `pullmenu.cpp`
  and `radioselector.cpp`, maps the same way.
- `00000300` and `00000302` appear in two groups. Each use site picks its group
  by its states: `00000301.PNG` for off `301` (`auctionboard.cpp:205`,
  `pullmenu.cpp:54`), `00000300.PNG` for off `300` (`queststateboard.cpp:101`).
- An `anim` group replaces code computing `base + frame`, e.g. the team flag
  cursor `0X00000210 + teamFlagIndex` becomes frame `teamFlagIndex` of
  `00000210.PNG`, and `WMDAniBoard`'s `0X04000010 + frame` becomes frame
  `frame` of `04000010.PNG`.

Examples:

- `00000005.PNG`, `00000006.PNG`, `00000007.PNG` (login button) become
  `00000005.PNG`, 3 frames.
- `0000001C.PNG`, `0000001D.PNG` (close button, no off image) become
  `0000001C.PNG`, frames `1C, 1C, 1D`.
- `00000210.PNG` to `0000021C.PNG` become `00000210.PNG`, 13 frames, 200 ms.
- `00000022.PNG` (control board title) is copied unchanged.

### proguse client changes

Everything the client needs to change for proguse. File:line references are to
`client/src` at commit `fa7d0ef9`; check them again before editing.

#### Loader

- The client loads proguse through `g_progUseDB`, a `PNGTexDB`
  (`pngtexdb.hpp/.cpp`, created in `main.cpp:66` and loaded from
  `texture/proguse.zsdb` in `initview.cpp:51`). `PNGTexDB::retrieve(key)` looks up
  the file whose name starts with the 8-hex key and decodes it with
  `g_sdlDevice->loadPNGTexture`.
- The converter output must still be packed into `proguse.zsdb` with
  `tools/zsdbmaker`, which the client build runs (`client/src/CMakeLists.txt`).
  Static images keep their names, so every static lookup works unchanged.
- An APNG read by a plain PNG decoder shows only its default image, which is
  frame 0: the off state of a button, the first frame of an anim. Until the
  client decodes APNG frames, every grouped button draws its off image in every
  state and every anim stands on its first frame. Old keys that are now frames
  1 and 2 no longer exist as files, so `retrieve` of them returns null.
- The client needs an APNG-aware retrieve: given the APNG name key and a frame
  index, return that frame's texture (and, for anims, the frame count and
  per-frame delay). Frames keep the source size and have no offset.
- One key resolves to exactly one file. An old key that is now a frame inside
  another APNG (e.g. `0000001D`) has no file of its own, and an APNG name never
  collides with a static image.

#### Old key lookup

To rewrite a lookup of old key `K`:

1. If `K` is the name of a line in `proguse_groups_v3.txt` (see the APNG table
   below), load `K.PNG`; it holds that line's frames. Frame 0 is the line's
   first key, which differs from the name `K` for the 5 buttons listed below.
2. Otherwise, if `K` appears in some line, it is a frame of that line's APNG;
   the frame index is its position in the line. A key can be in several lines
   (`00000300`, `00000302`); pick the line whose states match the use site.
3. Otherwise `K` is a static image, `K.PNG`, unchanged.

Buttons whose name is not their off key (the name is the smallest key, and
these have a lower down or on key):

| APNG | Frames off, on, down |
|---|---|
| `00000032.PNG` | 00000033, 00000033, 00000032 |
| `00000034.PNG` | 00000035, 00000035, 00000034 |
| `00000310.PNG` | 00000311, 00000310, 00000312 |
| `00000320.PNG` | 00000321, 00000320, 00000322 |
| `00000330.PNG` | 00000331, 00000330, 00000332 |

#### APNG table

All 112 APNGs in manifest order. Frame `f` of a button is off/on/down for
`f` = 0/1/2. "Used by" lists every client site found, including the
indirect ones described under Special cases.

| APNG | Kind | Frames (old keys) | Delay ms | Size | Used by |
|---|---|---|---:|---|---|
| `04000000.PNG` | anim | 04000000–04000003 (4) | 200 | 36×20 | `gui/controlboard/cbtitle.cpp:39` |
| `04000010.PNG` | anim | 04000010–04000019 (10) | 125 | 60×22 | `wmdaniboard.hpp:22` |
| `00000210.PNG` | anim | 00000210–0000021C (13) | 200 | 30×29 | `processrun.cpp:531` |
| `00000005.PNG` | button | 00000005, 00000006, 00000007 | — | 96×26 | `processlogin.cpp:32` |
| `00000008.PNG` | button | 00000008, 00000009, 0000000A | — | 96×26 | `processlogin.cpp:33` |
| `0000000B.PNG` | button | 0000000B, 0000000C, 0000000D | — | 96×26 | `processlogin.cpp:34` |
| `0000000E.PNG` | button | 0000000E, 0000000F, 00000010 | — | 96×24 | `processlogin.cpp:35` |
| `0000001C.PNG` | button | 0000001C, 0000001C, 0000001D | — | 32×30 | `baseframeboard.cpp:54`, `gui/auctionboard/auctionboard.cpp:367`, `gui/auctionboard/auctionregisterboard.cpp:121`, `gui/directtradeboard/directtradeboard.cpp:276`, `gui/friendchatboard/friendchatboard.cpp:109`, `gui/herostateboard/herostateboard.cpp:88`, `gui/myherostateboard/myherostateboard.cpp:88`, `gui/npcchatboard/npcchatboard.cpp:133`, `gui/purchaseboard/purchaseboard.cpp:37`, `gui/purchaseboard/purchaseboard.cpp:78`, `gui/purchaseboard/purchaseboard.cpp:162`, `gui/skillboard/skillboard.cpp:178`, `guildboard.cpp:40`, `horseboard.cpp:66`, `inventoryboard.cpp:72`, `itemlistboard.cpp:112`, `processchangepassword.cpp:188`, `processcreateaccount.cpp:147`, `queststateboard.cpp:116`, `teamstateboard.cpp:141` |
| `0000001E.PNG` | button | 0000001E, 0000001E, 0000001F | — | 24×24 | `gui/controlboard/cbleft.cpp:247` |
| `00000020.PNG` | button | 00000020, 00000020, 00000021 | — | 24×24 | `gui/controlboard/cbleft.cpp:268` |
| `00000023.PNG` | button | 00000023, 00000023, 00000024 | — | 34×34 | `gui/controlboard/cbmiddleexpand.cpp:125`, `gui/friendchatboard/friendchatboard.cpp:207` |
| `00000025.PNG` | button | 00000025, 00000025, 00000026 | — | 34×34 | `gui/controlboard/cbmiddleexpand.cpp:139` |
| `00000030.PNG` | button | 00000030, 00000030, 00000031 | — | 32×34 | `gui/controlboard/cbright.cpp:158` |
| `00000032.PNG` | button | 00000033, 00000033, 00000032 | — | 32×38 | `gui/controlboard/cbright.cpp:180` |
| `00000034.PNG` | button | 00000035, 00000035, 00000034 | — | 32×34 | `gui/controlboard/cbright.cpp:202` |
| `00000036.PNG` | button | 00000036, 00000036, 00000037 | — | 36×28 | `gui/controlboard/cbright.cpp:224` |
| `00000038.PNG` | button | 00000038, 00000038, 00000039 | — | 40×20 | `gui/controlboard/cbright.cpp:246` |
| `0000003A.PNG` | button | 0000003A, 0000003A, 0000003B | — | 36×28 | `gui/controlboard/cbright.cpp:277` |
| `0000003C.PNG` | button | 0000003C, 0000003C, 0000003D | — | 36×28 | `gui/controlboard/cbright.cpp:301` |
| `0000003E.PNG` | button | 0000003E, 0000003E, 0000003F | — | 40×20 | `gui/controlboard/cbright.cpp:323` |
| `00000040.PNG` | button | 00000040, 00000040, 00000041 | — | 36×28 | `gui/controlboard/cbright.cpp:345` |
| `00000055.PNG` | button | 00000055, 00000055, 00000056 | — | 24×18 | — |
| `00000057.PNG` | button | 00000057, 00000057, 00000058 | — | 24×18 | — |
| `00000061.PNG` | button | 00000061, 00000061, 00000062 | — | 16×16 | `gui/quickaccessboard/quickaccessboard.cpp:64` |
| `000000B1.PNG` | button | 000000B1, 000000B1, 000000B2 | — | 34×34 | `inventoryboard.cpp:97` |
| `000000B3.PNG` | button | 000000B3, 000000B3, 000000B4 | — | 34×34 | `gui/auctionboard/auctionboard.cpp:274`, `gui/auctionboard/auctionregisterboard.cpp:89`, `inventoryboard.cpp:95`, `inventoryboard.cpp:98` |
| `000000B5.PNG` | button | 000000B5, 000000B5, 000000B6 | — | 34×34 | `gui/directtradeboard/directtradeboard.cpp:238`, `inventoryboard.cpp:96` |
| `000000B7.PNG` | button | 000000B7, 000000B7, 000000B8 | — | 34×34 | `gui/auctionboard/auctionboard.cpp:302` |
| `000000C0.PNG` | button | 000000C0, 000000C0, 000000C1 | — | 50×26 | `inventoryboard.cpp:53` |
| `000000E0.PNG` | button | 000000E0, 000000E0, 000000E1 | — | 14×16 | no code reference |
| `000000E2.PNG` | button | 000000E2, 000000E2, 000000E3 | — | 14×16 | no code reference |
| `000000E4.PNG` | button | 000000E4, 000000E4, 000000E5 | — | 14×16 | no code reference |
| `000000E6.PNG` | button | 000000E6, 000000E6, 000000E7 | — | 14×16 | no code reference |
| `000000E8.PNG` | button | 000000E8, 000000E8, 000000E9 | — | 14×16 | no code reference |
| `000000EA.PNG` | button | 000000EA, 000000EA, 000000EB | — | 14×16 | no code reference |
| `000000EC.PNG` | button | 000000EC, 000000EC, 000000ED | — | 14×16 | no code reference |
| `00000110.PNG` | button | 00000110, 00000110, 00000111 | — | 59×24 | no code reference |
| `00000120.PNG` | button | 00000120, 00000120, 00000121 | — | 59×24 | no code reference |
| `00000130.PNG` | button | 00000130, 00000130, 00000131 | — | 59×24 | no code reference |
| `00000160.PNG` | button | 00000160, 00000160, 00000161 | — | 34×34 | `gui/friendchatboard/friendchatboard.cpp:339`, `teamstateboard.cpp:51` |
| `00000170.PNG` | button | 00000170, 00000170, 00000171 | — | 34×34 | `gui/friendchatboard/friendchatboard.cpp:457`, `teamstateboard.cpp:78` |
| `00000180.PNG` | button | 00000180, 00000180, 00000181 | — | 34×34 | `teamstateboard.cpp:100` |
| `00000190.PNG` | button | 00000190, 00000190, 00000191 | — | 34×34 | `gui/directtradeboard/directtradeboard.cpp:237`, `teamstateboard.cpp:122` |
| `00000200.PNG` | button | 00000200, 00000200, 00000201 | — | 23×24 | `teamstateboard.cpp:37` |
| `00000301.PNG` | button | 00000301, 00000300, 00000302 | — | 32×32 | `gui/auctionboard/auctionboard.cpp:205`, `pullmenu.cpp:54` |
| `00000300.PNG` | button | 00000300, 00000300, 00000302 | — | 32×32 | `queststateboard.cpp:101` |
| `00000310.PNG` | button | 00000311, 00000310, 00000312 | — | 32×32 | `gui/auctionboard/auctionboard.cpp:183` |
| `00000320.PNG` | button | 00000321, 00000320, 00000322 | — | 32×32 | no code reference |
| `00000330.PNG` | button | 00000331, 00000330, 00000332 | — | 32×30 | `gui/auctionboard/auctionboard.cpp:227` |
| `00000370.PNG` | button | 00000370, 00000370, 00000371 | — | 78×78 | `radioselector.cpp:47` |
| `00000510.PNG` | button | 00000510, 00000510, 00000511 | — | 34×34 | `guildboard.cpp:59` |
| `00000520.PNG` | button | 00000520, 00000520, 00000521 | — | 34×34 | `guildboard.cpp:77` |
| `00000530.PNG` | button | 00000530, 00000530, 00000531 | — | 34×34 | `guildboard.cpp:95` |
| `00000540.PNG` | button | 00000540, 00000540, 00000541 | — | 34×34 | `guildboard.cpp:113` |
| `00000550.PNG` | button | 00000550, 00000550, 00000551 | — | 34×34 | `guildboard.cpp:131` |
| `00000560.PNG` | button | 00000560, 00000560, 00000561 | — | 34×34 | `guildboard.cpp:149` |
| `00000570.PNG` | button | 00000570, 00000570, 00000571 | — | 34×34 | `guildboard.cpp:167` |
| `00000580.PNG` | button | 00000580, 00000580, 00000581 | — | 34×34 | `guildboard.cpp:185` |
| `00000590.PNG` | button | 00000590, 00000590, 00000591 | — | 34×34 | `gui/friendchatboard/friendchatboard.cpp:243` |
| `00000610.PNG` | button | 00000610, 00000610, 00000611 | — | 51×27 | no code reference |
| `00000710.PNG` | button | 00000710, 00000710, 00000711 | — | 47×26 | `horseboard.cpp:85` |
| `00000720.PNG` | button | 00000720, 00000720, 00000721 | — | 47×26 | `horseboard.cpp:103` |
| `00000730.PNG` | button | 00000730, 00000730, 00000731 | — | 47×26 | `horseboard.cpp:121` |
| `00000740.PNG` | button | 00000740, 00000740, 00000741 | — | 47×26 | `horseboard.cpp:139` |
| `00000820.PNG` | button | 00000820, 00000820, 00000821 | — | 34×34 | no code reference |
| `00000830.PNG` | button | 00000830, 00000830, 00000831 | — | 34×34 | no code reference |
| `00000840.PNG` | button | 00000840, 00000840, 00000841 | — | 34×34 | no code reference |
| `00000850.PNG` | button | 00000850, 00000850, 00000851 | — | 34×34 | `gui/auctionboard/auctionboard.cpp:334`, `gui/auctionboard/auctionregisterboard.cpp:105` |
| `00000860.PNG` | button | 00000860, 00000860, 00000861 | — | 34×34 | `gui/friendchatboard/friendchatboard.cpp:705` |
| `00000870.PNG` | button | 00000870, 00000870, 00000871 | — | 34×34 | no code reference |
| `00000880.PNG` | button | 00000880, 00000880, 00000881 | — | 34×34 | no code reference |
| `00000890.PNG` | button | 00000890, 00000890, 00000891 | — | 34×34 | no code reference |
| `000008A0.PNG` | button | 000008A0, 000008A0, 000008A1 | — | 34×34 | no code reference |
| `000008B0.PNG` | button | 000008B0, 000008B0, 000008B1 | — | 34×34 | `gui/friendchatboard/friendchatboard.cpp:225` |
| `000008C0.PNG` | button | 000008C0, 000008C0, 000008C1 | — | 34×34 | no code reference |
| `000008D0.PNG` | button | 000008D0, 000008D0, 000008D1 | — | 34×34 | no code reference |
| `000008E0.PNG` | button | 000008E0, 000008E0, 000008E1 | — | 34×34 | no code reference |
| `000008F0.PNG` | button | 000008F0, 000008F0, 000008F1 | — | 34×34 | `gui/friendchatboard/friendchatboard.cpp:188`, `gui/friendchatboard/friendchatboard.cpp:419`, `gui/friendchatboard/friendchatboard.cpp:536`, `gui/friendchatboard/friendchatboard.cpp:612` |
| `00000900.PNG` | button | 00000900, 00000900, 00000901 | — | 34×34 | `gui/friendchatboard/friendchatboard.cpp:438` |
| `00000910.PNG` | button | 00000910, 00000910, 00000911 | — | 34×34 | `gui/friendchatboard/friendchatboard.cpp:631` |
| `00001420.PNG` | button | 00001420, 00001420, 00001421 | — | 46×24 | `gui/auctionboard/auctionboard.cpp:244`, `gui/auctionboard/auctionboard.cpp:259` |
| `00002000.PNG` | button | 00002000, 00002000, 00002001 | — | 34×34 | no code reference |
| `05000020.PNG` | button | 05000020, 05000020, 05000030 | — | 36×32 | `gui/skillboard/skillboard.cpp:99 (tab 0)` |
| `05000021.PNG` | button | 05000021, 05000021, 05000031 | — | 36×32 | `gui/skillboard/skillboard.cpp:99 (tab 1)` |
| `05000022.PNG` | button | 05000022, 05000022, 05000032 | — | 36×32 | `gui/skillboard/skillboard.cpp:99 (tab 2)` |
| `05000023.PNG` | button | 05000023, 05000023, 05000033 | — | 36×32 | `gui/skillboard/skillboard.cpp:99 (tab 3)` |
| `05000024.PNG` | button | 05000024, 05000024, 05000034 | — | 36×32 | `gui/skillboard/skillboard.cpp:99 (tab 4)` |
| `05000025.PNG` | button | 05000025, 05000025, 05000035 | — | 36×32 | `gui/skillboard/skillboard.cpp:99 (tab 5)` |
| `05000026.PNG` | button | 05000026, 05000026, 05000036 | — | 36×32 | `gui/skillboard/skillboard.cpp:99 (tab 6)` |
| `05000027.PNG` | button | 05000027, 05000027, 05000037 | — | 36×32 | `gui/skillboard/skillboard.cpp:99 (tab 7)` |
| `07000001.PNG` | button | 07000001, 07000002, 07000003 | — | 79×32 | `inputstringboard.cpp:123` |
| `07000004.PNG` | button | 07000004, 07000005, 07000006 | — | 79×32 | `inputstringboard.cpp:152` |
| `07000007.PNG` | button | 07000007, 07000008, 07000009 | — | 79×32 | no code reference |
| `08000003.PNG` | button | 08000003, 08000003, 08000004 | — | 58×22 | `gui/purchaseboard/purchaseboard.cpp:57` |
| `08000005.PNG` | button | 08000005, 08000005, 08000006 | — | 42×23 | `gui/purchaseboard/purchaseboard.cpp:116`, `itemlistboard.cpp:74` |
| `08000007.PNG` | button | 08000007, 08000007, 08000008 | — | 36×23 | `gui/purchaseboard/purchaseboard.cpp:97`, `itemlistboard.cpp:55` |
| `08000009.PNG` | button | 08000009, 08000009, 0800000A | — | 36×23 | `gui/purchaseboard/purchaseboard.cpp:138`, `itemlistboard.cpp:93` |
| `0800000B.PNG` | button | 0800000B, 0800000B, 0800000C | — | 70×26 | `gui/purchaseboard/purchaseboard.cpp:181`, `processchangepassword.cpp:169`, `processcreateaccount.cpp:128` |
| `09000010.PNG` | button | 09000010, 09000010, 09000011 | — | 19×19 | `minimapboard.cpp:144` |
| `09000020.PNG` | button | 09000020, 09000020, 09000021 | — | 19×19 | `minimapboard.cpp:162` |
| `09000030.PNG` | button | 09000030, 09000030, 09000031 | — | 19×19 | `minimapboard.cpp:180` |
| `09000040.PNG` | button | 09000040, 09000040, 09000041 | — | 19×19 | `minimapboard.cpp:198` |
| `0B000000.PNG` | button | 0B000000, 0B000000, 0B000001 | — | 24×16 | `gui/controlboard/cbleft.cpp:226` |
| `0C000010.PNG` | button | 0C000010, 0C000010, 0C000011 | — | 96×26 | `processselectchar.cpp:30` |
| `0C000020.PNG` | button | 0C000020, 0C000020, 0C000021 | — | 96×26 | `processselectchar.cpp:31` |
| `0C000030.PNG` | button | 0C000030, 0C000030, 0C000031 | — | 96×24 | `processselectchar.cpp:29` |
| `0C000040.PNG` | button | 0C000040, 0C000040, 0C000041 | — | 48×26 | `processselectchar.cpp:32` |
| `0D000010.PNG` | button | 0D000010, 0D000011, 0D000012 | — | 28×28 | `processcreatechar.cpp:28` |
| `0D000020.PNG` | button | 0D000020, 0D000021, 0D000022 | — | 28×28 | `processcreatechar.cpp:29` |
| `0D000030.PNG` | button | 0D000030, 0D000031, 0D000032 | — | 40×38 | `processcreatechar.cpp:25` |
| `0D000040.PNG` | button | 0D000040, 0D000041, 0D000042 | — | 40×38 | `processcreatechar.cpp:26` |
| `0D000050.PNG` | button | 0D000050, 0D000051, 0D000052 | — | 40×38 | `processcreatechar.cpp:27` |

#### Use sites

Every `TritexButton` `.texIDList` literal and `texIDFunc` array in the client
that maps to a group (107 sites). Replace the three keys with the APNG and use
the button state as the frame index. `—` means the site has no `.off`; it maps
to the group whose off is its on image, so its off state now draws the on image
instead of nothing.

| Site | Field | off, on, down | APNG |
|---|---|---|---|
| `baseframeboard.cpp:54` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `gui/auctionboard/auctionboard.cpp:183` | `texIDList` | 00000311, 00000310, 00000312 | `00000310.PNG` |
| `gui/auctionboard/auctionboard.cpp:205` | `texIDList` | 00000301, 00000300, 00000302 | `00000301.PNG` |
| `gui/auctionboard/auctionboard.cpp:227` | `texIDList` | 00000331, 00000330, 00000332 | `00000330.PNG` |
| `gui/auctionboard/auctionboard.cpp:244` | `texIDList` | —, 00001420, 00001421 | `00001420.PNG` |
| `gui/auctionboard/auctionboard.cpp:259` | `texIDList` | —, 00001420, 00001421 | `00001420.PNG` |
| `gui/auctionboard/auctionboard.cpp:274` | `texIDList` | 000000B3, 000000B3, 000000B4 | `000000B3.PNG` |
| `gui/auctionboard/auctionboard.cpp:302` | `texIDList` | —, 000000B7, 000000B8 | `000000B7.PNG` |
| `gui/auctionboard/auctionboard.cpp:334` | `texIDList` | 00000850, 00000850, 00000851 | `00000850.PNG` |
| `gui/auctionboard/auctionboard.cpp:367` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `gui/auctionboard/auctionregisterboard.cpp:105` | `texIDList` | —, 00000850, 00000851 | `00000850.PNG` |
| `gui/auctionboard/auctionregisterboard.cpp:121` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `gui/auctionboard/auctionregisterboard.cpp:89` | `texIDList` | —, 000000B3, 000000B4 | `000000B3.PNG` |
| `gui/controlboard/cbleft.cpp:226` | `texIDList` | —, 0B000000, 0B000001 | `0B000000.PNG` |
| `gui/controlboard/cbleft.cpp:247` | `texIDList` | —, 0000001E, 0000001F | `0000001E.PNG` |
| `gui/controlboard/cbleft.cpp:268` | `texIDList` | —, 00000020, 00000021 | `00000020.PNG` |
| `gui/controlboard/cbmiddleexpand.cpp:125` | `texIDList` | —, 00000023, 00000024 | `00000023.PNG` |
| `gui/controlboard/cbmiddleexpand.cpp:139` | `texIDList` | —, 00000025, 00000026 | `00000025.PNG` |
| `gui/controlboard/cbright.cpp:158` | `texIDList` | 00000030, 00000030, 00000031 | `00000030.PNG` |
| `gui/controlboard/cbright.cpp:180` | `texIDList` | 00000033, 00000033, 00000032 | `00000032.PNG` |
| `gui/controlboard/cbright.cpp:202` | `texIDList` | 00000035, 00000035, 00000034 | `00000034.PNG` |
| `gui/controlboard/cbright.cpp:224` | `texIDList` | 00000036, 00000036, 00000037 | `00000036.PNG` |
| `gui/controlboard/cbright.cpp:246` | `texIDList` | 00000038, 00000038, 00000039 | `00000038.PNG` |
| `gui/controlboard/cbright.cpp:277` | `texIDList` | 0000003A, 0000003A, 0000003B | `0000003A.PNG` |
| `gui/controlboard/cbright.cpp:301` | `texIDList` | 0000003C, 0000003C, 0000003D | `0000003C.PNG` |
| `gui/controlboard/cbright.cpp:323` | `texIDList` | 0000003E, 0000003E, 0000003F | `0000003E.PNG` |
| `gui/controlboard/cbright.cpp:345` | `texIDList` | 00000040, 00000040, 00000041 | `00000040.PNG` |
| `gui/directtradeboard/directtradeboard.cpp:237` | `texIDFunc` | 00000190, 00000190, 00000191 | `00000190.PNG` |
| `gui/directtradeboard/directtradeboard.cpp:238` | `texIDFunc` | 000000B5, 000000B5, 000000B6 | `000000B5.PNG` |
| `gui/directtradeboard/directtradeboard.cpp:276` | `texIDList` | 0000001C, 0000001C, 0000001D | `0000001C.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:109` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:188` | `texIDList` | 000008F0, 000008F0, 000008F1 | `000008F0.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:207` | `texIDList` | 00000023, 00000023, 00000024 | `00000023.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:225` | `texIDList` | 000008B0, 000008B0, 000008B1 | `000008B0.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:243` | `texIDList` | 00000590, 00000590, 00000591 | `00000590.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:339` | `texIDList` | 00000160, 00000160, 00000161 | `00000160.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:419` | `texIDList` | 000008F0, 000008F0, 000008F1 | `000008F0.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:438` | `texIDList` | 00000900, 00000900, 00000901 | `00000900.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:457` | `texIDList` | 00000170, 00000170, 00000171 | `00000170.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:536` | `texIDList` | 000008F0, 000008F0, 000008F1 | `000008F0.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:612` | `texIDList` | 000008F0, 000008F0, 000008F1 | `000008F0.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:631` | `texIDList` | 00000910, 00000910, 00000911 | `00000910.PNG` |
| `gui/friendchatboard/friendchatboard.cpp:705` | `texIDList` | 00000860, 00000860, 00000861 | `00000860.PNG` |
| `gui/herostateboard/herostateboard.cpp:88` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `gui/myherostateboard/myherostateboard.cpp:88` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `gui/npcchatboard/npcchatboard.cpp:133` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `gui/purchaseboard/purchaseboard.cpp:116` | `texIDList` | —, 08000005, 08000006 | `08000005.PNG` |
| `gui/purchaseboard/purchaseboard.cpp:138` | `texIDList` | —, 08000009, 0800000A | `08000009.PNG` |
| `gui/purchaseboard/purchaseboard.cpp:162` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `gui/purchaseboard/purchaseboard.cpp:181` | `texIDList` | —, 0800000B, 0800000C | `0800000B.PNG` |
| `gui/purchaseboard/purchaseboard.cpp:37` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `gui/purchaseboard/purchaseboard.cpp:57` | `texIDList` | —, 08000003, 08000004 | `08000003.PNG` |
| `gui/purchaseboard/purchaseboard.cpp:78` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `gui/purchaseboard/purchaseboard.cpp:97` | `texIDList` | —, 08000007, 08000008 | `08000007.PNG` |
| `gui/quickaccessboard/quickaccessboard.cpp:64` | `texIDList` | 00000061, 00000061, 00000062 | `00000061.PNG` |
| `gui/skillboard/skillboard.cpp:178` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `guildboard.cpp:113` | `texIDList` | —, 00000540, 00000541 | `00000540.PNG` |
| `guildboard.cpp:131` | `texIDList` | —, 00000550, 00000551 | `00000550.PNG` |
| `guildboard.cpp:149` | `texIDList` | —, 00000560, 00000561 | `00000560.PNG` |
| `guildboard.cpp:167` | `texIDList` | —, 00000570, 00000571 | `00000570.PNG` |
| `guildboard.cpp:185` | `texIDList` | —, 00000580, 00000581 | `00000580.PNG` |
| `guildboard.cpp:40` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `guildboard.cpp:59` | `texIDList` | —, 00000510, 00000511 | `00000510.PNG` |
| `guildboard.cpp:77` | `texIDList` | —, 00000520, 00000521 | `00000520.PNG` |
| `guildboard.cpp:95` | `texIDList` | —, 00000530, 00000531 | `00000530.PNG` |
| `horseboard.cpp:103` | `texIDList` | —, 00000720, 00000721 | `00000720.PNG` |
| `horseboard.cpp:121` | `texIDList` | —, 00000730, 00000731 | `00000730.PNG` |
| `horseboard.cpp:139` | `texIDList` | —, 00000740, 00000741 | `00000740.PNG` |
| `horseboard.cpp:66` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `horseboard.cpp:85` | `texIDList` | —, 00000710, 00000711 | `00000710.PNG` |
| `inputstringboard.cpp:123` | `texIDList` | 07000001, 07000002, 07000003 | `07000001.PNG` |
| `inputstringboard.cpp:152` | `texIDList` | 07000004, 07000005, 07000006 | `07000004.PNG` |
| `inventoryboard.cpp:53` | `texIDList` | —, 000000C0, 000000C1 | `000000C0.PNG` |
| `inventoryboard.cpp:72` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `inventoryboard.cpp:95` | `texIDFunc` | 000000B3, 000000B3, 000000B4 | `000000B3.PNG` |
| `inventoryboard.cpp:96` | `texIDFunc` | 000000B5, 000000B5, 000000B6 | `000000B5.PNG` |
| `inventoryboard.cpp:97` | `texIDFunc` | 000000B1, 000000B1, 000000B2 | `000000B1.PNG` |
| `inventoryboard.cpp:98` | `texIDFunc` | 000000B3, 000000B3, 000000B4 | `000000B3.PNG` |
| `itemlistboard.cpp:112` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `itemlistboard.cpp:55` | `texIDList` | —, 08000007, 08000008 | `08000007.PNG` |
| `itemlistboard.cpp:74` | `texIDList` | —, 08000005, 08000006 | `08000005.PNG` |
| `itemlistboard.cpp:93` | `texIDList` | —, 08000009, 0800000A | `08000009.PNG` |
| `processchangepassword.cpp:169` | `texIDList` | —, 0800000B, 0800000C | `0800000B.PNG` |
| `processchangepassword.cpp:188` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `processcreateaccount.cpp:128` | `texIDList` | —, 0800000B, 0800000C | `0800000B.PNG` |
| `processcreateaccount.cpp:147` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `processcreatechar.cpp:25` | `texIDList` | 0D000030, 0D000031, 0D000032 | `0D000030.PNG` |
| `processcreatechar.cpp:26` | `texIDList` | 0D000040, 0D000041, 0D000042 | `0D000040.PNG` |
| `processcreatechar.cpp:27` | `texIDList` | 0D000050, 0D000051, 0D000052 | `0D000050.PNG` |
| `processcreatechar.cpp:28` | `texIDList` | 0D000010, 0D000011, 0D000012 | `0D000010.PNG` |
| `processcreatechar.cpp:29` | `texIDList` | 0D000020, 0D000021, 0D000022 | `0D000020.PNG` |
| `processlogin.cpp:32` | `texIDList` | 00000005, 00000006, 00000007 | `00000005.PNG` |
| `processlogin.cpp:33` | `texIDList` | 00000008, 00000009, 0000000A | `00000008.PNG` |
| `processlogin.cpp:34` | `texIDList` | 0000000B, 0000000C, 0000000D | `0000000B.PNG` |
| `processlogin.cpp:35` | `texIDList` | 0000000E, 0000000F, 00000010 | `0000000E.PNG` |
| `processselectchar.cpp:29` | `texIDList` | 0C000030, 0C000030, 0C000031 | `0C000030.PNG` |
| `processselectchar.cpp:30` | `texIDList` | 0C000010, 0C000010, 0C000011 | `0C000010.PNG` |
| `processselectchar.cpp:31` | `texIDList` | 0C000020, 0C000020, 0C000021 | `0C000020.PNG` |
| `processselectchar.cpp:32` | `texIDList` | 0C000040, 0C000040, 0C000041 | `0C000040.PNG` |
| `queststateboard.cpp:101` | `texIDList` | 00000300, 00000300, 00000302 | `00000300.PNG` |
| `queststateboard.cpp:116` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `teamstateboard.cpp:100` | `texIDList` | 00000180, 00000180, 00000181 | `00000180.PNG` |
| `teamstateboard.cpp:122` | `texIDList` | —, 00000190, 00000191 | `00000190.PNG` |
| `teamstateboard.cpp:141` | `texIDList` | —, 0000001C, 0000001D | `0000001C.PNG` |
| `teamstateboard.cpp:37` | `texIDList` | —, 00000200, 00000201 | `00000200.PNG` |
| `teamstateboard.cpp:51` | `texIDList` | —, 00000160, 00000161 | `00000160.PNG` |
| `teamstateboard.cpp:78` | `texIDList` | 00000170, 00000170, 00000171 | `00000170.PNG` |

#### Special cases

- `TritexButton` (`tritexbutton.cpp:112–115`) retrieves `texIDList[state]` as
  three separate keys. It needs a mode that takes one APNG name and draws
  frame `state`. It must keep a single-key mode for static images used for all
  three states: `gui/skillboard/magiciconbutton.cpp:21` sets off = on = down =
  the magic icon (`SkillBoard::getMagicIconGfx(m_magicID)->magicIcon`, static
  `05001xxx` keys).
- Skill board tabs, `gui/skillboard/skillboard.cpp:99–102`: `.on = 0X05000020 + i`,
  `.down = 0X05000030 + i`, no `.off`, for `i` = 0–7. They map to
  `0500002i.PNG` (frames `0500002i, 0500002i, 0500003i`). The tab page images
  `05000010 + i` (`skillboard.cpp:52`) are static.
- Minimap toggles, `minimapboard.cpp:142–199`: each `texIDFunc` returns the same
  key in all three states, chosen by a flag: `09000010`/`09000011` by
  `m_alphaOn`, `09000020`/`09000021` by `m_extended`, `09000030`/`09000031` by
  `m_autoCenter`, `09000040`/`09000041` by `m_autoCenter` (the config button
  also reads `m_autoCenter`). The groups are `0900x0 0900x0 0900x1`, so in
  `0900x0.PNG` frames 0 and 1 are image `x0` and frame 2 is image `x1`. To keep
  today's look, draw frame 2 when the flag is set and frame 0 otherwise; to use
  it as a normal button, draw frame `state`.
- `pullmenu.cpp:54–56` draws three `ImageBoard`s from `00000301`, `00000300`,
  `00000302` (off, on, down): use frames 0/1/2 of `00000301.PNG`.
- `radioselector.cpp:47–49` draws `00000370` (off, white), `00000370` (on, red
  tint) and `00000371` (down): use frames 0/1/2 of `00000370.PNG`, keeping the
  tints.
- `00000300` and `00000302` are in two APNGs: `00000301.PNG` for off `301`
  (`auctionboard.cpp:205`, `pullmenu.cpp:54`) and `00000300.PNG` for off `300`
  (`queststateboard.cpp:101`).
- Team flag cursor, `processrun.cpp:531–532`:
  `retrieve(0X00000210 + m_teamFlag.seqFrame() % 13)` becomes frame
  `seqFrame() % 13` of `00000210.PNG`. The 200 ms delay matches
  `m_teamFlag(5)`.
- `TexAniBoard` (`texaniboard.cpp`) draws `m_startTexID + frame` and, with fade,
  blends the current and next frames by alpha. It needs per-frame access to the
  APNG, not just playback. Users:
  - `WMDAniBoard` (`wmdaniboard.hpp:22`): `0X04000010`, 10 frames, 8 fps, used by
    `inventoryboard.hpp:49` and `gui/auctionboard/auctionboard.hpp:25`. Matches the
    125 ms delay.
  - `gui/controlboard/cbtitle.cpp:39` `m_arcAni`: `0X04000000`, 4 frames, with
    fade and loop, at **1 fps** in the code. The manifest uses 200 ms (5 fps).
    Decide which is right; if the client keeps its own fps, the APNG delay is
    unused.
- `gui/controlboard/cbmiddle.cpp:132` and `cbmiddleexpand.cpp:105`: `.on =
  0X00000028`, `.down = 0X00000029`. `00000029` does not exist, so `00000028` is
  static and is not in any group; these sites keep loading it as a static
  image (the down state already draws nothing).
- `AlphaOnButton` (`alphaonbutton.cpp`, used by `gui/controlboard/cbright.cpp:88,
  112, 142`) loads `.downTexID` `00000042`, `00000043`, `00000044`. They are three
  different static icons, not in any group, and need no change.

#### Static keys referenced by code

These are not in any group and keep their name and content, so the code needs no
change. Computed ones are listed with their range:

- `acbutton` `00000046`–`00000049`; HP bar `00000014`–`00000017`;
  `cbleft` `00000018`, `00000019`; `cbtitle` `00000022`; `cbmiddleexpand`
  `00000027`; `cbmiddle`/`cbmiddleexpand` `00000028`.
- `texslider` `00000080`, `00000081`, `00000088`, `00000089`, `0000008A`.
- `baseframeboard.hpp:42` frame `00000450`; `modalstringboard.cpp:44`
  `07000000`; `queststateboard.cpp:44` `00000350`; `auctionboard.cpp:34`
  `00001400`; skill board background `05000000`; inventory `000000B0`.
- `ascendstr.cpp:60–68`: `0X03000000 | (type << 4) | digit`, plus `0A`/`0B` for
  the sign.
- `purchaseboard.cpp:28`: `0X08000000 + extendedBoardGfxID()`, 0–2.
- My hero state board `06000010 + i`, `06000020 + i`; skill pages `05000010 + i`;
  magic icons `05001xxx`; minimap frame `19xxxxxx`; buff icons `18xxxxxx`.

#### Unreferenced groups

These 26 APNGs have no client reference yet. They were grouped by size and
consecutive keys (or, for the quest marks and `00000055`/`00000057`, by the user):
`00000055`, `00000057`, `000000E0`, `000000E2`, `000000E4`, `000000E6`, `000000E8`, `000000EA`, `000000EC`, `00000110`, `00000120`, `00000130`, `00000320`, `00000610`, `00000820`, `00000830`, `00000840`, `00000870`, `00000880`, `00000890`, `000008A0`, `000008C0`, `000008D0`, `000008E0`, `00002000`, `07000007`.
`000000E0`–`000000EC` are the quest `!`/`?` marks, with off = on.

### emoji

```text
old  SS UUUU 00 CC PP WWWW HHHH hhhh .PNG     sprite sheet, CC frames of WWWW x HHHH
new  "%011X.PNG" % ((set << 16 | subset) << 12 | (frameH1 & 0xFFF))
     = 00 SS UUUU hhh .PNG

old cell i, i < CC   ->  new APNG frame i, delay 1/PP s
old cell i, i >= CC  ->  dropped
```

```python
# inverse
value   = int(stem, 16)
frameH1 = (value & 0xFFF) - 0x1000 if value & 0x800 else value & 0xFFF
key     = value >> 12            # 0x00SSUUUU
set     = key >> 16
subset  = key & 0xFFFF
```

Old `hhhh` is 16 bits, the new field is 12-bit two's complement. Every current
`frameH1` is between 17 and 100, so it fits; a value outside -2048 to 2047 is
an error.

Examples:

- `01002A00120A006400640064.PNG` (set 1, subset 0x2A, 18 frames, 10 fps,
  100×100, frameH1 100) becomes `0001002A064.PNG`, 18 frames of 100×100,
  1/10 s each.
- `00006C0005050035001B001B.PNG` (set 0, subset 0x6C, 5 frames, 5 fps,
  53×27, frameH1 27) becomes `0000006C01B.PNG`, 5 frames of 53×27, 1/5 s each.

### emoji client changes

Done in commit `4303a189`. Kept as the record of what changed. File:line
references are to `client/src` at commit `fa7d0ef9`, before the change.

#### Loader

- `g_emojiDB` is an `EmojiDB` (`emojidb.hpp/.cpp`), created in `main.cpp:80` and
  loaded from `emoji/emoji.zsdb` in `initview.cpp:65`. The zsdb is still packed
  by `tools/zsdbmaker` from the `emoji` directory.
- Key: today `EmojiDB::u32Key(set, subset)` is `set << 24 | subset << 8`, the
  low byte being the frame index, and `loadResource` looks up the 8-hex prefix
  `SSUUUU00`. The new file key is `0x00SSUUUU` = `set << 16 | subset`, so the
  8-hex lookup prefix becomes `00SSUUUU`, with no frame index in it.
- `loadResource` checks `strlen(fileName) >= 22` and parses `CC`, `PP`, `WWWW`,
  `HHHH` and `hhhh` from fixed name positions. The new name has 11 hex digits:
  parse only `frameH1` from chars 8–10, as 12-bit two's complement, and get
  the rest from the APNG.
- `EmojiElement` today is `{frameW, frameH, frameH1, fps, frameCount, texture}`
  with one sheet texture. It needs:
  - one texture per APNG frame (or an atlas the client builds itself),
  - the frame count from `acTL`,
  - each frame's delay from its `fcTL`, `delay_num / delay_den` s, where
    `delay_den == 0` means 100 (APNG spec),
  - frame width and height from the APNG,
  - `frameH1` from the name.
  `fps` goes away.
- `retrieve(key, srcX, srcY, srcW, srcH, frameH1, fps, frameCount)` computes the
  source rectangle of frame `key & 0xFF` in the sheet grid. With per-frame
  textures, the frame index selects the texture and the source rectangle is
  the whole frame, `(0, 0, W, H)`. The `fps` output must be replaced by
  per-frame delay access.
- An APNG read by a plain PNG decoder shows only frame 0.
- Keep the missing-emoji fallback: `retrieve` never returns `nullptr`. For a
  key not in the database it returns `client/src/emojidef.png` (embedded with
  `#embed`, one frame, no animation, `frameH1` = 80% of its height), created on
  first use on the main thread. The token keeps the requested key.

#### XML layout

- `<emoji id="N">` (`xmlparagraphleaf.cpp:50–58`) gives a 24-bit `N` =
  `set << 16 | subset`. `XMLTypeset::buildEmojiToken` (`xmltypeset.cpp:650–676`)
  rejects `N >= 2^24`, then shifts it left by 8 to make the old key. With the
  new key, `N` is the file key itself, no shift.
- `buildEmojiToken` fills `token.emoji.{key, fps, frameCount}` and the box size
  `(frameW, frameH)`, `h1 = frameH1`, `h2 = frameH - frameH1`. `fps` must be
  replaced by per-frame delays (`TOKEN::emoji` in `token.hpp:66–74`).
- Drawing, `xmltypeset.cpp:1412–1427`, computes
  `(key & 0xFFFFFF00) + frame % frameCount` and draws the sheet sub-rectangle.
  It becomes: draw frame `frame % frameCount` of the APNG, the whole frame.
- Animation, `XMLTypeset::update` (`xmltypeset.cpp:1443–1457`), advances
  `frame` by `elapsed / (1000 / fps)` and keeps the remainder in `tick`. It must
  instead subtract the current frame's delay from the accumulated time, move to
  the next frame, and repeat while the time left is at least the new current
  frame's delay. `tick` and `frame` are `uint8_t` today: a delay longer than
  255 ms doesn't fit in `tick`, and `frame` must stay below the frame count
  (at most 29 now).

## Sequence grouping

### monster, npc, hero, hair, helmet, weapon

Frames are grouped by `key >> 5`, which removes the 5-bit frame index. Frames in
a group must be numbered contiguously from 0. Any gap fails the run.

### magic

Magic frames in a WIL file are not grouped by key bits, so sequences come from
three sources, in this order:

1. **`magicrecord.inc` entries.** For every `MagicGfxEntry` with a `gfxID` and
   every direction `d` in `[0, gfxDirType)`:
   - frames start at `gfxID + d * gfxIDCount`,
   - the APNG keeps every contiguous frame present on disk, up to
     `gfxIDCount` frames,
   - the run fails if fewer than `frameCount` frames exist,
   - the APNG is named by `(file, image index of gfxID, d)`.

   The APNG stores the whole sequence. `frameCount` in `magicrecord.inc`
   selects how many frames from the start the magic plays, so one APNG can
   serve several entries, for example the first 10 frames of a 15-frame
   sequence.

   Entries that share a sequence identity keep the longest one. The run fails
   if the shorter one is not a prefix of the longer one, or if their heads
   differ.

   Parsing rules:
   - `//` comments are stripped.
   - Only top-level fields of an entry are read, so nested structs (such as
     `.seff.ref`) don't interfere.
   - `gfxIDCount` defaults to `frameCount`, `gfxDirType` defaults to 1.
   - `targetOffList`, if present, must have exactly `gfxDirType` entries.

   `MAGIC_RECORD_OVERRIDES` `{gfxID: frameCount}` fixes entries whose
   frames are really several animations, until `magicrecord.inc` is fixed in
   the client change: every entry with that gfxID keeps exactly `frameCount`
   frames per direction. It is an error if no entry has that gfxID.
   - `0x0200012C: 3`: 阴阳法环 挨打 plays MagicEx2 images 300–305 as one
     6-frame animation, but 300–302 are the sustain loop and 303–305 are
     the hit. The hit is the hardcoded sequence `0x0200012F` below. See
     [magicrecord.inc](#magicrecordinc) for the record fix.

   `MAGIC_RECORD_SKIPPED` lists gfxIDs of entries to ignore, also until
   `magicrecord.inc` is fixed. It is an error if no entry has that gfxID.
   - `0x0200012A`: 阴阳法环 运行 loops image 298, the last spawn frame,
     instead of the sustain loop 300–302. Without skipping it, image 298
     would also be a 1-frame APNG `020012A0...`; now it is only frame 8 of
     the spawn `02001220...`.
2. **Hardcoded sequences**, `MAGIC_HARDCODED_SEQUENCES` in the script, for
   sequences no `magicrecord.inc` entry describes. Each is `(gfxID,
   frameCount, gfxDirType, gfxIDCount)`, read like an entry, but keeps
   exactly `frameCount` frames per direction, and all of them must be on
   disk, except slots listed in `MAGIC_PADDED_KEYS`, which must be missing
   and become blank frames, and directions listed in
   `MAGIC_OMITTED_DIRECTIONS`, which must have no frames and get no APNG:
   - `0x0F0000DC`, `0x0F000104`, `0x0F000105`: single images used by
     `client/src/fixedlocmagic.cpp`.
   - `0x00000366`, 20 frames, and `0x0000037A`, 10 frames: Magic images
     870–889 and 890–899, two animations in one 30-image run.
   - `0x00000956` and `0x0000096A`, 20 frames each: the teleport effect,
     someone arriving and someone leaving. The 40 images are one contiguous
     run, so without these entries they would become one 40-frame APNG.
   - `0x00000D8E`, 8 directions of 3 frames, 10 apart, and `0x00000DE8`, 6
     frames. Without them, images 3470–3565 would become one 96-frame APNG.
   - `0x010003B6`, 10 frames, and `0x010003C0`, 6 frames: MagicEx (file 1)
     images 950–959 and 960–965, two animations in one 16-image run.
   - `0x0100076C`, 20 frames, and `0x01000780`, 14 frames: MagicEx images
     1900–1919 and 1920–1933, two animations in one 34-image run.
   - 39 directional magics, 8 directions 10 images apart, whose remaining
     slots after each direction are empty, for example `0x01000442`, MagicEx
     images 1090–1165, 6 frames per direction. Without these entries each
     direction would be a separate APNG with its own image index. Runs of 16
     or 24 such directions, e.g. MagicEx 1270–1422 or MonMagicEx3 190–420,
     are two or three magics of 8 directions each.
   - `0x0600005A`, MonMagicEx3 images 90–163, 8 directions of 4 frames.
     Direction 0 (90–93) directly follows the unrelated 10-frame run at
     80–89, so without this entry 80–93 would be one 14-frame APNG and
     100–163 7 separate APNGs.
   - `0x05000438`, MonMagicEx2 images 1080–1235, one magic of 16
     directions, 6 frames each, 10 apart: APNGs `(5, 0x438, 0)` to
     `(5, 0x438, 15)`, direction digits `0`–`F`.
   - `0x07000244`, MonMagicEx4 images 580–655, 8 directions of 6 frames.
     Direction 0 has only 581–584 on disk, so 580 and 585 are in
     `MAGIC_PADDED_KEYS`, and its APNG `(7, 0x244, 0)` has blank frames 0
     and 5. The 4-frame magic at 500–573 just before it traces the same 8
     positions, so they are two 8-direction magics, not one with 16.
   - `0x070002E4`, MonMagicEx4 images 740–811, 8 directions of 2 frames
     (the same 32×32 sprite). Directions 0, 2 and 6 (740–741, 760–761,
     800–801) are missing, they are in `MAGIC_OMITTED_DIRECTIONS`, so only
     `(7, 0x2E4, 1)`, `3`, `4`, `5` and `7` exist. The client still looks
     up the APNG for every direction; for directions 0, 2 and 6 the lookup
     returns null and the client skips drawing this gfx. Without this entry
     each existing direction would be a separate APNG.
   - `0x07000334`, MonMagicEx4 images 820–899, 8 directions of 10 frames,
     and `0x07000384`, images 900–975, 8 directions of 6 frames. All 80
     images of 820–899 and 900–905 are one contiguous run, so without these
     entries 820–905 would be one 86-frame APNG and 910–975 7 separate APNGs.
   - `0x070005A0`, MonMagicEx4 images 1440–1519, and `0x070006D6`, images
     1750–1829, each 8 directions of 10 frames, one contiguous 80-image run
     that would otherwise be one 80-frame APNG.
   - `0x0400028A`, `0x04000816` and `0x0400092E`, MonMagicEx images 650–729,
     2070–2149 and 2350–2429, each 8 directions of 10 frames, also one
     contiguous 80-image run each.
   - `0x0400065E`, MonMagicEx images 1630–1789, 8 directions 20 images
     apart, 19 frames each. Frame 18 is a 4×2 placeholder, which becomes a
     blank frame, except in direction 1, where image 1668 is a small 4×16
     piece. The 20th image of every slot is a placeholder too, left out
     and dropped as a blank run. Without this entry, 1630–1789 would be one
     160-frame APNG.
   - `0x05000230`, MonMagicEx2 images 560–638, 8 directions of 9 frames.
     Frame 0 of direction 0 (560) is missing, so 560 is in
     `MAGIC_PADDED_KEYS`. Frame 0 of directions 1 and 7 (570, 630) is a 4×2
     placeholder with offset (-9703, -9775), which becomes a blank frame.
     Without this entry, 561–568, 570–578, … 630–638 would be 8 separate
     APNGs.
   - `0x0200012F`, 3 frames: 阴阳法环 挨打, MagicEx2 images 303–305, split
     from `0x0200012C` by `MAGIC_RECORD_OVERRIDES`.
3. **Unreferenced frames.** Every remaining frame is still converted.
   `magicwil2png` skips empty WIL slots, so each run of contiguous image
   indices becomes one APNG, named by its first frame with direction 0 and
   head (0, 0). Long runs are kept whole and not split. A run whose frames
   are all fully transparent is dropped: 16 runs, 74 frames, 8 around
   `0xD8E` (66 frames) and the blank last image of each of the 8 direction
   slots of `0x0400065E`.

Frames in the weapon 44 effect range (`0x0700076C` to `0x070016B7`) are
skipped, because they go to `texture/weapon`.

## Canvas, offset and placeholders

For each sequence:

1. Every frame is placed in one shared coordinate space by its dx/dy, so each
   frame's anchor lands on the same point.
2. The canvas is the union of the visible (alpha > 0) bounding boxes of all
   frames. Margins that are transparent in every frame are trimmed.
3. The sequence's offset is the canvas's top-left corner relative to the
   anchor, so the anchor is at `(-x, -y)` relative to the APNG's top-left
   pixel.
4. The run fails if the offset doesn't fit in 12 bits.

**Placeholder frames** are frames that are fully transparent, or that have
abs(dx) or abs(dy) above 2047. For example, hair DIE frame 5 is a 4×2
transparent image at (+9703, +9775), because the body hides the head in that
frame. Placeholder frames:

- stay in the APNG as blank frames, so frame indices don't shift,
- are ignored when computing the canvas, so their bogus offsets don't create a
  huge canvas.

If every frame in a sequence is a placeholder, the APNG is 1×1, fully
transparent, with offset (0, 0).

Pixel handling:

- Alpha is kept exactly, including partial alpha.
- RGB is zeroed where alpha is 0, for stable compression and comparison.
- Identical consecutive frames are kept as separate frames.

## APNG encoding

Pillow's own APNG writer merges identical consecutive frames, which changes the
frame count. So the converter builds the chunks itself:

- `IHDR`, then `acTL` with the frame count and loop count 0 (loop forever).
- One `fcTL` per frame:
  - full canvas size at (0, 0),
  - delay `--delay`/1000 s, the group's DELAY/1000 s for proguse `anim`
    groups, or exactly 1/fps s (`delay_num` 1, `delay_den` fps) for emoji;
    every frame has its own delay field,
  - dispose op `NONE`,
  - blend op `SOURCE`.
- The first frame's image data is in `IDAT`, so it is also the default image
  for non-APNG readers. Later frames use `fdAT`.
- Each frame is compressed by Pillow with `optimize=True`.

`--apngopt` then optionally re-optimizes each file in place.

## Verification

Unless `--no-verify` is given, every output file is checked right after it is
written:

- The filename decodes back to the same identity and offset.
- The APNG has exactly one frame per source frame.
- Each non-placeholder frame, cropped at the source frame's position, is
  pixel-identical to the source PNG, after zeroing RGB where alpha is 0.
- Each placeholder frame, and each selectchar padding frame, is fully
  transparent.

For equip, verification checks the filename round-trip and that the copy is
byte-identical to the source. proguse static images are checked the same way.
Each proguse APNG must have one frame per listed state or animation frame,
each equal to its full source image, after zeroing RGB where alpha is 0.

Each emoji APNG must decode back to the same set, subset and `frameH1`, have
exactly `CC` frames, each equal to its sheet cell after zeroing RGB where
alpha is 0, and have `delay_num/delay_den = 1/PP` in every `fcTL`.

Every mirror-copied file is compared byte-for-byte with its source.

## Expected results

A full run over the current `mir2x_res`:

| Type | Source frames | APNGs | APNG frames |
|---|---:|---:|---:|
| monster | 82,820 | 14,446 | 82,820 |
| npc | 4,668 | 749 | 4,668 |
| hero | 57,408 | 10,816 | 57,408 |
| hair | 11,040 | 2,080 | 11,040 |
| helmet | 26,496 | 4,992 | 26,496 |
| weapon | 162,096 | 29,952 | 162,096 |
| magic | 6,093 | 990 | 6,027 |
| equip | 351 | 351 (copies) | 351 |
| selectchar | 952 | 74 | 988 |
| proguse | 1,652 | 112 + 1,393 copies | 354 + 1,393 |
| emoji | 186 sheets | 186 | 2,013 |

- weapon includes 384 effect APNGs (2,160 frames) from `texture/magic`.
- magic has 145 `magicrecord.inc` entries, 934 referenced sequences (including
  the hardcoded sequences) and 56 unreferenced runs. 116 sequences have a
  non-zero head. 5 source frames are in two APNGs each. 16 fully transparent
  unreferenced runs (74 frames) are dropped, 3 blank frames are padded.
- selectchar has 36 blank padding frames in its overlay APNGs.
- proguse has 3 `anim` APNGs (27 frames) and 109 button APNGs (327 frames),
  using 259 source images; `00000300` and `00000302` are in two APNGs each.
- emoji: 120 in set 0 (5 fps) and 66 in set 1 (65 at 10 fps, one at 6 fps).
  5 spare cells are dropped.
- The mirror copy holds every other file. With `--types emoji,proguse`, it
  copies 488,743 files (all other texture types, `texture/map`,
  `texture/item`, `font`, `map`, `sound`, `LICENSE`, `README.md`) in about
  25 s.

With `--jobs 8`, a full verified run takes about two minutes.

## Tests

```sh
cd tools/gfxres2apng/src
python3 -m unittest test_gfxres2apng
```

The tests use small synthetic frames and cover:

- filename layout round-trips and invalid names,
- the skipped weapon effect range in magic,
- weapon effect grouping,
- `magicrecord.inc` parsing, longest-prefix merging, missing frames and
  unreferenced runs,
- directional hardcoded magic sequences, padded missing slots, omitted
  missing directions and dropping fully transparent runs,
- `magicrecord.inc` entry overrides and skipped entries, and ones matching no
  entry,
- the skipped weapon effect range in magic,
- equip byte-for-byte copy and its new name,
- selectchar overlay padding and its missing-body / too-long errors,
- proguse group parsing, `name=`, shared keys, per-group delay, static copies
  and their extension normalization,
- emoji sheet cutting, dropped spare cells, exact 1/fps per-frame delays, the
  new name with a negative `frameH1`, and invalid old names,
- the mirror tree: rejecting a non-empty output (except `.git`), skipping
  `<res-path>/.git`, converting the selected types, and copying everything
  else, including unselected types, byte-for-byte, and copying nothing with
  `--skip-unconverted`,
- frame alignment, trimming, partial alpha and repeated frames,
- placeholder frames, and magic heads in the output name.

## Adding a texture type

1. Add the type name to `TEXTURE_TYPES` (types under `texture/`), or to
   `CONVERT_TYPES` and `TYPE_DIRS` for a type in another directory, as emoji
   does.
2. Add its filename encoder/decoder to `NAME_CODECS`. Keep the offset in bits
   0–23 and document the layout in this README and in the script's docstring.
3. Return its sequences from `collect_sequences()` as
   `{ident: [(path, dx, dy), ...]}` in frame order, where `ident` is what the
   encoder takes. If the key layout matches the `key >> 5` grouping, add a
   `KEY_LIMITS` entry instead.
4. Add tests to `test_gfxres2apng.py`, run a full conversion with verification,
   and update [Texture types](#texture-types) and
   [Expected results](#expected-results).
