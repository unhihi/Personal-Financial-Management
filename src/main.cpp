/*
 * main.cpp  –  CLI Entry Point cho FinanceApp
 *
 * Cach dung:
 *   FinanceApp get-data
 *   FinanceApp add-account <name> <balance>
 *   FinanceApp add-transaction <type> <date> <amount> <category> <account> <desc> [freq] [dueDay]
 *   FinanceApp set-budget <category> <limit>
 *   FinanceApp simulate-recurring <year> <month>
 *   FinanceApp preview-recurring <year> <month>
 *   FinanceApp load-sample
 *   (Ten file tren Windows la FinanceApp.exe.)
 *
 * Moi lenh doc/ghi CSV theo nhu cau va in ket qua JSON ra stdout.
 * Streamlit goi qua subprocess va doc JSON tu stdout.
 */

#include <cstdlib>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#endif
#include "FinanceManager.h"

using namespace std;

static const string DATA_FILE = "data/finance_data.csv";

// In loi dang JSON
static void errJson(const string& message) {
    cout << "{\"ok\":false,\"message\":\"" << message << "\"}" << endl;
}

int main(int argc, char* argv[]) {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);

    // MinGW tao argv theo ANSI; lay command line Unicode de giu nguyen tieng Viet.
    int wideArgc = 0;
    LPWSTR* wideArgv = CommandLineToArgvW(GetCommandLineW(), &wideArgc);
    if (wideArgv == nullptr) {
        cerr << "Khong the doc tham so dong lenh Unicode.\n";
        return 1;
    }
    vector<string> utf8Args;
    utf8Args.reserve(wideArgc);
    for (int i = 0; i < wideArgc; ++i) {
        int length = WideCharToMultiByte(CP_UTF8, 0, wideArgv[i], -1,
                                          nullptr, 0, nullptr, nullptr);
        if (length == 0) {
            LocalFree(wideArgv);
            cerr << "Khong the chuyen tham so sang UTF-8.\n";
            return 1;
        }
        string value(length, '\0');
        if (WideCharToMultiByte(CP_UTF8, 0, wideArgv[i], -1,
                                value.data(), length, nullptr, nullptr) == 0) {
            LocalFree(wideArgv);
            cerr << "Khong the chuyen tham so sang UTF-8.\n";
            return 1;
        }
        value.pop_back();  // Bo byte null cuoi chuoi.
        utf8Args.push_back(value);
    }
    LocalFree(wideArgv);

    vector<char*> utf8Argv;
    utf8Argv.reserve(utf8Args.size());
    for (string& arg : utf8Args) utf8Argv.push_back(arg.data());
    argc = wideArgc;
    argv = utf8Argv.data();
