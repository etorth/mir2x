import shutil
import sys
import tempfile
import unittest
from pathlib import Path
from unittest import mock

from PIL import Image

import gfxres2apng as g

MAGIC_RECORD = """
{   .name = u8"test",
    .gfxList
    {
        {
            .stage = u8"run",
            .gfxID = 0X03000100, // comment .gfxDirType = 9
            .frameCount = 2,
            .gfxIDCount = 10,
            .gfxDirType = 2,
            .targetOffList
            {
                { 31, -56},
                {-14,  -8},
            },
            .seff
            {
                .ref
                {
                    .name = u8"other",
                },
            },
        },

        {
            .gfxID = 0X03000100,
            .frameCount = 1,
            .gfxIDCount = 10,
            .gfxDirType = 2,
            .targetOffList
            {
                { 31, -56},
                {-14,  -8},
            },
        },
    },
},
"""


def save_frame(path, key, dx, dy, size=(5, 5), pixel=None):
    name = f"{key:08X}{int(dx > 0)}{int(dy > 0)}{abs(dx):04X}{abs(dy):04X}.PNG"
    img = Image.new("RGBA", size, (255, 255, 255, 0))
    if pixel:
        img.putpixel(*pixel)
    img.save(path / name)


def touch_frame(path, key):
    Path(path, f"{key:08X}1000010020.PNG").touch()


class TmpDirTest(unittest.TestCase):
    def setUp(self):
        tmp = tempfile.TemporaryDirectory()
        self.addCleanup(tmp.cleanup)
        self.source = Path(tmp.name) / "input"
        self.output = Path(tmp.name) / "output"
        self.source.mkdir()
        self.output.mkdir()


class NameTest(unittest.TestCase):
    def test_creature_identity_fields(self):
        key = (1 << 23) | (90 << 12) | (2 << 8) | (3 << 5)
        name = g.encode_creature_name(key >> 5, -296, 40)
        self.assertEqual(g.decode_creature_name(name), {"dx": -296, "dy": 40, "direction": 3, "motion": 2, "layer": 0, "shadow": 1, "look": 90})

    def test_human_identity_fields(self):
        key = (173 << 14) | (1 << 22) | (1 << 23) | (1 << 24) | (45 << 8) | (7 << 5)
        name = g.encode_human_name(key >> 5, -2048, 2047)
        self.assertEqual(g.decode_human_name(name), {
            "dx": -2048, "dy": 2047, "direction": 7, "motion": 45,
            "layer": 1, "shadow": 1, "gender": 1, "id": 173,
        })
        self.assertEqual(int(name[:-4], 16) >> 44, 0)

    def test_magic_identity_fields(self):
        name = g.encode_magic_name((255, 0xFFFFF, 15, (31, -56)), -296, 40, (-2048, 2047))
        self.assertEqual(len(name), 30)
        self.assertTrue(name.startswith("FFFFFFFF"))
        self.assertEqual(g.decode_magic_name(name), {
            "dx": -296, "dy": 40, "head": (31, -56), "tail": (-2048, 2047),
            "direction": 15, "image": 0xFFFFF, "file": 255,
        })
        self.assertTrue(g.encode_magic_name((3, 0x100, 1, (0, 0)), 0, 0).startswith("03001001000000000000"))
        for ident in [(256, 0, 0, (0, 0)), (0, 1 << 20, 0, (0, 0)), (0, 0, 16, (0, 0))]:
            with self.assertRaises(ValueError):
                g.encode_magic_name(ident, 0, 0)

    def test_invalid_names(self):
        for name in ["000000002000000000.PNG", "00000000100000000.PNG"]:
            with self.assertRaises(ValueError):
                g.parse_old_name(name)
        with self.assertRaises(ValueError):
            g.encode_human_name(0, 2048, 0)


