#include <iostream>
#include <cassert>
#include <vector>
#include <memory>
#include <sstream>
#include <fstream>
#include <filesystem>
#include "Account.h"
#include "Transaction.h"
#include "Budget.h"
#include "FinanceStorage.h"
#include "FinanceManager.h"
#include "Common.h"

using namespace std;

void testRq1_TransactionTypes() {
    cout << "[TEST 1] Kiem tra Transaction Types\n";
    unique_ptr<BaseTransaction> inc = make_unique<Income>(1, "2026-09-01", 10000000.0, "Luong", "Bank", "Luong thang 9");
    unique_ptr<BaseTransaction> exp = make_unique<Expense>(2, "2026-09-02", 50000.0, "An uong", "Cash", "An com");
    unique_ptr<BaseTransaction> rec = make_unique<RecurringExpense>(3, "2026-09-05", 2500000.0, "Thue nha", "Bank", "Tien phong tro", "Monthly", 5);

    assert(inc->getType() == "Income");
    assert(exp->getType() == "Expense");
    assert(rec->getType() == "RecurringExpense");

    RecurringExpense* recPtr = (RecurringExpense*)rec.get();
    assert(recPtr != nullptr);
    assert(recPtr->getFrequency() == "Monthly");
    assert(recPtr->getDueDay() == 5);

    cout << " PASSED: BaseTransaction va cac lop con ok.\n";
}

void testRq2_MultiAccountBalances() {
    cout << "[TEST 2] Kiem tra Multi-Account Balances (Cash, Bank, E-Wallet)\n";
    Account cash("Cash", 500000.0);
    Account bank("Bank", 10000000.0);
    Account wallet("E-Wallet", 1000000.0);

    cash.deposit(200000.0);
    assert(cash.getBalance() == 700000.0);

    bool ok = bank.withdraw(3000000.0);
    assert(ok == true);
    assert(bank.getBalance() == 7000000.0);

    bool fail = wallet.withdraw(2000000.0); // Rut qua so du
    assert(fail == false);
    assert(wallet.getBalance() == 1000000.0);

    cout << " PASSED: Account nap rut va quan ly so du ok.\n";
}

void testRq3_BudgetAndAlerts() {
    cout << "[TEST 3] Kiem tra Budget Limits & Over-budget Alerts\n";
    Budget b;
    b.setLimit("An uong", 2000000.0);

    // An toàn (< 80%)
    assert(b.checkStatus("An uong", 1000000.0) == BudgetStatus::SAFE);

    // Cảnh báo (>= 80% va <= 100%)
    assert(b.checkStatus("An uong", 1700000.0) == BudgetStatus::WARNING);

    // Vượt ngân sách (> 100%)
    assert(b.checkStatus("An uong", 2100000.0) == BudgetStatus::OVERBUDGET);

    string alertMsg = b.getAlertMessage("An uong", 2200000.0);
    assert(alertMsg.find("VUOT NGAN SACH") != string::npos);

    cout << " PASSED: Ngan sach canh bao ok.\n";
}

void testRq4_ASCIIChart() {
    cout << "[TEST 4] Kiem tra ASCII Chart.\n";
    FinanceManager fm;
    assert(fm.loadSampleData());
    assert(fm.getAccounts().size() == 3);

    // Goi render bieu do truc quan
    cout << "\n--- ASCII Monthly Chart ---\n";
    fm.renderMonthlySpendingChart(2026);

    cout << "\n--- ASCII Category Chart ---\n";
    fm.renderCategorySpendingChart(9, 2026);

    cout << " PASSED: ASCII ok.\n";
}

