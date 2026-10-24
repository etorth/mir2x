import unittest

import npc


class NPCConversionTest(unittest.TestCase):
    def test_identity_fields(self):
        key = (1 << 23) | (90 << 12) | (2 << 8) | (3 << 5)
        name = npc.encode_new_name(key >> 5, 0, -296, 40)
        self.assertEqual(npc.decode_new_name(name), {"dx": -296, "dy": 40, "direction": 3, "motion": 2, "layer": 0, "shadow": 1, "look": 90})


if __name__ == "__main__":
    unittest.main()
