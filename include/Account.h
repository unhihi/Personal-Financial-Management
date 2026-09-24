#pragma once
#include <bits/stdc++.h>
using namespace std;

/* Class quản lý tài khoản tài chính (Tiền mặt Cash, Ngân hàng Bank, Ví điện tử E-Wallet)
 * Yêu cầu #2: Multi-Account Balances
 */
class Account {
private:
    string name;       // Tên tài khoản (Cash, Bank, MoMo, ZaloPay,...)
    double balance;    // Số dư hiện tại của tài khoản

public:
    // Hàm khởi tạo mặc định và có tham số
    Account(const string& accountName = "", double initialBalance = 0.0);

    // Getters
    string getName() const;
    double getBalance() const;

    // Nạp tiền vào tài khoản
    void deposit(double amount);

    // Rút tiền/Chi tiêu từ tài khoản
    // Trả về true nếu đủ số dư, false nếu không đủ
    bool withdraw(double amount);

    // Hiển thị thông tin tài khoản
    void display() const;

    // Chuyển đối tượng thành dòng CSV
    string toCSV() const;
};