#endif

    // Khong co lenh -> in huong dan
    if (argc < 2) {
        cerr << "Finance App CLI\n"
             << "  get-data\n"
             << "  add-account <name> <balance>\n"
             << "  add-transaction <type> <date> <amount> <category> <account> <desc> [freq] [dueDay]\n"
             << "  set-budget <category> <limit>\n"
             << "  simulate-recurring <year> <month>\n"
             << "  preview-recurring <year> <month>\n"
             << "  load-sample (doc data/finance_data_fixed.csv va ghi de du lieu hien tai)\n";
        return 1;
    }

    string command = argv[1];

    // ──────────────────────────────────────────────
    //  add-account <name> <balance>
    // ──────────────────────────────────────────────
    if (command == "add-account") {
        if (argc < 4) { errJson("Thieu tham so: add-account <name> <balance>"); return 1; }
        string name = argv[2];
        double balance = atof(argv[3]);

        FinanceManager manager;
        manager.loadData(DATA_FILE);
        manager.addAccount(name, balance);
        manager.saveData(DATA_FILE);

        cout << manager.toJson() << endl;
        return 0;
    }

    // ──────────────────────────────────────────────
    //  add-transaction <type> <date> <amount> <category> <account> <desc> [freq] [dueDay]
    // ──────────────────────────────────────────────
    if (command == "add-transaction") {
        if (argc < 9) {
            errJson("Thieu tham so: add-transaction type date amount category account desc [freq] [dueDay]");
            return 1;
        }
        string type     = argv[2];
        string date     = argv[3];
        double amount   = atof(argv[4]);
        string category = argv[5];
        string account  = argv[6];
        string desc     = argv[7];
        string freq     = (argc > 8) ? argv[8] : "Monthly";
        int dueDay      = (argc > 9) ? atoi(argv[9]) : 1;

        FinanceManager manager;
        manager.loadData(DATA_FILE);

        // Kiem tra canh bao ngan sach truoc khi them
        int txYear  = 0;
        int txMonth = 0;
        if ((int)date.size() >= 7) {
            txYear  = atoi(date.substr(0, 4).c_str());
            txMonth = atoi(date.substr(5, 2).c_str());
        }
        string alertJson = "";
        if (type != "Income") {
            alertJson = manager.budgetAlertJson(category, txMonth, txYear, amount);
        }

        int txId = manager.generateNextId();
        bool ok = false;

        if (type == "Income") {
            ok = manager.addTransaction(make_unique<Income>(txId, date, amount, category, account, desc));
        } else if (type == "Expense") {
            ok = manager.addTransaction(make_unique<Expense>(txId, date, amount, category, account, desc));
        } else if (type == "RecurringExpense") {
            ok = manager.addTransaction(make_unique<RecurringExpense>(txId, date, amount, category, account, desc, freq, dueDay));
        } else {
            errJson("Loai giao dich khong hop le: " + type);
            return 1;
        }

        if (!ok) {
            errJson("Giao dich bi tu choi (co the khong du so du hoac tai khoan khong ton tai)");
            return 1;
        }

        manager.saveData(DATA_FILE);

        // Tra ve du lieu moi + canh bao ngan sach
        stringstream result;
        result << "{\"ok\":true,\"budget_alert\":" << (alertJson.empty() ? "null" : alertJson)
               << ",\"data\":" << manager.toJson() << "}";
        cout << result.str() << endl;
        return 0;
    }

    // ──────────────────────────────────────────────
    //  set-budget <category> <limit>
    // ──────────────────────────────────────────────
    if (command == "set-budget") {
        if (argc < 4) { errJson("Thieu tham so: set-budget <category> <limit>"); return 1; }
        string category = argv[2];
        double limit    = atof(argv[3]);

        FinanceManager manager;
        manager.loadData(DATA_FILE);
        manager.setBudgetLimit(category, limit);
        manager.saveData(DATA_FILE);

        cout << manager.toJson() << endl;
        return 0;
    }

    // ──────────────────────────────────────────────
    //  preview-recurring / simulate-recurring <year> <month>
    // ──────────────────────────────────────────────
    if (command == "simulate-recurring" || command == "preview-recurring") {
        if (argc < 4) { errJson("Thieu tham so: simulate-recurring <year> <month>"); return 1; }
        int year  = atoi(argv[2]);
        int month = atoi(argv[3]);
        if (year < 1 || month < 1 || month > 12) {
            errJson("Thang hoac nam khong hop le");
            return 1;
        }

        FinanceManager manager;
        manager.loadData(DATA_FILE);
        int count = manager.executeRecurringExpensesForMonth(year, month);
        if (command == "simulate-recurring" && !manager.saveData(DATA_FILE)) {
            errJson("Khong the luu du lieu mo phong");
            return 1;
        }

        stringstream result;
        result << "{\"ok\":true,\"applied\":" << count
               << ",\"data\":" << manager.toJson() << "}";
        cout << result.str() << endl;
        return 0;
    }

    // Lenh khong nhan ra
    errJson("Lenh khong hop le: " + command);
    return 1;
}
