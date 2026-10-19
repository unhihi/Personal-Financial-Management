import subprocess
import unittest
from unittest.mock import patch

from finance_data import BUILD_COMMAND, EXE_PATH, PROJECT_DIR, backend_path, get_data


class FinanceBridgeTests(unittest.TestCase):
    def test_backend_path_uses_platform_filename(self):
        self.assertEqual(backend_path("win32").name, "FinanceApp.exe")
        self.assertEqual(backend_path("darwin").name, "FinanceApp")

    @patch("finance_data.subprocess.run")
    def test_missing_stdout_reports_backend_error(self, run):
        run.return_value = subprocess.CompletedProcess(
            args=[], returncode=1, stdout=None, stderr="backend failure"
        )

        with self.assertRaisesRegex(RuntimeError, "backend failure"):
            get_data()

    @patch("finance_data.subprocess.run")
    def test_runs_backend_from_project_and_parses_json(self, run):
        run.return_value = subprocess.CompletedProcess(
            args=[], returncode=0,
            stdout='log\n{"accounts":[],"budgets":{},"transactions":[]}\n',
            stderr="",
        )

        self.assertEqual(get_data()["accounts"], [])
        self.assertEqual(run.call_args.args[0], [str(EXE_PATH), "get-data"])
        self.assertEqual(run.call_args.kwargs["cwd"], PROJECT_DIR)

    @patch("finance_data.subprocess.run", side_effect=FileNotFoundError)
    def test_missing_executable_explains_build_step(self, run):
        with self.assertRaises(RuntimeError) as error:
            get_data()
        self.assertIn(BUILD_COMMAND, str(error.exception))


if __name__ == "__main__":
    unittest.main()
