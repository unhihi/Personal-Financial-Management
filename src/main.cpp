#include <bits/stdc++.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include "FinanceManager.h"
#include "Common.h"

using namespace std;

// Ham xoa bo dem ban phim khi nhap sai hoac sau khi nhap so
void clearInputBuffer() {
    cin.clear();
    cin.ignore(10000, '\n');
}

// In menu chính
void showMainMenu() {
    cout << "\n===============================================================\n";
    cout << "        HE THONG QUAN LY TAI CHINH CA NHAN            \n";
    cout << "===============================================================\n";
    cout << " [1] Them giao dich moi (Income, Expense, Recurring Expense)\n";
    cout << " [2] Xem toan bo lich su giao dich\n";
    cout << " [3] Xem so du cac tai khoan (Cash, Bank, E-Wallet)\n";
    cout << " [4] Them tai khoan / vi moi\n";
    cout << " [5] Quan ly Ngan sach & Canh bao vuot muc (Budget Alerts)\n";
    cout << " [6] Mo phong Chi phi Dinh ky (Recurring Scheduler Simulation)\n";
    cout << " [7] Loc & Tim kiem giao dich (Ngay, Danh muc, Tai khoan)\n";
    cout << " [8] Bieu do truc quan ASCII Chart (So sanh chi tieu)\n";
    cout << " [9] Luu & Doc du lieu tu file CSV (FinanceStorage)\n";
    cout << " [10] Nap bo du lieu mau\n";
    cout << " [0] Thoat chuong trinh\n";
    cout << "---------------------------------------------------------------\n";
    cout << "Lua chon cua ban: ";
}

