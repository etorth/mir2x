import unittest

import weapon


class WeaponConversionTest(unittest.TestCase):
    def test_identity_fields(self):
        key = (1 << 23) | (1 << 22) | (200 << 14) | (33 << 8) | (6 << 5)
        name = weapon.encode_new_name(key >> 5, -106, 45)
        self.assertEqual(weapon.decode_new_name(name), {"dx": -106, "dy": 45, "direction": 6, "motion": 33, "shadow": 1, "gender": 1, "weapon": 200})
        self.assertEqual((int(name[:-4], 16) >> 33) & 1, 0)
        with self.assertRaises(ValueError):
            weapon.parse_old_name("010000000000000000.PNG")


if __name__ == "__main__":
    unittest.main()
