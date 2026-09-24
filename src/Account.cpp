#include "Account.h"
#include "Common.h"
#include <bits/stdc++.h>

using namespace std;

Account::Account(const string& Name, double money)
    : name(Name), balance(money) {}

string Account::getName() const {
    return name;
}

double Account::getBalance() const {
    return balance;
}

void Account::deposit(double amount) {
    if (amount > 0) {
        balance += amount;
    }
}

bool Account::withdraw(double amount) {
    if (amount <= 0) return false;
    if (balance >= amount) {
        balance -= amount;
        return true;
    }
    // Không đủ số dư
    return false;
}

void Account::display() const {
    cout << " | " << left << setw(15) << name
         << " | " << right << setw(20) << Utils::formatCurrency(balance)
         << " |\n";
}

string Account::toCSV() const {
    stringstream ss;
    ss << name << "," << fixed << setprecision(2) << balance;
    return ss.str();
}
