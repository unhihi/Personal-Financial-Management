#pragma once

#include <bits/stdc++.h>
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

public:
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

    // 3. Quan ly ngan sach & canh bao (Budget Limits & Alerts)
    void setBudgetLimit(const string& category, double limit);
    void displayBudgetReport(int month = 0, int year = 0) const;
    const Budget& getBudget() const;

    // 4. Mo phong chi phi dinh ky (Recurring Scheduler Simulation)
    double calculateMonthlyFixedExpenses() const;
    void displayRecurringPlan() const;
    int executeRecurringExpensesForMonth(int year, int month);

    // 5. Loc va tim kiem giao dich (Filtering & Searching)
    vector<const BaseTransaction*> filterByDateRange(const string& startDate, const string& endDate) const;
    vector<const BaseTransaction*> filterByCategory(const string& category) const;
    vector<const BaseTransaction*> filterByAccount(const string& accountName) const;
    static void printTransactionTable(const vector<const BaseTransaction*>& list);

    // 6. ASCII Chart
    void renderMonthlySpendingChart(int year) const;
    void renderCategorySpendingChart(int month = 0, int year = 0) const;

    // 7. Xuat & nap file CSV (Storage)
    bool saveData(const string& filename = "data/finance_data.csv");
    bool loadData(const string& filename = "data/finance_data.csv");
    bool exportQueryResults(const string& filename, const vector<const BaseTransaction*>& list);

    // Nap du lieu mau ban dau
    void loadSampleData();
};
