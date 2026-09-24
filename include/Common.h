#pragma once
#include <bits/stdc++.h>

using namespace std;

namespace Utils {

    // Dinh dang tien (VD: 1000000 -> "1,000,000 VND")
    inline string formatCurrency(double amount) {
        stringstream ss;
        ss << fixed << setprecision(0) << amount;
        string str = ss.str();
        
        int n = str.length();
        int insertPos = n - 3;
        while (insertPos > 0) {
            str.insert(insertPos, ",");
            insertPos -= 3;
        }
        return str + " VND";
    }

    // Cat bo khoang trang thua o 2 dau chuoi (dung vong lap co ban)
    inline string trim(const string& str) {
        int start = 0;
        while (start < (int)str.length() && (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n')) {
            start++;
        }
        int end = (int)str.length() - 1;
        while (end >= start && (str[end] == ' ' || str[end] == '\t' || str[end] == '\r' || str[end] == '\n')) {
            end--;
        }
        if (start > end) return "";
        return str.substr(start, end - start + 1);
    }

    // Tach chuoi theo dau phan cach bang vong lap ky tu co ban
    inline vector<string> split(const string& s, char delimiter) {
        vector<string> tokens;
        string current = "";
        for (int i = 0; i < (int)s.length(); i++) {
            if (s[i] == delimiter) {
                tokens.push_back(trim(current));
                current = "";
            } else {
                current += s[i];
            }
        }
        tokens.push_back(trim(current));
        return tokens;
    }

    // Kiem tra dinh dang ngay co ban YYYY-MM-DD
    inline bool isValidDate(const string& date) {
        if (date.length() != 10) return false;
        if (date[4] != '-' || date[7] != '-') return false;
        for (int i = 0; i < 10; ++i) {
            if (i == 4 || i == 7) continue;
            if (date[i] < '0' || date[i] > '9') return false;
        }
        return true;
    }

    // Trich xuat thang va nam tu ngay (YYYY-MM-DD)
    inline void parseDate(const string& date, int& year, int& month, int& day) {
        if (!isValidDate(date)) {
            year = month = day = 0;
            return;
        }
        year = stoi(date.substr(0, 4));
        month = stoi(date.substr(5, 2));
        day = stoi(date.substr(8, 2));
    }

}