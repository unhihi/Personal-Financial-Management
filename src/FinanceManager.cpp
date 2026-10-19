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


// ================= 4. BIEU DO TRUC QUAN ASCII CHART =================
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

// ================= 5. MO PHONG CHI PHI DINH KY =================
double FinanceManager::calculateMonthlyFixedExpenses() const {
    double total = 0.0;
    set<tuple<string, string, string, int>> seen;
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i]->getType() == "RecurringExpense") {
            const RecurringExpense* rec = static_cast<const RecurringExpense*>(transactions[i].get());
            if (rec->getFrequency() != "Monthly" ||
                rec->getDescription().rfind("[Tu dong] ", 0) == 0) continue;
            auto key = make_tuple(rec->getCategory(), rec->getAccountName(),
                                  rec->getDescription(), rec->getDueDay());
            if (seen.insert(key).second) total += rec->getAmount();
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
    if (year < 1 || month < 1 || month > 12) return 0;

    const string automaticPrefix = "[Tu dong] ";
    const int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int lastDay = daysInMonth[month - 1];
    if (month == 2 && (year % 400 == 0 || (year % 4 == 0 && year % 100 != 0))) {
        lastDay = 29;
    }

    set<tuple<string, string, string, int>> seen;
    vector<unique_ptr<BaseTransaction>> newTransactions;
    // Chi lay lich hang thang goc; giao dich tu dong khong tro thanh lich moi.
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (transactions[i]->getType() == "RecurringExpense") {
            const RecurringExpense* rec = static_cast<const RecurringExpense*>(transactions[i].get());
            const string desc = rec->getDescription();
            if (rec->getFrequency() != "Monthly" || desc.rfind(automaticPrefix, 0) == 0 ||
                !Utils::isValidDate(rec->getDate()) || rec->getDueDay() < 1 || rec->getDueDay() > 31) continue;

            int startYear = stoi(rec->getDate().substr(0, 4));
            int startMonth = stoi(rec->getDate().substr(5, 2));
            if (year * 12 + month <= startYear * 12 + startMonth) continue;

            auto key = make_tuple(rec->getCategory(), rec->getAccountName(),
                                  desc, rec->getDueDay());
            if (!seen.insert(key).second) continue;

            int dueDay = min(rec->getDueDay(), lastDay);
            string dateStr = to_string(year) + "-" + (month < 10 ? "0" : "") + to_string(month)
                           + "-" + (dueDay < 10 ? "0" : "") + to_string(dueDay);
            string newDescription = automaticPrefix + desc;
            bool alreadyApplied = false;
            for (int j = 0; j < (int)transactions.size(); j++) {
                const BaseTransaction* existing = transactions[j].get();
                if (existing->getType() == "RecurringExpense" && existing->getDate() == dateStr &&
                    existing->getDescription() == newDescription &&
                    existing->getAccountName() == rec->getAccountName() &&
                    existing->getCategory() == rec->getCategory()) {
                    alreadyApplied = true;
                    break;
                }
            }
            if (alreadyApplied) continue;

            newTransactions.push_back(make_unique<RecurringExpense>(
                generateNextId(), dateStr, rec->getAmount(), rec->getCategory(),
                rec->getAccountName(), newDescription, rec->getFrequency(), rec->getDueDay()
            ));
        }
    }
    // Ap dung cac giao dich vao tai khoan
    int appliedCount = 0;
    for (int i = 0; i < (int)newTransactions.size(); i++) {
        if (addTransaction(move(newTransactions[i]))) {
            appliedCount++;
        }
    }
    return appliedCount;
}

// ================= 6. LOC VA TIM KIEM GIAO DICH =================
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

// ================= 7. XUAT & DOC DU LIEU CSV =================
bool FinanceManager::saveData(const string& filename) {
    return storage.saveToFile(filename, accounts, transactions, budget);
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
        return true;
    }
    return false;
}
bool FinanceManager::exportQueryResults(const string& filename, const vector<const BaseTransaction*>& list) {
    return storage.exportTransactionsOnly(filename, list);
}


// ================= Dữ liệu mẫu (Demo Data) =================

bool FinanceManager::loadSampleData() {
    return loadData("data/finance_data_fixed.csv");
}

// ================= JSON EXPORT (dung cho Streamlit UI) =================

// Ham thoat dau ngoac kep trong chuoi JSON
static string jsonEscape(const string& s) {
    string out;
    for (int i = 0; i < (int)s.size(); i++) {
        char c = s[i];
        if (c == '"')  out += "\\\"";
        else if (c == '\\') out += "\\\\";
        else if (c == '\n') out += "\\n";
        else if (c == '\r') out += "\\r";
        else out += c;
    }
    return out;
}

string FinanceManager::toJson() const {
    stringstream ss;
    ss << "{";

    // Accounts
    ss << "\"accounts\":[";
    for (int i = 0; i < (int)accounts.size(); i++) {
        if (i > 0) ss << ",";
        ss << "{\"name\":\"" << jsonEscape(accounts[i].getName()) << "\","
           << "\"balance\":" << accounts[i].getBalance() << "}";
    }
    ss << "],";

    // Budgets
    map<string, double> limits = budget.getAllLimits();
    ss << "\"budgets\":{";
    bool firstB = true;
    for (pair<string, double> p : limits) {
        if (!firstB) ss << ",";
        ss << "\"" << jsonEscape(p.first) << "\":" << p.second;
        firstB = false;
    }
    ss << "},";

    // Transactions
    ss << "\"transactions\":[";
    for (int i = 0; i < (int)transactions.size(); i++) {
        if (i > 0) ss << ",";
        const BaseTransaction* tx = transactions[i].get();
        ss << "{\"type\":\"" << jsonEscape(tx->getType()) << "\","
           << "\"id\":"      << tx->getId()        << ","
           << "\"date\":\""  << jsonEscape(tx->getDate())        << "\","
           << "\"amount\":"  << tx->getAmount()     << ","
           << "\"category\":\"" << jsonEscape(tx->getCategory()) << "\","
           << "\"account\":\"" << jsonEscape(tx->getAccountName()) << "\","
           << "\"description\":\"" << jsonEscape(tx->getDescription()) << "\"";

        // Them frequency va due_day neu la RecurringExpense
        if (tx->getType() == "RecurringExpense") {
            RecurringExpense* rec = (RecurringExpense*)tx;
            ss << ",\"frequency\":\"" << jsonEscape(rec->getFrequency()) << "\","
               << "\"due_day\":" << rec->getDueDay();
        } else {
            ss << ",\"frequency\":\"\",\"due_day\":1";
        }
        ss << "}";
    }
    ss << "]";

    ss << "}";
    return ss.str();
}

string FinanceManager::budgetAlertJson(const string& category, int month, int year, double newAmount) const {
    double spent = calculateCategorySpending(category, month, year) + newAmount;
    double limit = budget.getLimit(category);

    stringstream ss;
    ss << "{";
    ss << "\"category\":\"" << jsonEscape(category) << "\",";
    ss << "\"spent\":"      << spent << ",";
    ss << "\"limit\":"      << limit << ",";
    if (limit > 0) {
        int pct = (int)((spent / limit) * 100);
        ss << "\"percent\":"   << pct << ",";
        if (spent > limit)
            ss << "\"status\":\"over\"";
        else if (pct >= 80)
            ss << "\"status\":\"warning\"";
        else
            ss << "\"status\":\"ok\"";
    } else {
        ss << "\"percent\":0,\"status\":\"ok\"";
    }
    ss << "}";
    return ss.str();
}