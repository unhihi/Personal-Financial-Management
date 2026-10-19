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