void testRq5_RecurringScheduler() {
    cout << "[TEST 5] Kiem tra Recurring Transaction Scheduler Simulation.\n";
    FinanceManager fm;
    fm.addAccount("Bank", 10000000.0);
    fm.addAccount("Cash", 1000000.0);

    // Them cac khoan chi phi dinh ky
    fm.addTransaction(make_unique<RecurringExpense>(fm.generateNextId(), "2026-09-05", 2500000.0, "Thue nha", "Bank", "Tien phong tro", "Monthly", 5));
    fm.addTransaction(make_unique<RecurringExpense>(fm.generateNextId(), "2026-09-10", 250000.0, "Internet", "Bank", "Wifi FPT", "Monthly", 10));

    double fixedMonthly = fm.calculateMonthlyFixedExpenses();
    assert(fixedMonthly == 2750000.0);

    // Mo phong thuc thi cho thang 10
    int count = fm.executeRecurringExpensesForMonth(2026, 10);
    assert(count == 2);

    // Kiem tra so du tai khoan Bank da bi tru: 10,000,000 - 2,750,000 (thang 9) - 2,750,000 (mo phong thang 10) = 4,500,000
    const Account* acc = fm.findAccount("Bank");
    assert(acc != nullptr);
    assert(acc->getBalance() == 4500000.0);

    // Chay lai cung ky khong duoc ghi trung hay tru tien them.
    assert(fm.executeRecurringExpensesForMonth(2026, 10) == 0);
    assert(fm.findAccount("Bank")->getBalance() == 4500000.0);

    FinanceManager edgeCases;
    edgeCases.addAccount("Bank", 1000000.0);
    edgeCases.addTransaction(make_unique<RecurringExpense>(1, "2026-01-31", 100000.0,
        "Tien nha", "Bank", "Ngay 31", "Monthly", 31));
    edgeCases.addTransaction(make_unique<RecurringExpense>(2, "2026-01-10", 100000.0,
        "Bao hiem", "Bank", "Hang nam", "Yearly", 10));
    assert(edgeCases.executeRecurringExpensesForMonth(2026, 2) == 1);
    assert(edgeCases.filterByDateRange("2026-02-28", "2026-02-28").size() == 1);

    cout << " PASSED: Tinh Recurr_expense va Simulate ok.\n";
}


void testRq6_FilteringAndSearching() {
    cout << "[TEST 6] Kiem tra Filtering & Searching query methods.\n";
    FinanceManager fm;
    fm.addAccount("Bank", 20000000.0);
    fm.addAccount("Cash", 5000000.0);

    fm.addTransaction(make_unique<Income>(fm.generateNextId(), "2026-08-01", 10000000.0, "Luong", "Bank", "Luong T8"));
    fm.addTransaction(make_unique<Expense>(fm.generateNextId(), "2026-09-02", 150000.0, "An uong", "Cash", "Cafe & An sang"));
    fm.addTransaction(make_unique<Expense>(fm.generateNextId(), "2026-09-15", 300000.0, "Giai tri", "Bank", "Mua sach"));
    fm.addTransaction(make_unique<Expense>(fm.generateNextId(), "2026-10-01", 500000.0, "An uong", "Cash", "Tiec BBQ"));

    // Loc theo khoang thoi gian thang 9
    vector<const BaseTransaction*> resDate = fm.filterByDateRange("2026-09-01", "2026-09-30");
    assert(resDate.size() == 2);

    // Loc theo category "An uong"
    vector<const BaseTransaction*> resCat = fm.filterByCategory("An uong");
    assert(resCat.size() == 2);

    // Loc theo tai khoan "Cash"
    vector<const BaseTransaction*> resAcc = fm.filterByAccount("Cash");
    assert(resAcc.size() == 2);

    cout << " PASSED: Cac phuong thuc truy van loc giao dich ok.\n";
}


int main() {
    cout << "   RUNNING AUTOMATED TEST SUITE FOR FINANCE SYSTEM      \n";

    testRq1_TransactionTypes();
    testRq2_MultiAccountBalances();
    testRq3_BudgetAndAlerts();
    testRq6_ASCIIChart();
    testRq4_RecurringScheduler();

    cout << "\n[PASS TEST 100%!]\n";
    return 0;
}