int main() {
#ifdef _WIN32
    // Thiết lập bảng mã UTF-8 cho console Windows
    SetConsoleOutputCP(CP_UTF8);
#endif

    FinanceManager manager;
    int choice = -1;
    while (choice != 0) {
        showMainMenu();
        if (!(cin >> choice)) {
            if (cin.eof()) break;
            cout << "[!] Lua chon khong hop le. Vui long nhap so!\n";
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        switch (choice) {

            case 1: {
                // Yêu cầu 1: Transaction Types (Income, Expense, RecurringExpense)
                cout << "\n--- THEM GIAO DICH MOI ---\n";
                cout << "Chon loai giao dich:\n";
                cout << "  1. Thu nhap (Income - Luong, Thuong, Freelance...)\n";
                cout << "  2. Chi tieu (Expense - An uong, Mua sam, Di lai...)\n";
                cout << "  3. Chi phi dinh ky (Recurring Expense - Tien nha, Internet...)\n";
                cout << "Lua chon (1-3): ";
                int typeChoice = 1;
                cin >> typeChoice;
                clearInputBuffer();

                string date, category, accountName, desc;
                double amount = 0.0;

                cout << "Nhap ngay giao dich (YYYY-MM-DD, VD: 2026-09-21): ";
                getline(cin, date);
                if (date.empty()) date = "2026-09-21";

                cout << "Nhap so tien: ";
                while (!(cin >> amount) || amount <= 0) {
                    cout << "[!] So tien phai lon hon 0. Nhap lai: ";
                    clearInputBuffer();
                }
                clearInputBuffer();

                cout << "Nhap danh muc (VD: An uong, Thue nha, Luong, Giai tri): ";
                getline(cin, category);

                cout << "Nhap tai khoan thanh toan (VD: Cash, Bank, E-Wallet): ";
                getline(cin, accountName);

                cout << "Nhap ghi chu: ";
                getline(cin, desc);

                int txId = manager.generateNextId();

                if (typeChoice == 1) {
                    manager.addTransaction(make_unique<Income>(txId, date, amount, category, accountName, desc));
                } else if (typeChoice == 2) {
                    manager.addTransaction(make_unique<Expense>(txId, date, amount, category, accountName, desc));
                } else if (typeChoice == 3) {
                    string freq = "Monthly";
                    int dueDay = 1;
                    cout << "Nhap tan suat (Mac dinh: Monthly): ";
                    string inputFreq;
                    getline(cin, inputFreq);
                    if (!inputFreq.empty()) freq = inputFreq;

                    cout << "Nhap ngay den han hang thang (1-31): ";
                    while (!(cin >> dueDay) || dueDay < 1 || dueDay > 31) {
                        cout << "[!] Ngay khong hop le (1-31). Nhap lai: ";
                        clearInputBuffer();
                    }
                    clearInputBuffer();

                    manager.addTransaction(make_unique<RecurringExpense>(txId, date, amount, category, accountName, desc, freq, dueDay));
                } else {
                    cout << "[!] Loai giao dich khong hop le.\n";
                }
                break;
            }
            case 2: {
                // Xem toàn bộ lịch sử giao dịch
                cout << "\n================ TOAN BO LICH SU GIAO DICH ================\n";
                manager.displayAllTransactions();
                break;
            }
            case 3: {
                // Yêu cầu 2: Multi-Account Balances
                manager.displayAccounts();
                break;
            }
            case 4: {
                // Thêm tài khoản mới
                string accName;
                double initialBalance = 0.0;
                cout << "\n--- THEM TAI KHOAN MOI ---\n";
                cout << "Nhap ten tai khoan (VD: MoMo, ZaloPay, ViettelMoney, VCB): ";
                getline(cin, accName);
                cout << "Nhap so du ban dau: ";
                while (!(cin >> initialBalance) || initialBalance < 0) {
                    cout << "[!] So du phai la so khong am. Nhap lai: ";
                    clearInputBuffer();
                }
                clearInputBuffer();
                manager.addAccount(accName, initialBalance);
                break;
            }

            case 5: {
                // Yêu cầu 3: Category Budget Limits & Over-budget Alerts
                cout << "\n--- QUAN LY NGAN SACH & CANH BAO ---\n";
                cout << "  1. Xem bao cao ngan sach hien tai\n";
                cout << "  2. Dat/Chinh sua han muc ngan sach cho danh muc\n";
                cout << "Lua chon: ";
                int bChoice = 1;
                cin >> bChoice;
                clearInputBuffer();

                if (bChoice == 1) {
                    cout << "Nhap thang can xem bao cao (1-12, hoac 0 de xem theo nam/tat ca): ";
                    int m = 0, y = 0;
                    cin >> m;
                    clearInputBuffer();
                    cout << "Nhap nam (VD: 2026, hoac 0 de xem tat ca cac nam): ";
                    cin >> y;
                    clearInputBuffer();
                    manager.displayBudgetReport(m, y);
                } else if (bChoice == 2) {
                    string cat;
                    double limit = 0.0;
                    cout << "Nhap ten danh muc (VD: An uong, Giai tri, Mua sam): ";
                    getline(cin, cat);
                    cout << "Nhap han muc toi da moi thang: ";
                    while (!(cin >> limit) || limit <= 0) {
                        cout << "[!] Han muc phai lon hon 0. Nhap lai: ";
                        clearInputBuffer();
                    }
                    clearInputBuffer();
                    manager.setBudgetLimit(cat, limit);
                }
                break;
            }
            case 6: {
                // Yêu cầu 4: Recurring Transaction Scheduler Simulation
                cout << "\n--- MO PHONG CHI PHI DINH KY HANG THANG ---\n";
                cout << "  1. Xem danh sach & tong chi phi co dinh hang thang\n";
                cout << "  2. Mo phong thuc thi chi phi dinh ky cho thang moi\n";
                cout << "Lua chon: ";
                int rChoice = 1;
                cin >> rChoice;
                clearInputBuffer();

                if (rChoice == 1) {
                    manager.displayRecurringPlan();
                } else if (rChoice == 2) {
                    int simMonth = 10, simYear = 2026;
                    cout << "Nhap thang can mo phong (1-12): ";
                    cin >> simMonth;
                    clearInputBuffer();
                    cout << "Nhap nam (VD: 2026): ";
                    cin >> simYear;
                    clearInputBuffer();

                    manager.executeRecurringExpensesForMonth(simYear, simMonth);
                }
                break;
            }
            case 7: {
                // Yêu cầu 5: Filtering & Searching (by Date Range, Category, Account)
                cout << "\n--- LOC & TIM KIEM GIAO DICH ---\n";
                cout << "  1. Loc theo khoang thoi gian (Date Range)\n";
                cout << "  2. Loc theo danh muc chi tieu (Category)\n";
                cout << "  3. Loc theo tai khoan thanh toan (Account)\n";
                cout << "Lua chon: ";
                int filterChoice = 1;
                cin >> filterChoice;
                clearInputBuffer();

                vector<const BaseTransaction*> results;
                if (filterChoice == 1) {
                    string startDate, endDate;
                    cout << "Nhap ngay bat dau (YYYY-MM-DD): ";
                    getline(cin, startDate);
                    cout << "Nhap ngay ket thuc (YYYY-MM-DD): ";
                    getline(cin, endDate);
                    results = manager.filterByDateRange(startDate, endDate);
                } else if (filterChoice == 2) {
                    string cat;
                    cout << "Nhap ten danh muc can tim: ";
                    getline(cin, cat);
                    results = manager.filterByCategory(cat);
                } else if (filterChoice == 3) {
                    string acc;
                    cout << "Nhap ten tai khoan can tim: ";
                    getline(cin, acc);
                    results = manager.filterByAccount(acc);
                }

                FinanceManager::printTransactionTable(results);

                if (!results.empty()) {
                    cout << "\nBan co muon xuat ket qua loc ra file CSV? (y/n): ";
                    char exportChoice;
                    cin >> exportChoice;
                    clearInputBuffer();
                    if (exportChoice == 'y' || exportChoice == 'Y') {
                        cout << "Nhap ten file luu (VD: data/search_result.csv): ";
                        string exportName;
                        getline(cin, exportName);
                        if (exportName.empty()) exportName = "data/search_result.csv";
                        manager.exportQueryResults(exportName, results);
                    }
                }
                break;
            }
            case 8: {
                // Yêu cầu 6: ASCII Chart
                cout << "\n--- BIEU DO TRUC QUAN ASCII CHART ---\n";
                cout << "  1. Bieu do so sanh chi tieu giua cac thang trong nam\n";
                cout << "  2. Bieu do so sanh chi tieu theo danh muc\n";
                cout << "Lua chon: ";
                int cChoice = 1;
                cin >> cChoice;
                clearInputBuffer();

                if (cChoice == 1) {
                    int chartYear = 2026;
                    cout << "Nhap nam can xem bieu do (VD: 2026): ";
                    cin >> chartYear;
                    clearInputBuffer();
                    manager.renderMonthlySpendingChart(chartYear);
                } else if (cChoice == 2) {
                    int cMonth = 0, cYear = 0;
                    cout << "Nhap thang can xem (1-12, hoac 0 de xem theo nam/tat ca): ";
                    cin >> cMonth;
                    clearInputBuffer();
                    cout << "Nhap nam (VD: 2026, hoac 0 de xem tat ca cac nam): ";
                    cin >> cYear;
                    clearInputBuffer();
                    manager.renderCategorySpendingChart(cMonth, cYear);
                }
                break;
            }
            case 9: {
                // Yêu cầu 7: CSV Export & Data Load
                cout << "\n--- LUU & DOC DU LIEU CSV ---\n";
                cout << "  1. Luu du lieu vao file CSV\n";
                cout << "  2. Doc & Khoi phuc du lieu tu file CSV\n";
                cout << "Lua chon: ";
                int sChoice = 1;
                cin >> sChoice;
                clearInputBuffer();

                string filename;
                cout << "Nhap duong dan file (Mac dinh: data/finance_data.csv): ";
                getline(cin, filename);
                if (filename.empty()) filename = "data/finance_data.csv";

                if (sChoice == 1) {
                    manager.saveData(filename);
                } else if (sChoice == 2) {
                    manager.loadData(filename);
                }
                break;
            }
            case 10: {
                manager.loadSampleData();
                break;
            }
            case 0: {
                cout << "\n[+] Cam on!\n";
                break;
            }
            default: {
                cout << "Lua chon khong hop le. Vui long chon lai!\n";
                break;
            }
        }
    }

    return 0;
}
