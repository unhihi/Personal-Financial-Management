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

def get_data():
    """Lay toan bo du lieu (accounts, budgets, transactions)."""
    return call_cpp("get-data")


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


def load_sample():
    """Nap du lieu mau, tra ve du lieu moi nhat."""
    return call_cpp("load-sample")


def validate_finance_csv(content):
    """Kiem tra CSV dung dinh dang ma FinanceStorage.cpp co the doc."""
    try:
        lines = [line.strip() for line in content.decode("utf-8-sig").splitlines()
                 if line.strip()]
    except UnicodeDecodeError as e:
        raise ValueError("Tệp phải dùng mã hóa UTF-8.") from e

    sections = (
        ("#ACCOUNTS", "Name,Balance", 2),
        ("#BUDGETS", "Category,Limit", 2),
        ("#TRANSACTIONS", "Type,Id,Date,Amount,Category,Account,Description,Frequency,DueDay", 9),
    )
    counts = {"accounts": 0, "budgets": 0, "transactions": 0}
    account_names = set()
    categories = set()
    transaction_ids = set()
    index = 0

    def number(value, label, row, positive=False):
        try:
            parsed = float(value)
        except ValueError as e:
            raise ValueError(f"Dòng {row}: {label} không phải là số.") from e
        if not math.isfinite(parsed) or parsed < 0 or (positive and parsed == 0):
            raise ValueError(f"Dòng {row}: {label} phải là số {'dương' if positive else 'không âm'}.")

    for section_index, (marker, header, columns) in enumerate(sections):
        if index + 1 >= len(lines) or lines[index:index + 2] != [marker, header]:
            raise ValueError(f"Thiếu mục {marker} hoặc dòng tiêu đề {header}.")
        index += 2
        next_marker = sections[section_index + 1][0] if section_index + 1 < len(sections) else None
        while index < len(lines) and lines[index] != next_marker:
            row = index + 1
            fields = [field.strip() for field in lines[index].split(",")]
            if len(fields) != columns:
                raise ValueError(f"Dòng {row}: cần đúng {columns} cột, phân cách bằng dấu phẩy.")
            if marker == "#ACCOUNTS":
                name, balance = fields
                if not name or name in account_names:
                    raise ValueError(f"Dòng {row}: tên tài khoản trống hoặc trùng lặp.")
                number(balance, "số dư", row)
                account_names.add(name)
                counts["accounts"] += 1
            elif marker == "#BUDGETS":
                category, limit = fields
                if not category or category in categories:
                    raise ValueError(f"Dòng {row}: danh mục ngân sách trống hoặc trùng lặp.")
                number(limit, "hạn mức", row)
                categories.add(category)
                counts["budgets"] += 1
            else:
                tx_type, tx_id, tx_date, amount, category, account, _, frequency, due_day = fields
                if tx_type not in ("Income", "Expense", "RecurringExpense"):
                    raise ValueError(f"Dòng {row}: loại giao dịch không hợp lệ.")
                if not tx_id.isdecimal() or int(tx_id) < 1 or int(tx_id) in transaction_ids:
                    raise ValueError(f"Dòng {row}: ID giao dịch phải là số dương và không trùng.")
                if not re.fullmatch(r"\d{4}-\d{2}-\d{2}", tx_date):
                    raise ValueError(f"Dòng {row}: ngày phải có dạng YYYY-MM-DD.")
                try:
                    date.fromisoformat(tx_date)
                except ValueError as e:
                    raise ValueError(f"Dòng {row}: ngày không hợp lệ.") from e
                number(amount, "số tiền", row, positive=True)
                if not category or account not in account_names:
                    raise ValueError(f"Dòng {row}: danh mục trống hoặc tài khoản không tồn tại.")
                if tx_type == "RecurringExpense":
                    if frequency not in ("Monthly", "Weekly", "Yearly"):
                        raise ValueError(f"Dòng {row}: tần suất định kỳ không hợp lệ.")
                    if not due_day.isdecimal() or not 1 <= int(due_day) <= 31:
                        raise ValueError(f"Dòng {row}: ngày đến hạn phải từ 1 đến 31.")
                transaction_ids.add(int(tx_id))
                counts["transactions"] += 1
            index += 1

    return counts, ("\n".join(lines) + "\n").encode("utf-8")


def import_finance_csv(content, data_file=DATA_FILE):
    """Nap CSV da kiem tra, giu ban sao du lieu cu de khoi phuc neu can."""
    counts, normalized = validate_finance_csv(content)
    data_file = Path(data_file)
    data_file.parent.mkdir(parents=True, exist_ok=True)
    backup = None
    temporary = None
    try:
        with tempfile.NamedTemporaryFile(dir=data_file.parent, suffix=".csv", delete=False) as file:
            temporary = Path(file.name)
            file.write(normalized)
        if data_file.exists():
            backup = data_file.with_name(f"{data_file.stem}.backup-{time.time_ns()}.csv")
            shutil.copy2(data_file, backup)
        os.replace(temporary, data_file)
        temporary = None
        result = get_data()
        if any(len(result[key]) != counts[key] for key in counts):
            raise RuntimeError("Ứng dụng C++ không đọc đủ dữ liệu trong tệp đã nạp.")
        return result, backup
    except Exception:
        if temporary is None:
            if backup is not None:
                os.replace(backup, data_file)
            elif data_file.exists():
                data_file.unlink()
        raise
    finally:
        if temporary is not None:
            temporary.unlink(missing_ok=True)


# ─────────────────────────── HELPER UI ───────────────────────────

def format_vnd(amount):
    """Dinh dang so tien theo kieu Viet Nam: 1.500.000 VND"""
    return f"{int(amount):,} VND".replace(",", ".")
