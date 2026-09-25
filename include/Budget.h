#pragma once
#include <bits/stdc++.h>
using namespace std;

//Kết quả kiểm tra CurrentSpend so với Budget
enum class BudgetStatus {
    SAFE,       // Dưới 80% hạn mức: An toàn
    WARNING,    // Từ 80% đến 100% hạn mức: Cảnh báo
    OVERBUDGET  // Lớn hơn 100% hạn mức: Vượt ngân sách
};

/* Class quản lý hạn mức ngân sách theo từng danh mục chi tiêu
 * Yêu cầu #3: Category Budget Limits & Over-budget Alerts
 */
class Budget {
private:
    // Ánh xạ Tên danh mục (Category) -> Hạn mức chi tiêu tối đa (Limit)
    map<string, double> categoryLimits;

public:
    Budget() = default;

    // Thiết lập hạn mức ngân sách cho một danh mục
    void setLimit(const string& category, double limit);

    // Lấy hạn mức của danh mục
    double getLimit(const string& category) const;

    // Kiểm tra xem danh mục đã có đặt hạn mức chưa
    bool hasLimit(const string& category) const;

    // Kiểm tra tình trạng ngân sách khi biết số tiền đã chi
    BudgetStatus checkStatus(const string& category, double currentSpent) const;

    // Trả về thông báo cảnh báo chi tiết
    string getAlertMessage(const string& category, double currentSpent) const;

    // Lấy danh sách toàn bộ hạn mức
    const map<string, double>& getAllLimits() const;

    // Xóa toàn bộ hạn mức
    void clear();
};
