#pragma once

#include <memory>
#include <string>
#include <vector>
#include "Account.h"
#include "Transaction.h"
#include "Budget.h"
#include "FinanceStorage.h"

using namespace std;

/* Class quan ly he thong tai chinh ca nhan
 * Ket noi Tai khoan, Giao dich, Ngan sach va Luu tru file
 */
class FinanceManager {
private:
    vector<Account> accounts;                          // Danh sach cac tai khoan (Cash, Bank, E-Wallet)
    vector<unique_ptr<BaseTransaction>> transactions;  // Danh sach cac giao dich
    Budget budget;                                     // Quan ly han muc ngan sach
    FinanceStorage storage;                            // Xu ly doc ghi file CSV
    int nextId = 1;                                    // ID tu tang cho giao dich 

public: //Hàm - hành vi
    FinanceManager();

    // 1. Quan ly tai khoan (Multi-Account)
    void addAccount(const string& name, double initialBalance);
    Account* findAccount(const string& name);
    const vector<Account>& getAccounts() const;
    void displayAccounts() const;
    double getTotalBalance() const;

    // 2. Quan ly giao dich (Transaction Types)
    bool addTransaction(unique_ptr<BaseTransaction> tx);
    int generateNextId();
    void displayAllTransactions() const;
    double calculateCategorySpending(const string& category, int month = 0, int year = 0) const;
    static void printTransactionTable(const vector<const BaseTransaction*>& list);

    // 3. Quan ly ngan sach & canh bao (Budget Limits & Alerts)
    void setBudgetLimit(const string& category, double limit);
    void displayBudgetReport(int month = 0, int year = 0) const;
    const Budget& getBudget() const;
};
