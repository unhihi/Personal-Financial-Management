#include "FinanceManager.h"
#include "Common.h"
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <sstream>
#include <tuple>
#include <utility>

using namespace std;

FinanceManager::FinanceManager() {
    // Khoi tao mac dinh
}
int FinanceManager::generateNextId() {
    return nextId++;
}
// ================= 1. QUAN LY TAI KHOAN =================
void FinanceManager::addAccount(const string& name, double initialBalance) {
    // Kiem tra xem ten tai khoan da ton tai chua
    for (int i = 0; i < (int)accounts.size(); i++) {
        if (accounts[i].getName() == name) {
            return;
        }
    }
    // Them tai khoan moi vao danh sach
    accounts.push_back(Account(name, initialBalance));
}

Account* FinanceManager::findAccount(const string& name) {
    for (int i = 0; i < (int)accounts.size(); i++) {
        if (accounts[i].getName() == name) {
            return &accounts[i];
        }
    }
    return nullptr;
}

const vector<Account>& FinanceManager::getAccounts() const {
    return accounts;
}
double FinanceManager::getTotalBalance() const {
    double total = 0.0;
    for (int i = 0; i < (int)accounts.size(); i++) {
        total += accounts[i].getBalance();
    }
    return total;
}

void FinanceManager::displayAccounts() const {
    cout << "\n================= DANH SACH TAI KHOAN & SO DU ================\n";
    cout << "+-----------------+----------------------+\n";
    cout << "| Ten Tai Khoan   | So Du Hien Co        |\n";
    cout << "+-----------------+----------------------+\n";
    for (int i = 0; i < (int)accounts.size(); i++) {
        accounts[i].display();
    }
    cout << "+-----------------+----------------------+\n";
    cout << "| " << left << setw(15) << "TONG CONG"
         << "| " << right << setw(20) << Utils::formatCurrency(getTotalBalance())
         << "|\n";
    cout << "+-----------------+----------------------+\n";
}


// ================= 2. QUAN LY GIAO DICH =================
bool FinanceManager::addTransaction(unique_ptr<BaseTransaction> tx) {
    // Buoc 1: Tim tai khoan thanh toan
    Account* acc = findAccount(tx->getAccountName());
    if (acc == nullptr) {
        cerr << "[!] Loi: Khong tim thay tai khoan [" << tx->getAccountName() << "]!\n";
        return false;
    }
    // Buoc 2: Xu ly cong hoac tru tien
    if (tx->getType() == "Income") {
        // Thu nhap thi cong them vao tai khoan
        acc->deposit(tx->getAmount());
    } else {
        // Chi tieu thi tru tien, kiem tra so du
        if (!acc->withdraw(tx->getAmount())) {
            return false;
        }
    }
    // Cap nhat ID va dua vao danh sach giao dich
    if (tx->getId() >= nextId) {
        nextId = tx->getId() + 1;
    }
    transactions.push_back(move(tx));
    return true;
}
double FinanceManager::calculateCategorySpending(const string& category, int month, int year) const {
    double total = 0.0;
    for (int i = 0; i < (int)transactions.size(); i++) {
        // Chi tinh cac khoan chi tieu (bo qua thu nhap)
        if (transactions[i]->getType() != "Income" && transactions[i]->getCategory() == category) {
            if (month > 0 && year > 0) {
                int txYear = stoi(transactions[i]->getDate().substr(0, 4));
                int txMonth = stoi(transactions[i]->getDate().substr(5, 2));
                if (txYear == year && txMonth == month) {
                    total += transactions[i]->getAmount();
                }
            } else if (year > 0 && month == 0) {
                int txYear = stoi(transactions[i]->getDate().substr(0, 4));
                if (txYear == year) {
                    total += transactions[i]->getAmount();
                }
            } else {
                total += transactions[i]->getAmount();
            }
        }
    }
    return total;
}
void FinanceManager::displayAllTransactions() const {
    vector<const BaseTransaction*> list;
    for (int i = 0; i < (int)transactions.size(); i++) {
        list.push_back(transactions[i].get());
    }
    printTransactionTable(list);
}