class CollectTest(TmpDirTest):
    def test_key_limit_and_contiguous_frames(self):
        touch_frame(self.source, 0x00800000)
        g.collect_frame_groups(self.source, g.KEY_LIMITS["weapon"])
        with self.assertRaisesRegex(ValueError, "out of range"):
            g.collect_frame_groups(self.source, g.KEY_LIMITS["hair"])

        touch_frame(self.source, 0x00000001)
        with self.assertRaisesRegex(RuntimeError, "not contiguous from 0"):
            g.collect_frame_groups(self.source, g.KEY_LIMITS["weapon"])

    def test_weapon_effect_sequences(self):
        # male motion 20 dir 3 frames 0-1, female motion 0 dir 0 frame 0, plus frames outside the effect range
        for key in [0x07000000 + 1900 + 20 * 80 + 3 * 10, 0x07000000 + 1900 + 20 * 80 + 3 * 10 + 1, 0x07000000 + 3900, 0x0700076B, 0x060007D0]:
            touch_frame(self.source, key)

        seqKey = (1 << 19) | (1 << 17) | (44 << 9) | (20 << 3) | 3
        sequences = g.collect_weapon_effect_sequences(self.source)
        self.assertEqual(sorted(sequences), sorted([seqKey, (1 << 19) | (0 << 17) | (44 << 9)]))
        self.assertEqual([Path(f).name[:8] for f, _, _ in sequences[seqKey]], ["07000DCA", "07000DCB"])
        self.assertEqual(g.decode_human_name(g.encode_human_name(seqKey, 1, -2)), {
            "dx": 1, "dy": -2, "direction": 3, "motion": 20, "layer": 1, "shadow": 0, "gender": 1, "id": 44,
        })

        touch_frame(self.source, 0x07000000 + 1900 + 5)
        with self.assertRaisesRegex(RuntimeError, "not contiguous from 0"):
            g.collect_weapon_effect_sequences(self.source)

    def test_magic_record_sequences_and_unreferenced_runs(self):
        entries = g.parse_magic_record(MAGIC_RECORD)
        self.assertEqual([(e["gfxID"], e["frameCount"], e["stride"], e["dirType"], e["heads"]) for e in entries], [
            (0x03000100, 2, 10, 2, [(31, -56), (-14, -8)]),
            (0x03000100, 1, 10, 2, [(31, -56), (-14, -8)]),
        ])

        # direction 0 has 3 frames on disk, more than frameCount, direction 1 has 2
        keys = {0x03000100, 0x03000101, 0x03000102, 0x0300010A, 0x0300010B, 0x03000110, 0x03000111, 0x03000113, 0x0F000104, 0x0F000105}
        sequences, referenced, runs = g.build_magic_sequences(keys, entries, hardcodedGfxIDs=(0x0F000104, 0x0F000105))
        self.assertEqual((referenced, runs), (4, 2))
        self.assertEqual(sequences, {
            (3, 0x100, 0): ((31, -56), [0x03000100, 0x03000101, 0x03000102]),
            (3, 0x100, 1): ((-14, -8), [0x0300010A, 0x0300010B]),
            (3, 0x110, 0): ((0, 0), [0x03000110, 0x03000111]),
            (3, 0x113, 0): ((0, 0), [0x03000113]),
            (15, 0x104, 0): ((0, 0), [0x0F000104]),
            (15, 0x105, 0): ((0, 0), [0x0F000105]),
        })

        with self.assertRaisesRegex(RuntimeError, "needs 2 frames"):
            g.build_magic_sequences(keys - {0x0300010B}, entries, hardcodedGfxIDs=())

    def test_magic_skips_weapon_effect(self):
        record = self.source / "magicrecord.inc"
        record.write_text("")
        for key in [0x0700076B, 0x0700076C, 0x070016B7, *g.MAGIC_HARDCODED_GFX_IDS]:
            touch_frame(self.source, key)
        sequences = g.collect_magic_sequences(self.source, record)
        self.assertEqual(sorted(k for k in sequences if k[0] == 7), [(7, 0x76B, 0, (0, 0))])


