import tempfile
import unittest
from pathlib import Path

from PIL import Image

import magic

RECORD = """
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


class MagicConversionTest(unittest.TestCase):
    def test_identity_fields(self):
        name = magic.encode_new_name((15, 0xFFFF, 15), (31, -56), (-2048, 2047), -296, 40)
        self.assertEqual(len(name), 28)
        self.assertEqual(magic.decode_new_name(name), {
            "dx": -296, "dy": 40, "head": (31, -56), "tail": (-2048, 2047),
            "direction": 15, "image": 0xFFFF, "file": 15,
        })
        self.assertTrue(magic.encode_new_name((3, 0x100, 1), (0, 0), (0, 0), 0, 0).startswith("301001000000000000"))
        with self.assertRaises(ValueError):
            magic.encode_new_name((16, 0, 0), (0, 0), (0, 0), 0, 0)

    def test_record_sequences_and_unreferenced_runs(self):
        entries = magic.parse_magic_record(RECORD)
        self.assertEqual([(e["gfxID"], e["frameCount"], e["stride"], e["dirType"], e["heads"]) for e in entries], [
            (0x03000100, 2, 10, 2, [(31, -56), (-14, -8)]),
            (0x03000100, 1, 10, 2, [(31, -56), (-14, -8)]),
        ])

        # direction 0 has 3 frames on disk, more than frameCount, direction 1 has 2
        keys = {0x03000100, 0x03000101, 0x03000102, 0x0300010A, 0x0300010B, 0x03000110, 0x03000111, 0x03000113, 0x0F000104, 0x0F000105}
        sequences, referenced, runs = magic.build_sequences(keys, entries, hardcodedGfxIDs=(0x0F000104, 0x0F000105))
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
            magic.build_sequences(keys - {0x0300010B}, entries, hardcodedGfxIDs=())

    def test_convert_with_head_and_placeholder(self):
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "input"
            output = Path(tmp) / "output"
            source.mkdir()
            output.mkdir()
            frames = []
            for index, (dx, dy, color) in enumerate([(-4, -6, (90, 80, 70, 128)), (9703, 9775, None), (-2, -3, (10, 20, 30, 255))]):
                name = f"{0x03000100 + index:08X}{int(dx > 0)}{int(dy > 0)}{abs(dx):04X}{abs(dy):04X}.PNG"
                img = Image.new("RGBA", (5, 5), (255, 255, 255, 0))
                if color:
                    img.putpixel((1, 1), color)
                img.save(source / name)
                frames.append(magic.collect_frames(source)[0x03000100 + index])

            self.assertEqual(magic.convert_one((source, output, (3, 0x100, 0), (31, -56), frames, 100, None, True)), 3)
            path, = output.iterdir()
            self.assertEqual(magic.decode_new_name(path.name), {
                "dx": -3, "dy": -5, "head": (31, -56), "tail": (0, 0), "direction": 0, "image": 0x100, "file": 3,
            })
            with Image.open(path) as apng:
                self.assertEqual((apng.size, apng.n_frames), ((3, 4), 3))
                apng.seek(1)
                self.assertIsNone(apng.convert("RGBA").getchannel("A").getbbox())


if __name__ == "__main__":
    unittest.main()
