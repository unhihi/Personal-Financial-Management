#pragma once

#include <bits/stdc++.h>
#include "Account.h"
#include "Transaction.h"
#include "Budget.h"

using namespace std;

/* Class xử lý lưu và nạp dữ liệu từ file CSV
 * Yêu cầu #7: CSV Export & Data Load (File stream parsing via FinanceStorage class)
 */
class FinanceStorage {
public:
    FinanceStorage() = default;

    // Lưu toàn bộ trạng thái tài khoản, ngân sách và lịch sử giao dịch vào file CSV
    bool saveToFile(const string& filename,
                    const vector<Account>& accounts,
                    const vector<unique_ptr<BaseTransaction>>& transactions,
                    const Budget& budget);

    // Đọc và nạp dữ liệu từ file CSV vào bộ nhớ
    bool loadFromFile(const string& filename,
                      vector<Account>& accounts,
                      vector<unique_ptr<BaseTransaction>>& transactions,
                      Budget& budget);

    // Xuất danh sách giao dịch (hoặc kết quả tìm kiếm/lọc) ra file CSV
    bool exportTransactionsOnly(const string& filename,
                                const vector<const BaseTransaction*>& transactions);
};