class ConvertTest(TmpDirTest):
    def convert(self, typeName, ident, keys):
        frames = dict(g.list_frames(self.source))
        frames = [frames[key] for key in keys]
        self.assertEqual(g.convert_one((typeName, self.output, ident, frames, 100, None, True)), len(frames))
        path, = self.output.iterdir()
        return path

    def test_aligned_trimmed_alpha_and_repeated_frames(self):
        save_frame(self.source, 0, -4, -6, pixel=((1, 1), (90, 80, 70, 128)))
        save_frame(self.source, 1, -2, -3, pixel=((1, 1), (10, 20, 30, 255)))
        save_frame(self.source, 2, -2, -3, pixel=((1, 1), (10, 20, 30, 255)))

        path = self.convert("hero", 0, [0, 1, 2])
        meta = g.decode_human_name(path.name)
        self.assertEqual((meta["dx"], meta["dy"]), (-3, -5))
        with Image.open(path) as apng:
            self.assertEqual((apng.size, apng.n_frames), ((3, 4), 3))
            self.assertEqual(apng.convert("RGBA").getpixel((0, 0)), (90, 80, 70, 128))
            for index in range(3):
                apng.seek(index)
                pixels = apng.convert("RGBA")
                self.assertEqual(pixels.getpixel((1, 1)), (0, 0, 0, 0))
                if index:
                    self.assertEqual(pixels.getpixel((0, 0)), (0, 0, 0, 0))
                    self.assertEqual(pixels.getpixel((2, 3)), (10, 20, 30, 255))

    def test_placeholders_keep_blank_slots(self):
        save_frame(self.source, 0, -4, -6, pixel=((1, 1), (90, 80, 70, 128)))
        save_frame(self.source, 1, 9703, 9775, (4, 2))
        save_frame(self.source, 2, -2, -3, pixel=((1, 1), (10, 20, 30, 255)))
        save_frame(self.source, 3, -9703, 9775, (4, 2), ((0, 0), (1, 2, 3, 255)))

        path = self.convert("hair", 0, [0, 1, 2, 3])
        meta = g.decode_human_name(path.name)
        self.assertEqual((meta["dx"], meta["dy"]), (-3, -5))
        with Image.open(path) as apng:
            self.assertEqual((apng.size, apng.n_frames), ((3, 4), 4))
            for index in [1, 3]:
                apng.seek(index)
                self.assertIsNone(apng.convert("RGBA").getchannel("A").getbbox())
            apng.seek(2)
            self.assertEqual(apng.convert("RGBA").getpixel((2, 3)), (10, 20, 30, 255))

    def test_magic_with_head(self):
        keys = [0x03000100, 0x03000101, 0x03000102]
        save_frame(self.source, keys[0], -4, -6, pixel=((1, 1), (90, 80, 70, 128)))
        save_frame(self.source, keys[1], 9703, 9775)
        save_frame(self.source, keys[2], -2, -3, pixel=((1, 1), (10, 20, 30, 255)))

        path = self.convert("magic", (3, 0x100, 0, (31, -56)), keys)
        self.assertEqual(g.decode_magic_name(path.name), {
            "dx": -3, "dy": -5, "head": (31, -56), "tail": (0, 0), "direction": 0, "image": 0x100, "file": 3,
        })
        with Image.open(path) as apng:
            self.assertEqual((apng.size, apng.n_frames), ((3, 4), 3))

    def test_equip_copied_and_renamed(self):
        equipDir = self.source / "texture" / "equip"
        equipDir.mkdir(parents=True)
        save_frame(equipDir, 0x01000B72, -128, 6, pixel=((1, 1), (90, 80, 70, 128)))

        (ident, frames), = g.collect_sequences(self.source, "equip").items()
        self.assertEqual(g.convert_one(("equip", self.output, ident, frames, 100, None, True)), 1)
        path, = self.output.iterdir()
        self.assertEqual(path.name, "01000B72006F80.PNG")
        self.assertEqual(g.decode_static_name(path.name), {"dx": -128, "dy": 6, "key": 0x01000B72})
        self.assertEqual(path.read_bytes(), Path(frames[0][0]).read_bytes())

    def test_selectchar_overlay_padded_to_body(self):
        charDir = self.source / "texture" / "selectchar"
        charDir.mkdir(parents=True)
        body = (2 << 10) | (1 << 9) | (3 << 5)  # job 2, male, motion 3
        for frame in range(4):
            save_frame(charDir, body | frame, -2, -3, pixel=((1, 1), (10, 20, 30, 255)))
        for frame in [1, 2]:
            save_frame(charDir, body | (1 << 13) | frame, -4, -6, pixel=((1, 1), (90, 80, 70, 128)))

        sequences = g.collect_sequences(self.source, "selectchar")
        overlay = (1 << 19) | (1 << 17) | (2 << 9) | (3 << 3)
        self.assertEqual(sorted(sequences), [overlay & ~(1 << 19), overlay])
        self.assertEqual([f is None for f in sequences[overlay]], [True, False, False, True])

        self.assertEqual(g.convert_one(("selectchar", self.output, overlay, sequences[overlay], 100, None, True)), 2)
        path, = self.output.iterdir()
        self.assertEqual(g.decode_human_name(path.name), {
            "dx": -3, "dy": -5, "direction": 0, "motion": 3, "layer": 1, "shadow": 0, "gender": 1, "id": 2,
        })
        with Image.open(path) as apng:
            self.assertEqual(apng.n_frames, 4)
            for index in [0, 3]:
                apng.seek(index)
                self.assertIsNone(apng.convert("RGBA").getchannel("A").getbbox())

        save_frame(charDir, body | (1 << 13) | 4, -4, -6)
        with self.assertRaisesRegex(RuntimeError, "body has 4 frames"):
            g.collect_sequences(self.source, "selectchar")

    def test_proguse_groups_and_static_copies(self):
        useDir = self.source / "texture" / "proguse"
        useDir.mkdir(parents=True)
        for key, ext in [(0x300, "PNG"), (0x301, "png"), (0x302, "PNG"), (0x400, "png")]:
            Image.new("RGBA", (3, 2), (key & 0xFF, 1, 2, 255)).save(useDir / f"{key:08X}.{ext}")

        groupsPath = self.source / "groups.txt"
        groupsPath.write_text("""
            # comment
            button  00000301  00000300  00000302  name=00000301  # off differs
            button  00000300  00000300  00000302
            anim    00000300-00000301  250
        """)
        with self.assertRaisesRegex(ValueError, "line 5: APNG name 00000300 already used by line 4"):
            g.collect_sequences(self.source, "proguse", proguseGroups=groupsPath)

        groupsPath.write_text("button 00000301 00000300 00000302 name=00000301\nbutton 00000300 00000300 00000302\nanim 00000301-00000302 250 name=00000302\n")
        sequences = g.collect_sequences(self.source, "proguse", proguseGroups=groupsPath)
        self.assertEqual(sorted(sequences), [0x300, 0x301, 0x302, 0x400])
        for key, (paths, delayMs, isAPNG) in sequences.items():
            g.convert_one(("proguse", self.output, key, (paths, delayMs, isAPNG), 100, None, True))

        self.assertEqual(sorted(p.name for p in self.output.iterdir()), ["00000300.PNG", "00000301.PNG", "00000302.PNG", "00000400.PNG"])
        self.assertEqual((self.output / "00000400.PNG").read_bytes(), (useDir / "00000400.png").read_bytes())
        for name, reds, delayMs in [("00000301.PNG", [1, 0, 2], 100), ("00000300.PNG", [0, 0, 2], 100), ("00000302.PNG", [1, 2], 250)]:
            with Image.open(self.output / name) as apng:
                self.assertEqual((apng.size, apng.n_frames), ((3, 2), len(reds)))
                for index, red in enumerate(reds):
                    apng.seek(index)
                    self.assertEqual(apng.info["duration"], delayMs)
                    self.assertEqual(apng.convert("RGBA").getpixel((0, 0)), (red, 1, 2, 255))

        for line, msg in [("button 00000300 00000301 00000302 name=00000400", "not a key of the group"),
                          ("button - 00000301 00000302", "invalid key"),
                          ("anim 00000301-00000300 100", "FIRST > LAST")]:
            with self.assertRaisesRegex(ValueError, msg):
                g.parse_proguse_groups(line)
    def test_emoji_sheet_to_apng(self):
        # 5 frames of 2x3 in a 3x2 grid, the spare cell 5 has a stray pixel and is dropped
        sheet = Image.new("RGBA", (6, 6), (9, 9, 9, 0))
        for cell in range(6):
            sheet.putpixel(((cell % 3) * 2, (cell // 3) * 3), (cell, 7, 8, 255))
        sheet.save(self.source / "01002A00050600020003000A.PNG")

        sequences = g.collect_emoji_sequences(self.source)
        self.assertEqual(list(sequences), [0x01002A])
        self.assertEqual(g.convert_one(("emoji", self.output, 0x01002A, sequences[0x01002A], 100, None, True)), 5)

        path, = self.output.iterdir()
        self.assertEqual(path.name, "0001002A00A.PNG")
        self.assertEqual(g.decode_emoji_name(path.name), {"frameH1": 10, "key": 0x01002A, "set": 1, "subset": 0x2A})
        self.assertEqual(g.decode_emoji_name(g.encode_emoji_name(0xFFFFFF, -2))["frameH1"], -2)
        self.assertEqual(g.read_apng_delays(path), [(1, 6)] * 5)
        with Image.open(path) as apng:
            self.assertEqual((apng.size, apng.n_frames), ((2, 3), 5))
            for index in range(5):
                apng.seek(index)
                self.assertEqual(apng.convert("RGBA").getpixel((0, 0)), (index, 7, 8, 255))

        for name in ["01002A01050600020003000A.PNG", "01002A00000600020003000A.PNG", "01002A0005060002000.PNG"]:
            with self.assertRaises(ValueError):
                g.parse_emoji_old_name(name)


class MirrorTest(TmpDirTest):
    def run_main(self, *args):
        with mock.patch.object(sys, "argv", ["gfxres2apng.py", "--res-path", str(self.source), "--output-dir", str(self.output), "--jobs", "1", *args]):
            return g.main()

    def test_mirror_tree(self):
        files = {
            ".git/HEAD": b"ref",
            "LICENSE": b"license",
            "font/00_A.TTF": b"font",
            "sound/bgm/01.mp3": b"mp3",
            "texture/monster/000000001000010020.PNG": b"not converted",
        }
        for rel, data in files.items():
            (self.source / rel).parent.mkdir(parents=True, exist_ok=True)
            (self.source / rel).write_bytes(data)
        (self.source / "emoji").mkdir()
        Image.new("RGBA", (2, 2), (1, 2, 3, 255)).save(self.source / "emoji" / "000001000105000200020002.PNG")

        (self.output / "stale").touch()
        with self.assertRaises(SystemExit), mock.patch("sys.stderr"):
            self.run_main("--types", "emoji")

        (self.output / "stale").unlink()
        (self.output / ".git").mkdir()
        self.assertEqual(self.run_main("--types", "emoji"), 0)

        self.assertEqual(sorted(str(p.relative_to(self.output)) for p in self.output.rglob("*") if p.is_file()),
                         sorted(["emoji/00000001002.PNG", "LICENSE", "font/00_A.TTF", "sound/bgm/01.mp3", "texture/monster/000000001000010020.PNG"]))
        for rel, data in files.items():
            if not rel.startswith(".git"):
                self.assertEqual((self.output / rel).read_bytes(), data)

        with self.assertRaises(SystemExit), mock.patch("sys.stderr"):
            self.run_main()

        shutil.rmtree(self.output)
        self.assertEqual(self.run_main("--types", "emoji", "--skip-unconverted"), 0)
        self.assertEqual([str(p.relative_to(self.output)) for p in self.output.rglob("*") if p.is_file()], ["emoji/00000001002.PNG"])


if __name__ == "__main__":
    unittest.main()
