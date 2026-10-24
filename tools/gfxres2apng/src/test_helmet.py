import unittest

import helmet


class HelmetConversionTest(unittest.TestCase):
    def test_identity_fields(self):
        key = (1 << 22) | (5 << 14) | (19 << 8) | (4 << 5)
        name = helmet.encode_new_name(key >> 5, -30, -89)
        self.assertEqual(helmet.decode_new_name(name), {"dx": -30, "dy": -89, "direction": 4, "motion": 19, "gender": 1, "helmet": 5})
        with self.assertRaises(ValueError):
            helmet.parse_old_name("008000000000000000.PNG")


if __name__ == "__main__":
    unittest.main()
