@echo off
if not exist bin mkdir bin
if not exist data mkdir data

echo ========================================================
echo  BIEN DICH HE THONG QUAN LY TAI CHINH VA TEST SUITE
echo ========================================================

echo.
echo [1/2] Bien dich Ung dung Chinh (FinanceApp.exe)...
g++ -std=c++17 -Wall -Iinclude src/Account.cpp src/Transaction.cpp src/Budget.cpp src/FinanceStorage.cpp src/FinanceManager.cpp src/main.cpp -o bin/FinanceApp.exe

if %ERRORLEVEL% equ 0 (
    echo [THANH CONG] Da tao file: bin\FinanceApp.exe
) else (
    echo [LOI] Bien dich FinanceApp that bai!
)

echo.
echo [2/2] Bien dich Bo Test Kiem Thu (test_finance.exe)...
g++ -std=c++17 -Wall -Iinclude src/Account.cpp src/Transaction.cpp src/Budget.cpp src/FinanceStorage.cpp src/FinanceManager.cpp tests/test_finance.cpp -o bin/test_finance.exe

if %ERRORLEVEL% equ 0 (
    echo [THANH CONG] Da tao file: bin\test_finance.exe
) else (
    echo [LOI] Bien dich test_finance that bai!
)

echo.
echo ========================================================
echo  HOAN TAT!
echo   - Chay ung dung: .\bin\FinanceApp.exe
echo   - Chay test:     .\bin\test_finance.exe (hoac .\run_test.bat)
echo ========================================================
