#include "Budget.h"
#include "Common.h"
#include <bits/stdc++.h>

using namespace std;

void Budget::setLimit(const string& category, double limit) {
    if (limit > 0) {
        categoryLimits[category] = limit;
    }
}

double Budget::getLimit(const string& category) const {
    for (pair<string, double> p : categoryLimits) {
        if (p.first == category) {
            return p.second;
        }
    }
    return 0.0;
}

bool Budget::hasLimit(const string& category) const {
    for (pair<string, double> p : categoryLimits) {
        if (p.first == category) {
            return true;
        }
    }
    return false;
}

BudgetStatus Budget::checkStatus(const string& category, double currentSpent) const {
    double limit = getLimit(category);
    if (limit <= 0.0) return BudgetStatus::SAFE;

    if (currentSpent > limit) {
        return BudgetStatus::OVERBUDGET;
    } else if (currentSpent >= 0.8 * limit) {
        return BudgetStatus::WARNING;
    }
    return BudgetStatus::SAFE;
}

string Budget::getAlertMessage(const string& category, double currentSpent) const {
    double limit = getLimit(category);
    if (limit <= 0.0) return "";

    double ratio = (currentSpent / limit) * 100.0;
    stringstream ss;

    if (currentSpent > limit) {
        ss << " CANH BAO NGUY HIEM: Ban da VUOT NGAN SACH danh muc [" << category << "]!\n"
           << "    Da chi: " << Utils::formatCurrency(currentSpent)
           << " / Han muc: " << Utils::formatCurrency(limit)
           << " (Vuot " << Utils::formatCurrency(currentSpent - limit) << " - " << fixed << setprecision(1) << ratio << "%)";
    } else if (currentSpent >= 0.8 * limit) {
        ss << " CANH BAO: Chi tieu danh muc [" << category << "] sap cham han muc toi da!\n"
           << "    Da chi: " << Utils::formatCurrency(currentSpent)
           << " / Han muc: " << Utils::formatCurrency(limit)
           << " (" << fixed << setprecision(1) << ratio << "%)";
    }

    return ss.str();
}

const map<string, double>& Budget::getAllLimits() const {
    return categoryLimits;
}

void Budget::clear() {
    categoryLimits.clear();
}
