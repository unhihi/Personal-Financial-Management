"""
finance_data.py
Module goi FinanceApp qua subprocess va parse ket qua JSON.
Toan bo logic nghiep vu do C++ xu ly - Python chi la lop giao tiep.
"""

import subprocess
import json
import os
import sys
import math
import re
import shutil
import tempfile
import time
from datetime import date
from pathlib import Path

PROJECT_DIR = Path(__file__).resolve().parent


def backend_path(platform_name):
    name = "FinanceApp.exe" if platform_name == "win32" else "FinanceApp"
    return PROJECT_DIR / "bin" / name


EXE_PATH = backend_path(sys.platform)
BUILD_COMMAND = r".\build.bat" if sys.platform == "win32" else "bash build.sh"
DATA_FILE = PROJECT_DIR / "data" / "finance_data.csv"


def call_cpp(*args):
    """
    Goi FinanceApp voi cac tham so args.
    Tra ve dict da parse tu JSON stdout cua C++.
    Nem RuntimeError neu exe bao loi.
    """
    cmd = [str(EXE_PATH)] + [str(a) for a in args]
    try:
        result = subprocess.run(
            cmd,
            cwd=PROJECT_DIR,
            capture_output=True,
            text=True,
            encoding="utf-8",
            timeout=10,
        )
    except FileNotFoundError as e:
        raise RuntimeError(
            f"Không tìm thấy '{EXE_PATH}'. "
            f"Hãy chạy {BUILD_COMMAND} trong thư mục project trước khi mở Streamlit."
        ) from e
    except subprocess.TimeoutExpired as e:
        raise RuntimeError(f"{EXE_PATH.name} chạy quá 10 giây.") from e
    except OSError as e:
        raise RuntimeError(f"Không thể chạy {EXE_PATH.name}: {e}") from e

    # stdout co the rong hoac None neu backend khong tra ket qua.
    stdout_lines = [line for line in (result.stdout or "").splitlines() if line.strip()]
    if not stdout_lines:
        detail = (result.stderr or "").strip() or f"mã thoát {result.returncode}"
        raise RuntimeError(
            f"{EXE_PATH.name} không trả dữ liệu JSON. "
            f"Chi tiết: {detail}. Hãy kiểm tra kết quả chạy {BUILD_COMMAND}."
        )

    try:
        return json.loads(stdout_lines[-1])
    except json.JSONDecodeError as e:
        raise RuntimeError(
            f"{EXE_PATH.name} trả JSON không hợp lệ: {e}. "
            f"stdout: {result.stdout}; stderr: {result.stderr}"
        ) from e


# ─────────────────────────── API ĐƠN GIẢN ───────────────────────────


def add_account(name, balance):
    """Them tai khoan moi, tra ve du lieu moi nhat."""
    return call_cpp("add-account", name, balance)


def add_transaction(tx_type, date, amount, category, account, desc,
                    freq="Monthly", due_day=1):
    """
    Them giao dich moi.
    tx_type: "Income" | "Expense" | "RecurringExpense"
    Tra ve dict co key: ok, budget_alert, data
    """
    return call_cpp("add-transaction", tx_type, date, amount,
                    category, account, desc, freq, due_day)


def set_budget(category, limit):
    """Dat han muc ngan sach, tra ve du lieu moi nhat."""
    return call_cpp("set-budget", category, limit)


def simulate_recurring(year, month):
    """Mo phong chi phi dinh ky cho thang/nam, tra ve dict co key: ok, applied, data."""
    return call_cpp("simulate-recurring", year, month)


def preview_recurring(year, month):
    """Xem truoc ket qua mo phong trong bo nho, khong ghi file CSV."""
    return call_cpp("preview-recurring", year, month)


# ─────────────────────────── HELPER UI ───────────────────────────

def format_vnd(amount):
    """Dinh dang so tien theo kieu Viet Nam: 1.500.000 VND"""
    return f"{int(amount):,} VND".replace(",", ".")
