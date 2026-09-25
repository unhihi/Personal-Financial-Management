#pragma once

#include <bits/stdc++.h>
using namespace std;

/* Class cơ sở cho tất cả các loại giao dịch
 * Yêu cầu #1: BaseTransaction class with derived specific classes
 */
class BaseTransaction {
protected:
    int id;               // Mã định danh giao dịch
    string date;          // Ngày giao dịch (YYYY-MM-DD)
    double amount;        // Số tiền
    string category;      // Danh mục (Lương, Ăn uống, Thuê nhà,...)
    string accountName;   // Tài khoản sử dụng (Cash, Bank, E-Wallet,...)
    string description;   // Note

public:
    BaseTransaction(int Id, const string& Date, double Amount,
                    const string& Category, const string& Account,
                    const string& Desc);
    virtual ~BaseTransaction() = default;

    // Getters
    int getId() const;
    string getDate() const;
    double getAmount() const;
    string getCategory() const;
    string getAccountName() const;
    string getDescription() const;

    // Setter
    void setId(int newId);

    // Phương thức thuần ảo (Pure Virtual) xác định loại giao dịch
    virtual string getType() const = 0;

    // Hiển thị thông tin giao dịch ra màn hình
    virtual void display() const = 0;

    // Chuyển đổi thành dòng dữ liệu CSV
    virtual string toCSV() const = 0;
};

/*
 * Class giao dịch Thu nhập (Income)
 * Kế thừa BaseTransaction
 */
class Income : public BaseTransaction {
public:
    Income(int Id, const string& Date, double Amount,
           const string& Category, const string& Account,
           const string& Desc);

    string getType() const override;
    void display() const override;
    string toCSV() const override;
};

/**
 * Class giao dịch Chi tiêu (Expense)
 * Kế thừa BaseTransaction
 */
class Expense : public BaseTransaction {
public:
    Expense(int Id, const string& Date, double Amount,
            const string& Category, const string& Account,
            const string& Desc);

    string getType() const override;
    void display() const override;
    string toCSV() const override;
};

/**
 * Class giao dịch Chi phí định kỳ (RecurringExpense)
 * Kế thừa Expense
 * Có tần suất ( Monthly/Weekly) và ngày đến hạn (dueDay)
 */
class RecurringExpense : public Expense {
private:
    string frequency;   // Tần suất: "Monthly", "Weekly", "Yearly"
    int dueDay;         // Ngày đến hạn trong tháng (1 - 31)

public:
    RecurringExpense(int Id, const string& Date, double Amount,
                     const string& Category, const string& Account,
                     const string& Desc,
                     const string& freq = "Monthly", int due = 1);

    string getFrequency() const;
    int getDueDay() const;

    string getType() const override;
    void display() const override;
    string toCSV() const override;
};
