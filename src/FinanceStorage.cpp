#include "FinanceStorage.h"
#include "Common.h"
#include <bits/stdc++.h>

#include <filesystem>

using namespace std;

bool FinanceStorage::saveToFile(const string& filename,
                                const vector<Account>& accounts,
                                const vector<unique_ptr<BaseTransaction>>& transactions,
                                const Budget& budget) {
    try {
        filesystem::path p(filename);
        if (p.has_parent_path()) {
            filesystem::create_directories(p.parent_path());
        }
    } catch (...) {}

    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "[!] Loi: Khong the mo file de ghi: " << filename << "\n";
        return false;
    }

    // 1. Ghi danh sach tai khoan
    outFile << "#ACCOUNTS\n";
    outFile << "Name,Balance\n";
    for (int i = 0; i < (int)accounts.size(); i++) {
        outFile << accounts[i].toCSV() << "\n";
    }

    // 2. Ghi danh sach han muc ngan sach
    outFile << "#BUDGETS\n";
    outFile << "Category,Limit\n";
    const map<string, double>& limits = budget.getAllLimits();
    for (pair<string, double> p : limits) {
        outFile << p.first << "," << fixed << setprecision(2) << p.second << "\n";
    }

    // 3. Ghi danh sach giao dich
    outFile << "#TRANSACTIONS\n";
    outFile << "Type,Id,Date,Amount,Category,Account,Description,Frequency,DueDay\n";
    for (int i = 0; i < (int)transactions.size(); i++) {
        outFile << transactions[i]->toCSV() << "\n";
    }

    outFile.close();
    return true;
}

bool FinanceStorage::loadFromFile(const string& filename,
                                  vector<Account>& accounts,
                                  vector<unique_ptr<BaseTransaction>>& transactions,
                                  Budget& budget) {
    ifstream inFile(filename);
    if (!inFile.is_open()) {
        cerr << "[!] Khong the mo file: " << filename << " (Co the file chua ton tai).\n";
        return false;
    }

    accounts.clear();
    transactions.clear();
    budget.clear();

    enum class Section { NONE, ACCOUNTS, BUDGETS, TRANSACTIONS };
    Section currentSection = Section::NONE;

    string line;
    while (getline(inFile, line)) {
        line = Utils::trim(line);
        if (line.empty()) continue;

        // Nhan dien phan section
        if (line == "#ACCOUNTS") {
            currentSection = Section::ACCOUNTS;
            continue;
        } else if (line == "#BUDGETS") {
            currentSection = Section::BUDGETS;
            continue;
        } else if (line == "#TRANSACTIONS") {
            currentSection = Section::TRANSACTIONS;
            continue;
        }

        // Bo qua dong tieu de cot
        if (line == "Name,Balance" || line == "Category,Limit" || line.substr(0, 4) == "Type") {
            continue;
        }

        vector<string> tokens = Utils::split(line, ',');
        try {
            if (currentSection == Section::ACCOUNTS && tokens.size() >= 2) {
                string accName = tokens[0];
                double accBalance = stod(tokens[1]);
                accounts.push_back(Account(accName, accBalance));
            } else if (currentSection == Section::BUDGETS && tokens.size() >= 2) {
                string category = tokens[0];
                double limit = stod(tokens[1]);
                budget.setLimit(category, limit);
            } else if (currentSection == Section::TRANSACTIONS && tokens.size() >= 7) {
                string type = tokens[0];
                int id = stoi(tokens[1]);
                string date = tokens[2];
                double amount = stod(tokens[3]);
                string category = tokens[4];
                string accountName = tokens[5];
                string desc = tokens[6];

                if (type == "Income") {
                    transactions.push_back(make_unique<Income>(id, date, amount, category, accountName, desc));
                } else if (type == "Expense") {
                    transactions.push_back(make_unique<Expense>(id, date, amount, category, accountName, desc));
                } else if (type == "RecurringExpense") {
                    string freq = (tokens.size() > 7 && !tokens[7].empty()) ? tokens[7] : "Monthly";
                    int due = (tokens.size() > 8 && !tokens[8].empty()) ? stoi(tokens[8]) : 1;
                    transactions.push_back(make_unique<RecurringExpense>(id, date, amount, category, accountName, desc, freq, due));
                }
            }
        } catch (const exception& e) {
            // Bo qua dong loi dinh dang va doc tiep
            cerr << "[!] Canh bao loi doc dong: " << line << " (" << e.what() << ")\n";
        }
    }

    inFile.close();
    return true;
}

bool FinanceStorage::exportTransactionsOnly(const string& filename,
                                            const vector<const BaseTransaction*>& transactions) {
    try {
        filesystem::path p(filename);
        if (p.has_parent_path()) {
            filesystem::create_directories(p.parent_path());
        }
    } catch (...) {}

    ofstream outFile(filename);
    if (!outFile.is_open()) {
        cerr << "[!] Khong the mo file de xuat: " << filename << "\n";
        return false;
    }

    outFile << "Type,Id,Date,Amount,Category,Account,Description,Frequency,DueDay\n";
    for (int i = 0; i < (int)transactions.size(); i++) {
        outFile << transactions[i]->toCSV() << "\n";
    }

    outFile.close();
    return true;
}
