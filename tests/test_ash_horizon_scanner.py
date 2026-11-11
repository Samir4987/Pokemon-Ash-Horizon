# Pokémon: Ash Horizon
# Copyright (C) 2026 Pokémon: Ash Horizon contributors
# SPDX-License-Identifier: GPL-3.0-only
# Licensed under GNU GPL version 3 only; see the LICENSE file for details.

import sys
import tempfile
import unittest
from pathlib import Path

# Make the repository's tools/ module importable under unittest discovery.
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from ash_horizon_scanner import scan_database


class ScannerTests(unittest.TestCase):
    def test_duplicate_sections_and_keys(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            pbs = root / "PBS"
            pbs.mkdir()
            (pbs / "pokemon.txt").write_text(
                "[BULBASAUR]\nName=Bulbasaur\n[bulbasaur]\nName=Again\nName=Third\n",
                encoding="utf-8",
            )
            report = scan_database(root)
            self.assertEqual(report["files_scanned"], 1)
            self.assertEqual(report["sections_scanned"], 2)
            codes = [issue["code"] for item in report["files"] for issue in item["issues"]]
            self.assertIn("duplicate_section", codes)
            self.assertIn("duplicate_key", codes)

    def test_key_before_section(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            pbs = root / "PBS"
            pbs.mkdir()
            (pbs / "moves.txt").write_text(
                "Name=Orphan\n[TACKLE]\nName=Tackle\n", encoding="utf-8"
            )
            report = scan_database(root)
            codes = [issue["code"] for item in report["files"] for issue in item["issues"]]
            self.assertIn("key_outside_section", codes)

    def test_empty_database(self):
        with tempfile.TemporaryDirectory() as temp:
            report = scan_database(Path(temp))
            self.assertEqual(report["files_scanned"], 0)
            self.assertEqual(report["errors"], 0)


if __name__ == "__main__":
    unittest.main()