void FinanceManager::printTransactionTable(const vector<const BaseTransaction*>& list) {
    if (list.empty()) {
        cout << "\n[*] Khong co giao dich nao phu hop.\n";
        return;
    }
    cout << "\n+------+------------+------------------+----------------+------------+------------------+----------------------+\n";
    cout << "| ID   | Ngay       | Loai GD          | Danh Muc       | Tai Khoan  | So Tien          | Ghi Chu              |\n";
    cout << "+------+------------+------------------+----------------+------------+------------------+----------------------+\n";
    for (int i = 0; i < (int)list.size(); i++) {
        list[i]->display();
    }
    cout << "+------+------------+------------------+----------------+------------+------------------+----------------------+\n";
    cout << "Tong so giao dich: " << list.size() << "\n";
}

// ================= 3. QUAN LY NGAN SACH =================
void FinanceManager::setBudgetLimit(const string& category, double limit) {
    budget.setLimit(category, limit);
}

const Budget& FinanceManager::getBudget() const {
    return budget;
}

void FinanceManager::displayBudgetReport(int month, int year) const {
    map<string, double> limits = budget.getAllLimits();

    if (limits.empty()) {
        cout << "\n[*] Chua co han muc ngan sach nao duoc thiet lap.\n";
        return;
    }

    int monthsCount = 1;
    string timeTitle = "";

    if (month > 0 && year > 0) {
        monthsCount = 1;
        timeTitle = "Thang " + to_string(month) + "/" + to_string(year);
    } else if (year > 0 && month == 0) {
        monthsCount = 12;
        timeTitle = "Nam " + to_string(year) + " (12 thang)";
    } else {
        // Dem so thang rieng biet trong toan bo giao dich
        vector<string> distinctMonths;
        for (int i = 0; i < (int)transactions.size(); i++) {
            string ym = transactions[i]->getDate().substr(0, 7);
            bool daCo = false;
            for (int j = 0; j < (int)distinctMonths.size(); j++) {
                if (distinctMonths[j] == ym) {
                    daCo = true;
                    break;
                }
            }
            if (!daCo) distinctMonths.push_back(ym);
        }
        monthsCount = distinctMonths.empty() ? 1 : (int)distinctMonths.size();
        timeTitle = "Tat ca thoi gian (" + to_string(monthsCount) + " thang)";
    }

    cout << "\n============= BAO CAO NGAN SACH (" << timeTitle << ") =============\n";
    cout << "+----------------+--------------------+--------------------+--------------------+------------------+\n";
    cout << "| Danh Muc       | Han Muc            | Da Chi             | Con Lai            | Trang Thai       |\n";
    cout << "+----------------+--------------------+--------------------+--------------------+------------------+\n";

    for (pair<string, double> p : limits) {
        string category = p.first;
        double monthlyLimit = p.second;
        double limit = monthlyLimit * monthsCount; // Nhan voi so thang bao cao

        double spent = calculateCategorySpending(category, month, year);
        double remaining = limit - spent;

        double percent = 0;
        if (limit > 0) {
            percent = (spent / limit) * 100;
        }

        string status;
        if (spent > limit) {
            status = "VUOT! (" + to_string((int)percent) + "%)";
        } else if (spent >= limit * 0.8) {
            status = "Canh bao (" + to_string((int)percent) + "%)";
        } else {
            status = "An toan (" + to_string((int)percent) + "%)";
        }

        string remainingStr;
        if (remaining >= 0) {
            remainingStr = Utils::formatCurrency(remaining);
        } else {
            remainingStr = "-" + Utils::formatCurrency(-remaining);
        }

        cout << "| " << left << setw(14) << category
             << "| " << right << setw(18) << Utils::formatCurrency(limit)
             << "| " << setw(18) << Utils::formatCurrency(spent)
             << "| " << setw(18) << remainingStr
             << "| " << left << setw(16) << status
             << "|\n";
    }

    cout << "+----------------+--------------------+--------------------+--------------------+------------------+\n";
    if (monthsCount > 1) {
        cout << "(*) Ghi chu: Han muc da duoc tu dong nhan voi " << monthsCount
             << " thang theo khung thoi gian bao cao.\n";
    }
}

