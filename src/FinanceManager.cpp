#include "FinanceManager.h"
#include "Common.h"
#include <bits/stdc++.h>

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
            cout << "[!] Tai khoan [" << name << "] da ton tai!\n"; 
            return;
        }
    }
    // Them tai khoan moi vao danh sach
    accounts.push_back(Account(name, initialBalance));
    cout << "[+] Da them tai khoan [" << name << "] voi so du: "
         << Utils::formatCurrency(initialBalance) << "\n";
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
            cout << "[!] CANH BAO: Tai khoan [" << acc->getName() << "] khong du so du! (Hien co: " << Utils::formatCurrency(acc->getBalance()) << ", Can: " << Utils::formatCurrency(tx->getAmount()) << ")\n";
            cout << "    Giao dich bi tu choi do khong du tien.\n";
            return false;
        }
        // Buoc 3: Kiem tra han muc ngan sach theo thang
        int txYear = stoi(tx->getDate().substr(0, 4));
        int txMonth = stoi(tx->getDate().substr(5, 2));


        double previousSpent = calculateCategorySpending(tx->getCategory(), txMonth, txYear);
        double newTotalSpent = previousSpent + tx->getAmount();
        string alert = budget.getAlertMessage(tx->getCategory(), newTotalSpent);
        if (!alert.empty()) {
            cout << "\n--------------------------------------------------------------\n";
            cout << alert << "\n";
            cout << "--------------------------------------------------------------\n";
        }
    }
    // Buoc 4: Cap nhat ID va dua vao danh sach giao dich
    if (tx->getId() >= nextId) {
        nextId = tx->getId() + 1;
    }
    cout << "[+] Giao dich da duoc ghi nhan thanh cong! (ID: " << tx->getId() << ")\n";
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
    cout << "[+] Da dat han muc ngan sach cho [" << category << "]: " << Utils::formatCurrency(limit) << " / thang\n";
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

// ================= 4. MO PHONG CHI PHI DINH KY =================
double FinanceManager::calculateMonthlyFixedExpenses() const {
    double total = 0.0;
    vector<string> seen; // Luu cac ten khoan chi de tranh tinh trung
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i]->getType() == "RecurringExpense") {
            string desc = transactions[i]->getDescription();
            // Kiem tra xem da tinh khoan nay chua
            bool daTinh = false;
            for (int j = 0; j < (int)seen.size(); j++) {
                if (seen[j] == desc) {
                    daTinh = true;
                    break;
                }
            }
            if (!daTinh) {
                seen.push_back(desc);
                total += transactions[i]->getAmount();
            }
        }
    }
    return total;
}

void FinanceManager::displayRecurringPlan() const {
    cout << "\n============== DANH SACH CHI PHI DINH KY CO DINH =============\n";
    cout << "+----------------+------------+--------------------+-----------+---------+----------------------+\n";
    cout << "| Danh Muc       | Tai Khoan  | So Tien Dinh Ky    | Tan Suat  | Ngay Tra| Ghi Chu              |\n";
    cout << "+----------------+------------+--------------------+-----------+---------+----------------------+\n";
    vector<string> seen;
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i]->getType() == "RecurringExpense") {
            string desc = transactions[i]->getDescription();
            bool daIn = false;
            for (int j = 0; j < (int)seen.size(); j++) {
                if (seen[j] == desc) {
                    daIn = true;
                    break;
                }
            }
            if (!daIn) {
                seen.push_back(desc);
                RecurringExpense* rec = (RecurringExpense*)transactions[i].get();
                cout << " | " << left << setw(14) << rec->getCategory()
                     << " | " << setw(10) << rec->getAccountName()
                     << " | " << right << setw(18) << Utils::formatCurrency(rec->getAmount())
                     << " | " << left << setw(9) << rec->getFrequency()
                     << " | " << right << setw(7) << ("Ngay " + to_string(rec->getDueDay()))
                     << " | " << left << setw(20) << rec->getDescription()
                     << " |\n";
            }
        }
    }
    cout << "+----------------+------------+--------------------+-----------+---------+----------------------+\n";
    cout << "Tong chi phi co dinh hang thang uoc tinh: "
         << Utils::formatCurrency(calculateMonthlyFixedExpenses()) << "\n";
}

