import tempfile
import unittest
from pathlib import Path

from PIL import Image

import hair


def save_frame(path, index, dx, dy, size, pixel=None):
    name = f"{index:08X}{int(dx > 0)}{int(dy > 0)}{abs(dx):04X}{abs(dy):04X}.PNG"
    img = Image.new("RGBA", size, (255, 255, 255, 0))
    if pixel:
        img.putpixel(*pixel)
    img.save(path / name)


class HairConversionTest(unittest.TestCase):
    def test_identity_fields(self):
        key = (4 << 14) | (1 << 22) | (19 << 8) | (5 << 5)
        name = hair.encode_new_name(key >> 5, -7, 3)
        self.assertEqual(hair.decode_new_name(name), {"dx": -7, "dy": 3, "direction": 5, "motion": 19, "gender": 1, "hair": 4})
        self.assertEqual((int(name[:-4], 16) >> 33) & 0x3, 0)
        with self.assertRaises(ValueError):
            hair.parse_old_name("008000000000000000.PNG")

    def test_placeholders_keep_blank_slots(self):
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "input"
            output = Path(tmp) / "output"
            source.mkdir()
            output.mkdir()
            save_frame(source, 0, -4, -6, (5, 5), ((1, 1), (90, 80, 70, 128)))
            save_frame(source, 1, 9703, 9775, (4, 2))
            save_frame(source, 2, -2, -3, (5, 5), ((1, 1), (10, 20, 30, 255)))
            save_frame(source, 3, -9703, 9775, (4, 2), ((0, 0), (1, 2, 3, 255)))

            frames = hair.collect_sequences(source)[0]
            self.assertEqual(hair.convert_one((source, output, 0, frames, 100, None, True)), 4)
            path, = output.iterdir()
            meta = hair.decode_new_name(path.name)
            self.assertEqual((meta["dx"], meta["dy"]), (-3, -5))
            with Image.open(path) as apng:
                self.assertEqual((apng.size, apng.n_frames), ((3, 4), 4))
                for index in [1, 3]:
                    apng.seek(index)
                    self.assertIsNone(apng.convert("RGBA").getchannel("A").getbbox())
                apng.seek(2)
                self.assertEqual(apng.convert("RGBA").getpixel((2, 3)), (10, 20, 30, 255))


if __name__ == "__main__":
    unittest.main()
