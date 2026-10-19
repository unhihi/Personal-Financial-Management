#include "FinanceStorage.h"
#include "Common.h"
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>

using namespace std;

// Giu nguyen chuoi UTF-8 hop le, chi doi byte Windows-1252 cu bi loi.
static void appendUtf8(string& output, unsigned int codePoint) {
    if (codePoint < 0x80) {
        output += static_cast<char>(codePoint);
    } else if (codePoint < 0x800) {
        output += static_cast<char>(0xC0 | (codePoint >> 6));
        output += static_cast<char>(0x80 | (codePoint & 0x3F));
    } else {
        output += static_cast<char>(0xE0 | (codePoint >> 12));
        output += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        output += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
}

static unsigned int windows1252CodePoint(unsigned char byte) {
    static const unsigned int special[] = {
        0x20AC, 0xFFFD, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
        0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0xFFFD, 0x017D, 0xFFFD,
        0xFFFD, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
        0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0xFFFD, 0x017E, 0x0178,
    };
    return byte < 0xA0 ? special[byte - 0x80] : byte;
}

static string normalizeLegacyBytes(const string& text) {
    string output;
    for (size_t i = 0; i < text.size();) {
        unsigned char first = static_cast<unsigned char>(text[i]);
        if (first < 0x80) {
            output += text[i++];
            continue;
        }

        size_t length = 0;
        if (first >= 0xC2 && first <= 0xDF) length = 2;
        else if (first >= 0xE0 && first <= 0xEF) length = 3;
        else if (first >= 0xF0 && first <= 0xF4) length = 4;

        bool valid = length != 0 && i + length <= text.size();
        for (size_t j = 1; valid && j < length; ++j) {
            unsigned char next = static_cast<unsigned char>(text[i + j]);
            valid = (next & 0xC0) == 0x80;
        }
        if (valid) {
            unsigned char second = static_cast<unsigned char>(text[i + 1]);
            valid = !(first == 0xE0 && second < 0xA0) &&
                    !(first == 0xED && second > 0x9F) &&
                    !(first == 0xF0 && second < 0x90) &&
                    !(first == 0xF4 && second > 0x8F);
        }
        if (valid) {
            output.append(text, i, length);
            i += length;
        } else {
            appendUtf8(output, windows1252CodePoint(first));
            ++i;
        }
    }
    return output;
}

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
        line = Utils::trim(normalizeLegacyBytes(line));
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
