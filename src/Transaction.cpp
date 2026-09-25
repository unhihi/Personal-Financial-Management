#include "Transaction.h"
#include "Common.h"
#include <bits/stdc++.h>

using namespace std;

// ================= BaseTransaction Implementation =================

BaseTransaction::BaseTransaction(int txId, const string& txDate, double txAmount,
                                 const string& txCategory, const string& txAccount,
                                 const string& txDesc)
    : id(txId), date(txDate), amount(txAmount), category(txCategory),
      accountName(txAccount), description(txDesc) {}

int BaseTransaction::getId() const { return id; }
string BaseTransaction::getDate() const { return date; }
double BaseTransaction::getAmount() const { return amount; }
string BaseTransaction::getCategory() const { return category; }
string BaseTransaction::getAccountName() const { return accountName; }
string BaseTransaction::getDescription() const { return description; }

void BaseTransaction::setId(int newId) { id = newId; }

// ================= Income Implementation =================

Income::Income(int txId, const string& txDate, double txAmount,
               const string& txCategory, const string& txAccount,
               const string& txDesc)
    : BaseTransaction(txId, txDate, txAmount, txCategory, txAccount, txDesc) {}

string Income::getType() const {
    return "Income";
}

void Income::display() const {
    cout << "| " << setw(4) << id
         << " | " << setw(10) << date
         << " | " << left << setw(16) << "[+] " + getType()
         << " | " << setw(14) << category
         << " | " << setw(10) << accountName
         << " | " << right << setw(16) << ("+" + Utils::formatCurrency(amount))
         << " | " << left << setw(20) << description
         << " |\n";
}

string Income::toCSV() const {
    stringstream ss;
    ss << getType() << "," << id << "," << date << ","
       << fixed << setprecision(2) << amount << ","
       << category << "," << accountName << "," << description << ",,";
    return ss.str();
}

// ================= Expense Implementation =================

Expense::Expense(int txId, const string& txDate, double txAmount,
                 const string& txCategory, const string& txAccount,
                 const string& txDesc)
    : BaseTransaction(txId, txDate, txAmount, txCategory, txAccount, txDesc) {}

string Expense::getType() const {
    return "Expense";
}

void Expense::display() const {
    cout << "| " << setw(4) << id
         << " | " << setw(10) << date
         << " | " << left << setw(16) << "[-] " + getType()
         << " | " << setw(14) << category
         << " | " << setw(10) << accountName
         << " | " << right << setw(16) << ("-" + Utils::formatCurrency(amount))
         << " | " << left << setw(20) << description
         << " |\n";
}

string Expense::toCSV() const {
    stringstream ss;
    ss << getType() << "," << id << "," << date << ","
       << fixed << setprecision(2) << amount << ","
       << category << "," << accountName << "," << description << ",,";
    return ss.str();
}

// ================= RecurringExpense Implementation =================

RecurringExpense::RecurringExpense(int txId, const string& txDate, double txAmount,
                                   const string& txCategory, const string& txAccount,
                                   const string& txDesc,
                                   const string& freq, int due)
    : Expense(txId, txDate, txAmount, txCategory, txAccount, txDesc),
      frequency(freq), dueDay(due) {}

string RecurringExpense::getFrequency() const {
    return frequency;
}

int RecurringExpense::getDueDay() const {
    return dueDay;
}

string RecurringExpense::getType() const {
    return "RecurringExpense";
}

void RecurringExpense::display() const {
    string typeInfo = "[R] " + frequency + " (Day " + to_string(dueDay) + ")";
    cout << "| " << setw(4) << id
         << " | " << setw(10) << date
         << " | " << left << setw(16) << typeInfo
         << " | " << setw(14) << category
         << " | " << setw(10) << accountName
         << " | " << right << setw(16) << ("-" + Utils::formatCurrency(amount))
         << " | " << left << setw(20) << description
         << " |\n";
}

string RecurringExpense::toCSV() const {
    stringstream ss;
    ss << getType() << "," << id << "," << date << ","
       << fixed << setprecision(2) << amount << ","
       << category << "," << accountName << "," << description << ","
       << frequency << "," << dueDay;
    return ss.str();
}