int FinanceManager::executeRecurringExpensesForMonth(int year, int month) {
    vector<string> seen;
    vector<unique_ptr<BaseTransaction>> newTransactions;
    // Loc ra cac khoan dinh ky goc
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i]->getType() == "RecurringExpense") {
            string desc = transactions[i]->getDescription();
            bool daCo = false;
            for (int j = 0; j < (int)seen.size(); j++) {
                if (seen[j] == desc) {
                    daCo = true;
                    break;
                }
            }
            if (!daCo) {
                seen.push_back(desc);
                RecurringExpense* rec = (RecurringExpense*)transactions[i].get();
                // Tao ngay moi: YYYY-MM-DD
                string thangStr = (month < 10 ? "0" : "") + to_string(month);
                string ngayStr = (rec->getDueDay() < 10 ? "0" : "") + to_string(rec->getDueDay());
                string dateStr = to_string(year) + "-" + thangStr + "-" + ngayStr;
                newTransactions.push_back(make_unique<RecurringExpense>(
                    generateNextId(),
                    dateStr,
                    rec->getAmount(),
                    rec->getCategory(),
                    rec->getAccountName(),
                    "[Tu dong] " + rec->getDescription(),
                    rec->getFrequency(),
                    rec->getDueDay()
                ));
            }
        }
    }
    // Ap dung cac giao dich vao tai khoan
    int appliedCount = 0;
    cout << "\n[*] Dang mo phong ap dung chi phi dinh ky cho Thang " << month << "/" << year << ".\n";
    for (int i = 0; i < (int)newTransactions.size(); i++) {
        cout << " -> Xu ly khoan: " << newTransactions[i]->getDescription() << " ("
             << Utils::formatCurrency(newTransactions[i]->getAmount()) << " tu " << newTransactions[i]->getAccountName() << ")\n";
        if (addTransaction(move(newTransactions[i]))) {
            appliedCount++;
        }
    }
    cout << "[+] Mo phong hoan tat! Da ghi nhan " << appliedCount << " khoan chi dinh ky.\n";
    return appliedCount;
}

// ================= 5. LOC VA TIM KIEM GIAO DICH =================
vector<const BaseTransaction*> FinanceManager::filterByDateRange(const string& startDate, const string& endDate) const {
    vector<const BaseTransaction*> result;
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i]->getDate() >= startDate && transactions[i]->getDate() <= endDate) {
            result.push_back(transactions[i].get());
        }
    }
    return result;
}
vector<const BaseTransaction*> FinanceManager::filterByCategory(const string& category) const {
    vector<const BaseTransaction*> result;
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i]->getCategory() == category) {
            result.push_back(transactions[i].get());
        }
    }
    return result;
}
vector<const BaseTransaction*> FinanceManager::filterByAccount(const string& accountName) const {
    vector<const BaseTransaction*> result;
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i]->getAccountName() == accountName) {
            result.push_back(transactions[i].get());
        }
    }
    return result;
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

// ================= 7. XUAT & DOC DU LIEU CSV =================
bool FinanceManager::saveData(const string& filename) {
    if (storage.saveToFile(filename, accounts, transactions, budget)) {
        cout << "[+] Da luu du lieu thanh cong vao: " << filename << "\n";
        return true;
    }
    return false;
}
bool FinanceManager::loadData(const string& filename) {
    if (storage.loadFromFile(filename, accounts, transactions, budget)) {
        // Cap nhat lai nextId bang ID lon nhat da co + 1
        nextId = 1;
        for (int i = 0; i < (int)transactions.size(); i++) {
            if (transactions[i]->getId() >= nextId) {
                nextId = transactions[i]->getId() + 1;
            }
        }
        cout << "[+] Da nap thanh cong du lieu tu: " << filename << "\n";
        cout << "    - So tai khoan: " << accounts.size() << "\n";
        cout << "    - So giao dich: " << transactions.size() << "\n";
        cout << "    - So han muc ngan sach: " << budget.getAllLimits().size() << "\n";
        return true;
    }
    return false;
}
bool FinanceManager::exportQueryResults(const string& filename, const vector<const BaseTransaction*>& list) {
    if (storage.exportTransactionsOnly(filename, list)) {
        cout << "[+] Da xuat " << list.size() << " giao dich ra file CSV: " << filename << "\n";
        return true;
    }
    return false;
}


