import json
import subprocess
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

from finance_data import EXE_PATH, import_finance_csv, validate_finance_csv


VALID_CSV = (
    "#ACCOUNTS\nName,Balance\nVí,1000.00\n"
    "#BUDGETS\nCategory,Limit\nĂn uống,500.00\n"
    "#TRANSACTIONS\n"
    "Type,Id,Date,Amount,Category,Account,Description,Frequency,DueDay\n"
    "Expense,1,2026-10-01,100.00,Ăn uống,Ví,Bữa sáng,,1\n"
).encode("utf-8")


class FinanceImportTests(unittest.TestCase):
    def test_accepts_export_format_and_utf8_bom(self):
        counts, normalized = validate_finance_csv(b"\xef\xbb\xbf" + VALID_CSV)
        self.assertEqual(counts, {"accounts": 1, "budgets": 1, "transactions": 1})
        self.assertEqual(normalized, VALID_CSV)

    def test_rejects_rows_that_cpp_would_silently_skip(self):
        invalid_files = (
            VALID_CSV.replace(b"100.00", b"not-a-number"),
            VALID_CSV.replace("Bữa sáng".encode(), "Bữa,sáng".encode()),
            VALID_CSV.replace(b"2026-10-01", b"2026-02-30"),
            VALID_CSV.replace(b",V\xc3\xad,", b",Kh\xc3\xa1c,"),
        )
        for content in invalid_files:
            with self.subTest(content=content):
                with self.assertRaises(ValueError):
                    validate_finance_csv(content)

    def test_import_keeps_a_backup(self):
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            data_file = Path(directory) / "finance_data.csv"
            data_file.write_bytes(b"old data")
            loaded = {"accounts": [{}], "budgets": {"food": 100}, "transactions": [{}]}
            with patch("finance_data.get_data", return_value=loaded):
                result, backup = import_finance_csv(VALID_CSV, data_file)
            self.assertEqual(result, loaded)
            self.assertEqual(data_file.read_bytes(), VALID_CSV)
            self.assertEqual(backup.read_bytes(), b"old data")

    def test_failed_import_restores_original(self):
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            data_file = Path(directory) / "finance_data.csv"
            data_file.write_bytes(b"old data")
            with patch("finance_data.get_data", side_effect=RuntimeError("read failed")):
                with self.assertRaises(RuntimeError):
                    import_finance_csv(VALID_CSV, data_file)
            self.assertEqual(data_file.read_bytes(), b"old data")
            self.assertEqual(list(Path(directory).glob("*.backup-*.csv")), [])

    def test_import_is_readable_by_cpp_backend(self):
        executable = EXE_PATH
        if not executable.exists():
            self.skipTest(f"{executable.name} has not been built")
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            data_file = Path(directory) / "data" / "finance_data.csv"

            def native_data():
                result = subprocess.run(
                    [str(executable), "get-data"], cwd=directory,
                    capture_output=True, text=True, encoding="utf-8", check=True,
                )
                return json.loads(result.stdout.splitlines()[-1])

            with patch("finance_data.get_data", side_effect=native_data):
                loaded, backup = import_finance_csv(VALID_CSV, data_file)
            self.assertIsNone(backup)
            self.assertEqual(loaded["accounts"][0]["name"], "Ví")
            self.assertEqual(loaded["transactions"][0]["description"], "Bữa sáng")

    def test_unicode_command_line_is_saved_as_utf8(self):
        if not EXE_PATH.exists():
            self.skipTest(f"{EXE_PATH.name} has not been built")
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            result = subprocess.run(
                [str(EXE_PATH), "add-account", "Ví số", "100"], cwd=directory,
                capture_output=True, text=True, encoding="utf-8", check=True,
            )
            self.assertEqual(json.loads(result.stdout)["accounts"][0]["name"], "Ví số")
            saved = (Path(directory) / "data" / "finance_data.csv").read_bytes()
            self.assertIn("Ví số".encode("utf-8"), saved)
            saved.decode("utf-8")

    def test_legacy_ansi_byte_is_read_without_changing_csv(self):
        if not EXE_PATH.exists():
            self.skipTest(f"{EXE_PATH.name} has not been built")
        with tempfile.TemporaryDirectory(dir=Path(__file__).parent) as directory:
            data_dir = Path(directory) / "data"
            data_dir.mkdir()
            data_file = data_dir / "finance_data.csv"
            legacy = (
                b"#ACCOUNTS\nName,Balance\nCaf\xe9 " + "Việt".encode("utf-8") +
                b",100.00\n#BUDGETS\nCategory,Limit\n"
                b"#TRANSACTIONS\n"
                b"Type,Id,Date,Amount,Category,Account,Description,Frequency,DueDay\n"
            )
            data_file.write_bytes(legacy)

            result = subprocess.run(
                [str(EXE_PATH), "get-data"], cwd=directory,
                capture_output=True, text=True, encoding="utf-8", check=True,
            )
            self.assertEqual(json.loads(result.stdout)["accounts"][0]["name"], "Café Việt")
            self.assertEqual(data_file.read_bytes(), legacy)

            result = subprocess.run(
                [str(EXE_PATH), "add-account", "Café Việt", "100"], cwd=directory,
                capture_output=True, text=True, encoding="utf-8", check=True,
            )
            self.assertEqual(len(json.loads(result.stdout)["accounts"]), 1)
            self.assertIn("Café Việt".encode("utf-8"), data_file.read_bytes())
            data_file.read_bytes().decode("utf-8")


if __name__ == "__main__":
    unittest.main()
