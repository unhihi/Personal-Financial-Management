import json
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

from finance_data import EXE_PATH


class RecurringPreviewTests(unittest.TestCase):
    def test_preview_does_not_save_and_apply_is_idempotent(self):
        executable = EXE_PATH
        if not executable.exists():
            self.skipTest(f"{executable.name} has not been built")

        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            data_file = Path(directory) / "data" / "finance_data.csv"
            data_file.parent.mkdir()
            sample_file = executable.parents[1] / "data" / "finance_data_fixed.csv"
            shutil.copy2(sample_file, data_file.parent / sample_file.name)

            def run(*args):
                result = subprocess.run(
                    [str(executable), *args], cwd=directory,
                    capture_output=True, text=True, encoding="utf-8", check=True,
                )
                return json.loads(result.stdout)

            original = run("load-sample")
            original_csv = data_file.read_bytes()
            preview = run("preview-recurring", "2026", "10")
            self.assertEqual(data_file.read_bytes(), original_csv)
            self.assertGreater(preview["applied"], 0)
            self.assertEqual(
                len(preview["data"]["transactions"]),
                len(original["transactions"]) + preview["applied"],
            )

            applied = run("simulate-recurring", "2026", "10")
            self.assertEqual(applied["applied"], preview["applied"])
            self.assertNotEqual(data_file.read_bytes(), original_csv)
            self.assertEqual(run("preview-recurring", "2026", "10")["applied"], 0)
            self.assertEqual(run("simulate-recurring", "2026", "10")["applied"], 0)


if __name__ == "__main__":
    unittest.main()