// ================= Dữ liệu mẫu (Demo Data) =================

void FinanceManager::loadSampleData() {
    accounts.clear();
    transactions.clear();
    budget.clear();
    nextId = 1;

    // 1. Tài khoản (Cash, Bank, E-Wallet)
    accounts.emplace_back("Cash", 1500000.0);
    accounts.emplace_back("Bank", 20000000.0);
    accounts.emplace_back("E-Wallet", 2500000.0);

    // 2. Hạn mức ngân sách (Budget Limits)
    budget.setLimit("An uong", 3000000.0);
    budget.setLimit("Thue nha", 3500000.0);
    budget.setLimit("Giai tri", 1000000.0);
    budget.setLimit("Tien ich", 800000.0);
    budget.setLimit("Hoc tap", 1500000.0);

    // 3. Các giao dịch mẫu
    // Thu nhập
    transactions.push_back(make_unique<Income>(generateNextId(), "2026-09-01", 15000000.0, "Luong", "Bank", "Luong cong ty thang 9"));
    transactions.push_back(make_unique<Income>(generateNextId(), "2026-09-10", 3500000.0, "Freelance", "Bank", "Viet code web freelance"));
    transactions.push_back(make_unique<Income>(generateNextId(), "2026-09-15", 500000.0, "Thuong", "E-Wallet", "Thuong du an mini"));

    // Chi tiêu thông thường
    transactions.push_back(make_unique<Expense>(generateNextId(), "2026-09-02", 75000.0, "An uong", "Cash", "An trua com tam"));
    transactions.push_back(make_unique<Expense>(generateNextId(), "2026-09-03", 250000.0, "An uong", "E-Wallet", "Di sieu thi mua do an"));
    transactions.push_back(make_unique<Expense>(generateNextId(), "2026-09-08", 450000.0, "Giai tri", "E-Wallet", "Xem phim CGV & bap rang"));
    transactions.push_back(make_unique<Expense>(generateNextId(), "2026-09-12", 800000.0, "Hoc tap", "Bank", "Khoa hoc lap trinh C++"));
    transactions.push_back(make_unique<Expense>(generateNextId(), "2026-09-18", 2200000.0, "An uong", "Bank", "Lien hoan sinh nhat ban"));

    // Chi phí định kỳ (RecurringExpense)
    transactions.push_back(make_unique<RecurringExpense>(generateNextId(), "2026-09-05", 3200000.0, "Thue nha", "Bank", "Tien thue tro thang 9", "Monthly", 5));
    transactions.push_back(make_unique<RecurringExpense>(generateNextId(), "2026-09-10", 250000.0, "Tien ich", "Bank", "Tien Internet Viettel", "Monthly", 10));
    transactions.push_back(make_unique<RecurringExpense>(generateNextId(), "2026-09-15", 180000.0, "Giai tri", "E-Wallet", "Goi Netflix hang thang", "Monthly", 15));
    transactions.push_back(make_unique<RecurringExpense>(generateNextId(), "2026-09-01", 350000.0, "Suc khoe", "Cash", "Tien ve thang tap Gym", "Monthly", 1));

    // Thêm các tháng trước để biểu đồ so sánh tháng có dữ liệu đa dạng
    transactions.push_back(make_unique<Expense>(generateNextId(), "2026-07-15", 4200000.0, "An uong", "Bank", "Chi tieu thang 7"));
    transactions.push_back(make_unique<Expense>(generateNextId(), "2026-08-10", 5600000.0, "An uong", "Bank", "Chi tieu thang 8"));

    cout << "[+] Da nap bo du lieu mau day du (Accounts, Budgets, Incomes, Expenses, RecurringExpenses)!\n";
}