// ================= 6. BIEU DO TRUC QUAN ASCII CHART =================
void FinanceManager::renderMonthlySpendingChart(int year) const {
    double monthlyExpenses[13] = {0.0}; // Mang luu chi tieu 12 thang (chi so 1 -> 12)
    double maxExpense = 0.0;
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i]->getType() != "Income") {
            int txYear = stoi(transactions[i]->getDate().substr(0, 4));
            int txMonth = stoi(transactions[i]->getDate().substr(5, 2));
            if (txYear == year && txMonth >= 1 && txMonth <= 12) {
                monthlyExpenses[txMonth] += transactions[i]->getAmount();
                if (monthlyExpenses[txMonth] > maxExpense) {
                    maxExpense = monthlyExpenses[txMonth];
                }
            }
        }
    }
    cout << "\n================ BIEU DO CHI TIEU THEO THANG (NAM " << year << ") ================\n";
    if (maxExpense == 0.0) {
        cout << "[*] Khong co du lieu chi tieu cho nam " << year << ".\n";
        return;
    }
    int maxBar = 35; // Do dai thanh bar toi da
    for (int m = 1; m <= 12; m++) {
        int barLen = 0;
        if (maxExpense > 0) {
            barLen = (int)((monthlyExpenses[m] / maxExpense) * maxBar);
        }
        string barStr(barLen, '#'); // Tao chuoi bar bang ky tu '#'
        cout << "Thang " << (m < 10 ? "0" : "") << m
             << " [" << right << setw(15) << Utils::formatCurrency(monthlyExpenses[m]) << "] | "
             << barStr << "\n";
    }
    cout << "----------------------------------------------------------------------\n";
    cout << "Ghi chu: Moi ky tu '#' bieu thi khoang "
         << Utils::formatCurrency(maxExpense / maxBar) << " chi tieu.\n";
}

void FinanceManager::renderCategorySpendingChart(int month, int year) const {
    // Buoc 1: Lay danh sach cac danh muc chi tieu khong trung nhau
    vector<string> categories;
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i]->getType() != "Income") {
            string cat = transactions[i]->getCategory();
            bool daCo = false;
            for (int j = 0; j < (int)categories.size(); j++) {
                if (categories[j] == cat) {
                    daCo = true;
                    break;
                }
            }
            if (!daCo) categories.push_back(cat);
        }
    }
    // Buoc 2: Tim muc chi tieu lon nhat de chia ty le
    double maxSpend = 0.0;
    for (int i = 0; i < (int)categories.size(); i++) {
        double spent = calculateCategorySpending(categories[i], month, year);
        if (spent > maxSpend) maxSpend = spent;
    }
    int monthsCount = 1;
    string timeTitle = "";

    if (month > 0 && year > 0) {
        monthsCount = 1;
        timeTitle = "Thang " + to_string(month) + "/" + to_string(year);
    } else if (year > 0 && month == 0) {
        monthsCount = 12;
        timeTitle = "Nam " + to_string(year) + " (12 thang)";
    } else {
        vector<string> distinctMonths;
        for (int i = 0; i < (int)transactions.size(); i++) {
            string ym = transactions[i]->getDate().substr(0, 7);
            bool daCo = false;
            for (int j = 0; j < (int)distinctMonths.size(); j++) {
                if (distinctMonths[j] == ym) {
                    daCo = true;
                    break;
                }
            }
            if (!daCo) distinctMonths.push_back(ym);
        }
        monthsCount = distinctMonths.empty() ? 1 : (int)distinctMonths.size();
        timeTitle = "Tat ca thoi gian (" + to_string(monthsCount) + " thang)";
    }

    cout << "\n============ BIEU DO CHI TIEU THEO DANH MUC (" << timeTitle << ") ============\n";
    if (maxSpend == 0.0) {
        cout << "[*] Khong co du lieu chi tieu phu hop.\n";
        return;
    }
    int maxBar = 30;
    for (int i = 0; i < (int)categories.size(); i++) {
        string cat = categories[i];
        double spent = calculateCategorySpending(cat, month, year);
        int barLen = (int)((spent / maxSpend) * maxBar);
        string barStr(barLen, '#');

        double monthlyLimit = budget.getLimit(cat);
        double limit = monthlyLimit * monthsCount; // Nhan voi so thang
        string budgetInfo = "";
        if (limit > 0) {
            int ratio = (int)((spent / limit) * 100.0);
            budgetInfo = " (" + to_string(ratio) + "% han muc)";
            if (spent > limit) budgetInfo += " [VUOT!]";
        }
        cout << left << setw(15) << cat
             << " [" << right << setw(15) << Utils::formatCurrency(spent) << "] | "
             << left << setw(maxBar + 2) << barStr
             << budgetInfo << "\n";
    }
}

