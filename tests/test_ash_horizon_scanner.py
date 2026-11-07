import tempfile
import unittest
from pathlib import Path
from ash_horizon_scanner import scan_database

class ScannerTests(unittest.TestCase):
    def test_duplicate_sections_and_keys(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp); pbs = root / "PBS"; pbs.mkdir()
            (pbs / "pokemon.txt").write_text(
                "[BULBASAUR]\nName=Bulbasaur\n[bulbasaur]\nName=Again\nName=Third\n",
                encoding="utf-8")
            report = scan_database(root)
            self.assertEqual(report["files_scanned"], 1)
            self.assertEqual(report["sections_scanned"], 2)
            codes = [x["code"] for f in report["files"] for x in f["issues"]]
            self.assertIn("duplicate_section", codes)
            self.assertIn("duplicate_key", codes)

    def test_key_before_section(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp); pbs = root / "PBS"; pbs.mkdir()
            (pbs / "moves.txt").write_text("Name=Orphan\n[TACKLE]\nName=Tackle\n", encoding="utf-8")
            report = scan_database(root)
            codes = [x["code"] for f in report["files"] for x in f["issues"]]
            self.assertIn("key_outside_section", codes)

    def test_empty_database(self):
        with tempfile.TemporaryDirectory() as temp:
            report = scan_database(Path(temp))
            self.assertEqual(report["files_scanned"], 0)
            self.assertEqual(report["errors"], 0)

if __name__ == "__main__":
    unittest.main()
