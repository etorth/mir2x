import tempfile
import unittest
from pathlib import Path

from PIL import Image

import hero


class HeroConversionTest(unittest.TestCase):
    def test_identity_fields(self):
        key = (173 << 14) | (1 << 22) | (1 << 23) | (1 << 24) | (45 << 8) | (7 << 5)
        name = hero.encode_new_name(key >> 5, -2048, 2047)
        self.assertEqual(hero.decode_new_name(name), {
            "dx": -2048, "dy": 2047, "direction": 7, "motion": 45,
            "layer": 1, "shadow": 1, "gender": 1, "dress": 173,
        })
        self.assertEqual(int(name[:-4], 16) >> 44, 0)

    def test_aligned_trimmed_alpha_and_repeated_frames(self):
        with tempfile.TemporaryDirectory() as tmp:
            source = Path(tmp) / "input"
            output = Path(tmp) / "output"
            source.mkdir()
            output.mkdir()
            for index, (dx, dy, color) in enumerate([
                (-4, -6, (90, 80, 70, 128)),
                (-2, -3, (10, 20, 30, 255)),
                (-2, -3, (10, 20, 30, 255)),
            ]):
                name = f"{index:08X}{int(dx > 0)}{int(dy > 0)}{abs(dx):04X}{abs(dy):04X}.PNG"
                img = Image.new("RGBA", (5, 5), (255, 255, 255, 0))
                img.putpixel((1, 1), color)
                img.save(source / name)
            frames = hero.collect_sequences(source)[0]
            self.assertEqual(hero.convert_one((source, output, 0, frames, 100, None, True)), 3)
            path, = output.iterdir()
            meta = hero.decode_new_name(path.name)
            self.assertEqual((meta["dx"], meta["dy"]), (-3, -5))
            with Image.open(path) as apng:
                self.assertEqual(apng.size, (3, 4))
                self.assertEqual(apng.n_frames, 3)
                self.assertEqual(apng.convert("RGBA").getpixel((0, 0)), (90, 80, 70, 128))
                for index in range(3):
                    apng.seek(index)
                    pixels = apng.convert("RGBA")
                    self.assertEqual(pixels.getpixel((1, 1)), (0, 0, 0, 0))
                    if index:
                        self.assertEqual(pixels.getpixel((0, 0)), (0, 0, 0, 0))
                        self.assertEqual(pixels.getpixel((2, 3)), (10, 20, 30, 255))

    def test_invalid_inputs(self):
        for name in ["020000000000000000.PNG", "000000002000000000.PNG"]:
            with self.assertRaises(ValueError):
                hero.parse_old_name(name)
        with self.assertRaises(ValueError):
            hero.encode_new_name(0, 2048, 0)
        with tempfile.TemporaryDirectory() as tmp:
            Path(tmp, "000000010000000000.PNG").touch()
            with self.assertRaisesRegex(RuntimeError, "not contiguous from 0"):
                hero.collect_sequences(tmp)


if __name__ == "__main__":
    unittest.main()
