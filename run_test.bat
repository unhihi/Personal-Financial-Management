@echo off
if not exist bin mkdir bin
if not exist data mkdir data

echo ========================================================
echo   DANG BIEN DICH VA CHAY TEST (test_finance.cpp)
echo ========================================================

g++ -std=c++17 -Wall -Iinclude src/Account.cpp src/Transaction.cpp src/Budget.cpp src/FinanceStorage.cpp src/FinanceManager.cpp tests/test_finance.cpp -o bin/test_finance.exe

if %ERRORLEVEL% equ 0 (
    echo.
    echo [BIEN DICH THANH CONG] Dang chay cac test case...
    echo --------------------------------------------------------
    bin\test_finance.exe
) else (
    echo.
    echo [LOI] Khong the bien dich test_finance.cpp!
)
