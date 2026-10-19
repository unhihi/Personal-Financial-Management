"""
app.py  –  Streamlit UI for Personal Finance Management System
Chay: streamlit run app.py
Logic nghiep vu 100% do C++ xu ly (qua subprocess).
Streamlit chi hien thi ket qua JSON tu C++.
"""

import streamlit as st
import pandas as pd
import plotly.graph_objects as go
import hashlib
from datetime import date
from html import escape
from pathlib import Path

from finance_data import (
    get_data, add_account, add_transaction,
    set_budget, simulate_recurring, preview_recurring, load_sample,
    validate_finance_csv, import_finance_csv,
    format_vnd, BUILD_COMMAND,
)

# ══════════════════════════════════════════════════════════════════
#  CAI DAT TRANG
# ══════════════════════════════════════════════════════════════════
st.set_page_config(
    page_title="Tài chính cá nhân",
    page_icon="$",
    layout="wide",
    initial_sidebar_state="auto",
)

st.markdown(
    f"<style>{Path(__file__).with_name('style.css').read_text(encoding='utf-8')}</style>",
    unsafe_allow_html=True,
)


def page_header(title, subtitle):
    """Hien thi tieu de va tra ve vi tri cho hanh dong nhanh."""
    left, right = st.columns([3, 1.35], gap="medium")
    with left:
        st.markdown(
            f'<div class="page-heading"><h1>{escape(title)}</h1>'
            f'<p>{escape(subtitle)}</p></div>',
            unsafe_allow_html=True,
        )
    with right:
        period, action = st.columns([1, 1.25], gap="small")
        with period:
            st.markdown(
                f'<div class="header-period">Tháng {date.today().month}, '
                f'{date.today().year} ⌄</div>',
                unsafe_allow_html=True,
            )
    return action


def money(amount):
    """Dinh dang tien gon nhu tren ban thiet ke."""
    return format_vnd(amount).replace(" VND", " ₫")


def metric_card(label, value, note=""):
    st.markdown(
        '<div class="metric-card">'
        f'<span>{escape(str(label))}</span>'
        f'<strong>{escape(str(value))}</strong>'
        f'<small>{escape(str(note))}</small>'
        '</div>',
        unsafe_allow_html=True,
    )


def header_link(label, anchor):
    st.markdown(
        f'<a class="header-action" href="#{escape(anchor)}">{escape(label)}</a>',
        unsafe_allow_html=True,
    )


def go_to(page_name):
    st.session_state.navigate_to = page_name
    st.rerun()


def clear_transaction_form():
    revision = st.session_state.get("tx_form_revision", 0)
    for key in ("tx_type_display", "tx_date", "tx_account", "tx_amount",
                "tx_category", "tx_desc", "tx_freq", "tx_due_day"):
        st.session_state.pop(f"{key}_{revision}", None)
    st.session_state.tx_form_revision = revision + 1


def polish_chart(fig, height=300):
    """Dung chung mau va kieu chu cho bieu do Plotly."""
    fig.update_layout(
        template="plotly_white",
        height=height,
        paper_bgcolor="#ffffff",
        plot_bgcolor="#ffffff",
        font=dict(family="Inter, sans-serif", color="#262626", size=12),
        margin=dict(l=16, r=16, t=28, b=16),
        legend=dict(orientation="h", y=1.12, x=1, xanchor="right"),
    )
    fig.update_xaxes(showgrid=False, zeroline=False)
    fig.update_yaxes(gridcolor="#eeeeee", zeroline=False)
    return fig

# ══════════════════════════════════════════════════════════════════
#  SESSION STATE – Cache du lieu, tranh goi C++ qua nhieu
# ══════════════════════════════════════════════════════════════════
def refresh():
    """Goi C++ lay du lieu moi nhat, luu vao session state."""
    try:
        st.session_state.finance_data = get_data()
        st.session_state.cpp_error = None
    except RuntimeError as e:
        st.session_state.cpp_error = str(e)

if "finance_data" not in st.session_state:
    refresh()


def d():
    """Lay du lieu hien tai tu session state."""
    return st.session_state.get("finance_data", {"accounts": [], "budgets": {}, "transactions": []})


# ══════════════════════════════════════════════════════════════════
#  KIEM TRA EXE
# ══════════════════════════════════════════════════════════════════
if st.session_state.get("cpp_error"):
    st.error(f"**Lỗi khởi động C++ backend:**\n\n{st.session_state.cpp_error}")
    st.info(f"Chạy `{BUILD_COMMAND}` trong thư mục dự án, sau đó tải lại trang.")
    st.stop()

# ══════════════════════════════════════════════════════════════════
#  SIDEBAR – MENU DIEU HUONG
# ══════════════════════════════════════════════════════════════════
st.sidebar.markdown(
    '<div class="sidebar-brand"><span class="sidebar-brand-mark">$</span>'
    '<span>FINANCE MANAGER</span></div>'
    '<div class="sidebar-section-label">QUẢN LÝ</div>',
    unsafe_allow_html=True,
)

PAGES = [
    "▦  Tổng quan",
    "↗  Thêm giao dịch",
    "≡  Lịch sử giao dịch",
    "▤  Tài khoản",
    "◎  Ngân sách",
    "↻  Chi phí định kỳ",
    "⌕  Tìm kiếm",
    "▥  Biểu đồ",
    "⚙  Cài đặt",
]

if "navigate_to" in st.session_state:
    st.session_state.page_nav = st.session_state.pop("navigate_to")

page = st.sidebar.radio("Chọn chức năng", PAGES, label_visibility="collapsed", key="page_nav")

# Tong tai san sidebar
total_balance = sum(acc["balance"] for acc in d()["accounts"])
st.sidebar.markdown('<div class="sidebar-spacer"></div>', unsafe_allow_html=True)
st.sidebar.markdown('<div class="sidebar-section-label">DỮ LIỆU CỦA BẠN</div>', unsafe_allow_html=True)
st.sidebar.markdown(
    '<div class="sidebar-balance"><span>Tổng tài sản</span>'
    f'<strong>{escape(money(total_balance))}</strong></div>',
    unsafe_allow_html=True,
)

if st.sidebar.button("↻  Làm mới dữ liệu", width="stretch"):
    refresh()
    st.rerun()


# ══════════════════════════════════════════════════════════════════
#  TRANG 1: TONG QUAN (DASHBOARD)
# ══════════════════════════════════════════════════════════════════
if page == PAGES[0]:
    with page_header("Xin chào! 👋", "Đây là bức tranh tài chính của bạn hôm nay."):
        if st.button("+ Giao dịch mới", key="header_new_transaction"):
            go_to(PAGES[1])

    accounts     = d()["accounts"]
    transactions = d()["transactions"]
    budgets      = d()["budgets"]

    total_income  = sum(tx["amount"] for tx in transactions if tx["type"] == "Income")
    total_expense = sum(tx["amount"] for tx in transactions if tx["type"] != "Income")
    net           = total_income - total_expense

    col1, col2, col3, col4 = st.columns(4)
    with col1:
        metric_card("TỔNG TÀI SẢN", money(total_balance), "Từ tất cả tài khoản")
    with col2:
        metric_card("THU NHẬP", money(total_income), "Trong toàn bộ dữ liệu")
    with col3:
        metric_card("CHI TIÊU", money(total_expense), "Trong toàn bộ dữ liệu")
    with col4:
        metric_card("DÒNG TIỀN RÒNG", money(net), "Thu nhập trừ chi tiêu")

    st.markdown('<div class="section-gap"></div>', unsafe_allow_html=True)
    col_left, col_right = st.columns([1.7, 1], gap="medium")

    with col_left:
        with st.container(border=True, key="panel_monthly_chart"):
            st.markdown("""
                <div style="display:flex; align-items:center; justify-content:space-between; gap:12px;">
                    <h3 style="margin:0; padding:0;">Thu chi theo tháng</h3>
                    <div style="display:flex; gap:14px; font-size:13px; white-space:nowrap;">
                        <span>
                            <span style="color:#2b2b2b;">●</span> Thu nhập
                        </span>
                        <span>
                            <span style="color:#12a75d;">●</span> Chi tiêu
                        </span>
                    </div>
                </div>
                """, unsafe_allow_html=True)
            st.caption("Theo dõi dòng tiền trong 6 tháng gần nhất")
            today = date.today()
            months = []
            for offset in range(5, -1, -1):
                month_index = today.year * 12 + today.month - 1 - offset
                months.append((month_index // 12, month_index % 12 + 1))
            income_by_month = []
            expense_by_month = []
            for year, month in months:
                period = f"{year:04d}-{month:02d}"
                income_by_month.append(sum(
                    tx["amount"] for tx in transactions
                    if tx["date"].startswith(period) and tx["type"] == "Income"
                ))
                expense_by_month.append(sum(
                    tx["amount"] for tx in transactions
                    if tx["date"].startswith(period) and tx["type"] != "Income"
                ))
            labels = [f"T{month:02d}" for _, month in months]
            fig = go.Figure()

            fig.add_bar(
                name="Thu nhập",
                x=labels,
                y=income_by_month,
                marker_color="#2b2b2b",
            )

            fig.add_bar(
                name="Chi tiêu",
                x=labels,
                y=expense_by_month,
                marker_color="#12a75d",
            )

            fig = polish_chart(fig, 220)

            # Đặt sau polish_chart để các thiết lập này được áp dụng cuối cùng.
            fig.update_layout(
                barmode="group",
                bargap=0.35,
                bargroupgap=0.15,
                barcornerradius=7,
                showlegend=False,
                plot_bgcolor="white",
                paper_bgcolor="white",
                margin=dict(l=10, r=10, t=10, b=25),
            )

            fig.update_yaxes(visible=False)
            fig.update_xaxes(
                showgrid=False,
                showline=False,
                zeroline=False,
                ticks="",
            )
            fig.update_traces(
                base=0,
                marker_line_width=0,
                selector=dict(type="bar"),
            )

            st.plotly_chart(fig, width="stretch")

    with col_right:
        with st.container(border=True, key="panel_monthly_budget"):
            st.subheader("Ngân sách tháng này")
            st.caption("Bạn đang kiểm soát chi tiêu khá tốt")
            if budgets:
                period = today.strftime("%Y-%m")
                for category, limit in list(budgets.items())[:3]:
                    spent = sum(
                        tx["amount"] for tx in transactions
                        if tx["type"] != "Income"
                        and tx["category"] == category
                        and tx["date"].startswith(period)
                    )
                    ratio = spent / limit if limit > 0 else 0
                    st.markdown(
                        f'<div class="budget-line"><strong>{escape(category)}</strong>'
                        f'<span>{escape(money(spent))} / {escape(money(limit))}</span></div>',
                        unsafe_allow_html=True,
                    )
                    st.progress(min(ratio, 1.0))
            else:
                st.info("Chưa có hạn mức ngân sách nào.")

    st.markdown('<div class="section-gap"></div>', unsafe_allow_html=True)
    col_left, col_right = st.columns([1.7, 1], gap="medium")
    with col_left:
        with st.container(border=True, key="panel_recent_transactions"):
            title, more = st.columns([4, 1])
            with title:
                st.subheader("Giao dịch gần đây")
            with more:
                if st.button("Xem tất cả →", key="recent_all"):
                    go_to(PAGES[2])
            if transactions:
                last3 = sorted(transactions, key=lambda tx: tx["date"], reverse=True)[:3]
                rows = []
                for tx in last3:
                    is_income = tx["type"] == "Income"
                    icon = "↗" if is_income else ("↻" if tx["type"] == "RecurringExpense" else "↙")
                    amount = ("+ " if is_income else "− ") + money(tx["amount"])
                    title_text = tx["description"] or tx["category"]
                    rows.append(
                        '<div class="transaction-row">'
                        f'<span class="row-icon">{icon}</span>'
                        '<span class="row-main">'
                        f'<strong>{escape(title_text)}</strong>'
                        f'<small>{escape(tx["category"])} · {escape(tx["date"])}</small>'
                        '</span>'
                        f'<strong class="row-amount">{escape(amount)}</strong>'
                        '</div>'
                    )
                st.markdown("".join(rows), unsafe_allow_html=True)
            else:
                st.info("Chưa có giao dịch nào.")

    with col_right:
        with st.container(border=True, key="panel_accounts"):
            st.subheader("Tài khoản của bạn")
            if accounts:
                rows = []
                for acc in accounts[:3]:
                    rows.append(
                        '<div class="account-row">'
                        '<span class="row-icon">▣</span>'
                        f'<strong>{escape(acc["name"])}</strong>'
                        f'<span>{escape(money(acc["balance"]))}</span>'
                        '</div>'
                    )
                st.markdown("".join(rows), unsafe_allow_html=True)
            else:
                st.info("Chưa có tài khoản nào.")


# ══════════════════════════════════════════════════════════════════
#  TRANG 2: THEM GIAO DICH
# ══════════════════════════════════════════════════════════════════
elif page == PAGES[1]:
    with page_header("Thêm giao dịch mới", "Ghi lại khoản thu, chi và các khoản định kỳ."):
        if st.button("Xem lịch sử", key="header_history_from_add"):
            go_to(PAGES[2])

    account_names = [acc["name"] for acc in d()["accounts"]]
    if not account_names:
        st.warning("Bạn chưa có tài khoản nào. Hãy tạo tài khoản trước khi thêm giao dịch.")
        st.stop()

    tx_revision = st.session_state.get("tx_form_revision", 0)
    left, right = st.columns([1.75, 1], gap="medium")
    with left:
        with st.container(border=True, key="panel_transaction_form"):
            st.subheader("Thông tin giao dịch")
            st.caption("Điền các thông tin cần thiết để thêm giao dịch.")
            tx_type_display = st.radio(
                "Loại giao dịch", ["Chi tiêu", "Thu nhập", "Định kỳ"],
                horizontal=True, label_visibility="collapsed",
                key=f"tx_type_display_{tx_revision}",
            )
            tx_type = {
                "Chi tiêu": "Expense",
                "Thu nhập": "Income",
                "Định kỳ": "RecurringExpense",
            }[tx_type_display]

            with st.form(f"form_add_tx_{tx_revision}"):
                col1, col2 = st.columns(2)
                with col1:
                    tx_date = st.date_input(
                        "Ngày giao dịch", value=date.today(), key=f"tx_date_{tx_revision}",
                    )
                with col2:
                    tx_account = st.selectbox(
                        "Tài khoản", account_names, key=f"tx_account_{tx_revision}",
                    )
                col1, col2 = st.columns(2)
                with col1:
                    tx_amount = st.number_input(
                        "Số tiền (VND)", min_value=1.0, step=1000.0, format="%.0f",
                        key=f"tx_amount_{tx_revision}",
                    )
                with col2:
                    tx_category = st.text_input(
                        "Danh mục", placeholder="Ví dụ: Ăn uống, Lương...",
                        key=f"tx_category_{tx_revision}",
                    )
                tx_desc = st.text_input(
                    "Ghi chú", placeholder="Mô tả ngắn về giao dịch...",
                    key=f"tx_desc_{tx_revision}",
                )

                tx_freq = "Monthly"
                tx_due_day = 1
                if tx_type == "RecurringExpense":
                    col3, col4 = st.columns(2)
                    with col3:
                        tx_freq = st.selectbox(
                            "Tần suất", ["Monthly", "Weekly", "Yearly"],
                            format_func=lambda value: {
                                "Monthly": "Hàng tháng", "Weekly": "Hàng tuần",
                                "Yearly": "Hàng năm",
                            }[value], key=f"tx_freq_{tx_revision}",
                        )
                    with col4:
                        tx_due_day = st.number_input(
                            "Ngày đến hạn (1–31)", min_value=1, max_value=31,
                            value=1, key=f"tx_due_day_{tx_revision}",
                        )

                spacer, clear_col, submit_col = st.columns([1.4, 1, 1.35])
                with clear_col:
                    st.form_submit_button(
                        "Xóa nội dung", width="stretch", key="tx_clear",
                        on_click=clear_transaction_form,
                    )
                with submit_col:
                    submitted = st.form_submit_button("+ Thêm giao dịch", width="stretch", key="tx_submit")

    with right:
        with st.container(border=True, key="panel_transaction_tips"):
            st.subheader("Giao dịch rõ ràng hơn")
            st.caption("Chọn đúng danh mục để theo dõi ngân sách và biểu đồ chính xác.")
            st.markdown("✓  Ghi chú ngắn, dễ tìm lại")
            st.markdown("✓  Kiểm tra tài khoản trước khi lưu")
            st.markdown("✓  Dùng Định kỳ cho khoản lặp lại")
        with st.container(border=True, key="panel_transaction_recurring"):
            st.subheader("Chi phí định kỳ")
            st.caption("Khi chọn loại Định kỳ, biểu mẫu sẽ hiện thêm tần suất và ngày đến hạn.")
            st.markdown(
                '<div class="frequency-pills"><span>Hàng tháng</span>'
                '<span>Hàng tuần</span><span>Hàng năm</span></div>',
                unsafe_allow_html=True,
            )

    if submitted:
        if not tx_category.strip():
            st.error("Vui lòng nhập danh mục.")
        else:
            try:
                result = add_transaction(
                    tx_type, str(tx_date), float(tx_amount),
                    tx_category.strip(), tx_account, tx_desc.strip(),
                    tx_freq, tx_due_day
                )
                if result.get("ok"):
                    # C++ da xu ly, cap nhat session state
                    st.session_state.finance_data = result.get("data", d())
                    st.success("Đã thêm giao dịch thành công.")

                    # Hien thi canh bao ngan sach neu co
                    alert = result.get("budget_alert")
                    if alert and isinstance(alert, dict) and alert.get("status") != "ok":
                        cat   = alert.get("category", "")
                        spent = alert.get("spent", 0)
                        lim   = alert.get("limit", 0)
                        pct   = alert.get("percent", 0)
                        if alert.get("status") == "over":
                            st.error(f"⚠️ VUOT NGAN SACH [{cat}]! Da chi {format_vnd(spent)} / Han muc {format_vnd(lim)} ({pct}%)")
                        else:
                            st.warning(f"🔶 Canh bao: Da su dung {pct}% ngan sach [{cat}] ({format_vnd(spent)} / {format_vnd(lim)})")
                else:
                    st.error(f"❌ {result.get('message', 'Loi khong xac dinh')}")
            except RuntimeError as e:
                st.error(f"❌ Loi C++ backend: {e}")


# ══════════════════════════════════════════════════════════════════
#  TRANG 3: LICH SU GIAO DICH
# ══════════════════════════════════════════════════════════════════
elif page == PAGES[2]:
    header_action = page_header("Lịch sử giao dịch", "Xem, lọc và xuất toàn bộ giao dịch của bạn.")

    transactions = d()["transactions"]
    if not transactions:
        st.info("Chưa có giao dịch nào.")
        st.stop()

    df = pd.DataFrame(transactions)[["id","date","type","category","account","amount","description"]]
    df.columns = ["ID", "Ngày", "Loại", "Danh mục", "Tài khoản", "Số tiền", "Ghi chú"]

    with st.container(border=True, key="panel_history_filters"):
        st.subheader("Lọc giao dịch")
        col1, col2, col3 = st.columns(3)
        with col1:
            filter_type = st.selectbox(
                "Loại giao dịch", ["Tất cả", "Income", "Expense", "RecurringExpense"],
                format_func=lambda value: {
                    "Income": "Thu nhập", "Expense": "Chi tiêu",
                    "RecurringExpense": "Định kỳ",
                }.get(value, value),
            )
        with col2:
            all_cats = sorted(set(tx["category"] for tx in transactions))
            filter_cat = st.selectbox("Danh mục", ["Tất cả"] + all_cats)
        with col3:
            all_accs = sorted(set(tx["account"] for tx in transactions))
            filter_acc = st.selectbox("Tài khoản", ["Tất cả"] + all_accs)

    df_show = df.copy()
    if filter_type != "Tất cả":
        df_show = df_show[df_show["Loại"] == filter_type]
    if filter_cat != "Tất cả":
        df_show = df_show[df_show["Danh mục"] == filter_cat]
    if filter_acc != "Tất cả":
        df_show = df_show[df_show["Tài khoản"] == filter_acc]

    df_show = df_show.sort_values("Ngày", ascending=False)
    df_disp = df_show.copy()
    df_disp["Loại"] = df_disp["Loại"].map({
        "Income": "Thu nhập", "Expense": "Chi tiêu",
        "RecurringExpense": "Định kỳ",
    }).fillna(df_disp["Loại"])
    df_disp["Số tiền"] = df_disp["Số tiền"].apply(money)
    with st.container(border=True, key="panel_history_table"):
        st.subheader(f"{len(df_show)} giao dịch gần đây")
        st.dataframe(df_disp, width="stretch", hide_index=True)

    csv_bytes = df_show.to_csv(index=False).encode("utf-8")
    with header_action:
        st.download_button(
            "Xuất CSV", data=csv_bytes, file_name="ket_qua.csv",
            mime="text/csv", key="header_export_history",
        )


# ══════════════════════════════════════════════════════════════════
#  TRANG 4: QUAN LY TAI KHOAN
# ══════════════════════════════════════════════════════════════════
elif page == PAGES[3]:
    with page_header("Quản lý tài khoản", "Theo dõi số dư và thêm nguồn tiền mới."):
        header_link("+ Thêm tài khoản", "add-account")

    accounts = d()["accounts"]
    summary = st.columns(3)
    with summary[0]:
        metric_card("TỔNG TÀI SẢN", money(total_balance), "Trên tất cả tài khoản")
    with summary[1]:
        metric_card("TÀI KHOẢN ĐANG DÙNG", f"{len(accounts):02d}", "Được kết nối với giao dịch")
    with summary[2]:
        largest = max(accounts, key=lambda acc: acc["balance"]) if accounts else None
        metric_card(
            "SỐ DƯ LỚN NHẤT",
            money(largest["balance"]) if largest else money(0),
            largest["name"] if largest else "Chưa có tài khoản",
        )

    for offset in range(0, len(accounts), 3):
        cols = st.columns(3)
        for col, acc in zip(cols, accounts[offset:offset + 3]):
            name_lower = acc["name"].lower()
            kind = ("VÍ ĐIỆN TỬ" if "wallet" in name_lower or "momo" in name_lower
                    else "TIỀN MẶT" if "cash" in name_lower or "tiền mặt" in name_lower
                    else "NGÂN HÀNG")
            with col:
                st.markdown(
                    '<div class="account-card">'
                    f'<span>{kind}</span><strong>{escape(acc["name"])}</strong>'
                    f'<b>{escape(money(acc["balance"]))}</b>'
                    '<small>● Đang hoạt động</small></div>',
                    unsafe_allow_html=True,
                )

    st.markdown('<div class="section-gap"></div>', unsafe_allow_html=True)
    st.markdown('<div id="add-account"></div>', unsafe_allow_html=True)
    with st.form("form_add_account"):
        st.subheader("Thêm tài khoản mới")
        st.caption("Đặt tên dễ nhận biết khi ghi giao dịch.")
        col1, col2 = st.columns(2)
        with col1:
            acc_name = st.text_input("Tên tài khoản", placeholder="Ví dụ: MoMo, Vietcombank...")
        with col2:
            acc_balance = st.number_input("Số dư ban đầu (VND)", min_value=0.0, step=1000.0, format="%.0f")
        _, submit_col = st.columns([4, 1.2])
        with submit_col:
            add_btn = st.form_submit_button("+ Thêm tài khoản", width="stretch")

    if add_btn:
        if not acc_name.strip():
            st.error("Vui lòng nhập tên tài khoản.")
        else:
            try:
                result = add_account(acc_name.strip(), float(acc_balance))
                # add_account tra ve toan bo du lieu moi
                if "accounts" in result:
                    st.session_state.finance_data = result
                    st.success(f"Đã thêm tài khoản [{acc_name}].")
                    st.rerun()
                else:
                    st.error(result.get("message", "Loi"))
            except RuntimeError as e:
                st.error(f"❌ {e}")


# ══════════════════════════════════════════════════════════════════
#  TRANG 5: NGAN SACH & CANH BAO
# ══════════════════════════════════════════════════════════════════
elif page == PAGES[4]:
    with page_header("Ngân sách & cảnh báo", "Theo dõi hạn mức và mức sử dụng theo danh mục."):
        header_link("+ Đặt hạn mức", "set-budget")

    transactions = d()["transactions"]
    budgets      = d()["budgets"]

    with st.container(border=True, key="panel_budget_period"):
        st.subheader("Khoảng thời gian")
        col1, col2 = st.columns(2)
        with col1:
            b_month = st.selectbox("Tháng", [0] + list(range(1, 13)), index=date.today().month,
                                   format_func=lambda x: "Tất cả tháng" if x == 0 else f"Tháng {x}")
        with col2:
            budget_years = [0] + list(range(date.today().year - 2, date.today().year + 2))
            b_year = st.selectbox("Năm", budget_years, index=budget_years.index(date.today().year),
                                  format_func=lambda x: "Tất cả năm" if x == 0 else str(x))

    # Tinh so thang de nhan han muc
    if b_month > 0 and b_year > 0:
        months_count = 1
    elif b_year > 0 and b_month == 0:
        months_count = 12
    else:
        all_ym = set(tx["date"][:7] for tx in transactions)
        months_count = max(1, len(all_ym))

    # Ham tinh chi tieu tu du lieu Python (khong can goi C++ them)
    def calc_spent(cat, month, year):
        total = 0.0
        for tx in transactions:
            if tx["type"] == "Income" or tx["category"] != cat:
                continue
            ty = int(tx["date"][:4])
            tm = int(tx["date"][5:7])
            if month > 0 and year > 0:
                if ty == year and tm == month: total += tx["amount"]
            elif year > 0:
                if ty == year: total += tx["amount"]
            else:
                total += tx["amount"]
        return total

    if not budgets:
        st.info("Chưa có hạn mức ngân sách nào.")
    else:
        rows = []
        limit_column = f"Hạn mức ({months_count} tháng)"
        for cat, monthly_limit in budgets.items():
            limit     = monthly_limit * months_count
            spent     = calc_spent(cat, b_month, b_year)
            remaining = limit - spent
            pct       = (spent / limit * 100) if limit > 0 else 0
            status    = "Vượt hạn mức" if spent > limit else ("Sắp chạm hạn mức" if pct >= 80 else "An toàn")
            rows.append({
                "Danh mục": cat,
                limit_column: format_vnd(limit),
                "Đã chi": format_vnd(spent),
                "Còn lại": format_vnd(remaining) if remaining >= 0 else f"-{format_vnd(-remaining)}",
                "%": f"{pct:.0f}%",
                "Trạng thái": status,
            })
        for offset in range(0, len(rows), 3):
            cols = st.columns(3)
            for index, (col, item) in enumerate(zip(cols, rows[offset:offset + 3])):
                with col:
                    with st.container(border=True, key=f"panel_budget_{offset + index}"):
                        st.subheader(item["Danh mục"])
                        st.caption(f'{item["Đã chi"]} / {item[limit_column]}')
                        st.progress(min(float(item["%"].rstrip("%")) / 100, 1.0))
                        st.caption(f'{item["%"]} ngân sách đã dùng')
        with st.expander("Xem bảng chi tiết"):
            st.dataframe(pd.DataFrame(rows), width="stretch", hide_index=True)

    st.markdown('<div id="set-budget"></div>', unsafe_allow_html=True)
    with st.form("form_budget"):
        st.subheader("Đặt hạn mức mới")
        st.caption("Hạn mức được áp dụng theo từng tháng cho danh mục.")
        col1, col2 = st.columns(2)
        with col1:
            b_cat = st.text_input("Danh mục", placeholder="Ví dụ: Ăn uống, Giải trí...")
        with col2:
            b_limit = st.number_input("Hạn mức mỗi tháng (VND)", min_value=1.0, step=10000.0, format="%.0f")
        _, submit_col = st.columns([4, 1.2])
        with submit_col:
            b_submit = st.form_submit_button("+ Lưu hạn mức", width="stretch")

    if b_submit:
        if not b_cat.strip():
            st.error("Vui lòng nhập danh mục.")
        else:
            try:
                result = set_budget(b_cat.strip(), float(b_limit))
                if "budgets" in result:
                    st.session_state.finance_data = result
                    st.success(f"Đã đặt hạn mức [{b_cat}]: {format_vnd(b_limit)} / tháng.")
                    st.rerun()
                else:
                    st.error(result.get("message","Loi"))
            except RuntimeError as e:
                st.error(f"❌ {e}")


# ══════════════════════════════════════════════════════════════════
#  TRANG 6: CHI PHI DINH KY
# ══════════════════════════════════════════════════════════════════
elif page == PAGES[5]:
    with page_header("Chi phí định kỳ", "Xem trước các khoản đến hạn rồi quyết định có ghi nhận hay không."):
        header_link("Xem trước tháng mới", "simulate-month")

    if notice := st.session_state.pop("recurring_notice", None):
        st.success(notice)

    transactions = d()["transactions"]
    recurring    = [tx for tx in transactions if tx["type"] == "RecurringExpense"]

    # Giao dich da tao tu mo phong khong phai la lich dinh ky goc.
    seen_recurring = set()
    unique_rec = []
    for tx in recurring:
        if tx["description"].startswith("[Tu dong] "):
            continue
        key = (tx["category"], tx["account"], tx["description"],
               tx["due_day"], tx["frequency"])
        if key not in seen_recurring:
            seen_recurring.add(key)
            unique_rec.append(tx)

    monthly_rec = [tx for tx in unique_rec if tx["frequency"] == "Monthly"]
    total_fixed = sum(tx["amount"] for tx in monthly_rec)
    next_month = date.today().month % 12 + 1
    next_year = date.today().year + (date.today().month == 12)
    m1, m2, m3 = st.columns(3)
    with m1:
        metric_card("TỔNG CHI PHÍ / THÁNG", money(total_fixed), "Từ các khoản định kỳ đang có")
    with m2:
        metric_card("SỐ LỊCH HÀNG THÁNG", f"{len(monthly_rec):02d}", "Khoản tham gia mô phỏng")
    with m3:
        metric_card("KỲ TIẾP THEO", f"Tháng {next_month}", f"Năm {next_year}")

    st.markdown('<div class="section-gap"></div>', unsafe_allow_html=True)
    with st.container(border=True, key="panel_recurring_table"):
        st.subheader("Các khoản định kỳ")
        st.caption("Mô phỏng theo tháng hiện áp dụng cho các khoản có tần suất Hàng tháng.")
        if unique_rec:
            df_rec = pd.DataFrame(unique_rec)[["category","account","amount","frequency","due_day","description"]]
            df_rec["amount"] = df_rec["amount"].apply(money)
            df_rec.columns = ["Danh mục", "Tài khoản", "Số tiền / tháng", "Tần suất", "Ngày đến hạn", "Ghi chú"]
            df_rec["Tần suất"] = df_rec["Tần suất"].map({
                "Monthly": "Hàng tháng", "Weekly": "Hàng tuần", "Yearly": "Hàng năm",
            }).fillna(df_rec["Tần suất"])
            st.dataframe(df_rec, width="stretch", hide_index=True)
        else:
            st.info("Chưa có khoản chi phí định kỳ nào.")

    st.markdown('<div id="simulate-month"></div>', unsafe_allow_html=True)
    with st.form("form_simulate"):
        st.subheader("Bước 1 · Xem trước tháng mới")
        st.caption("Xem giao dịch và số dư dự kiến. Bước này không thay đổi dữ liệu của bạn.")
        col1, col2 = st.columns(2)
        with col1:
            sim_month = st.selectbox("Tháng", list(range(1, 13)), index=next_month - 1)
        with col2:
            sim_year  = st.number_input("Năm", min_value=2020, max_value=2030, value=next_year)
        _, submit_col = st.columns([4, 1.2])
        with submit_col:
            sim_submit = st.form_submit_button("▶ Xem trước", width="stretch")

    if sim_submit:
        try:
            current_data = get_data()
            result = preview_recurring(int(sim_year), sim_month)
            if result.get("ok") and "data" in result:
                st.session_state.finance_data = current_data
                st.session_state.recurring_preview = {
                    "month": sim_month, "year": int(sim_year),
                    "before": current_data, "after": result["data"],
                    "applied": False,
                }
                st.session_state.recurring_preview_revision = (
                    st.session_state.get("recurring_preview_revision", 0) + 1
                )
                st.rerun()
            else:
                st.error(result.get("message","Loi"))
        except RuntimeError as e:
            st.error(f"❌ {e}")

    preview = st.session_state.get("recurring_preview")
    if preview:
        before = preview["before"]
        after = preview["after"]
        before_ids = {tx["id"] for tx in before["transactions"]}
        new_transactions = [tx for tx in after["transactions"] if tx["id"] not in before_ids]
        with st.container(border=True, key="panel_recurring_preview"):
            step = "Bước 3 · Đã ghi nhận" if preview["applied"] else "Bước 2 · Kết quả xem trước"
            st.subheader(f'{step} tháng {preview["month"]}/{preview["year"]}')
            st.caption(
                "Các giao dịch dưới đây đã được lưu vào dữ liệu thật."
                if preview["applied"] else "Đây là bản xem trước; dữ liệu thật chưa thay đổi."
            )
            p1, p2, p3 = st.columns(3)
            with p1:
                metric_card("KHOẢN SẼ GHI", f"{len(new_transactions):02d}", "Giao dịch mới")
            with p2:
                metric_card("TỔNG CHI DỰ KIẾN", money(sum(tx["amount"] for tx in new_transactions)), "Trong kỳ đã chọn")
            with p3:
                metric_card("TRẠNG THÁI", "Đã lưu" if preview["applied"] else "Chưa lưu",
                            "Dữ liệu đã cập nhật" if preview["applied"] else "An toàn để trình diễn")

            if new_transactions:
                st.markdown("**Giao dịch sẽ được tạo**")
                st.dataframe(pd.DataFrame([{
                    "Ngày": tx["date"], "Khoản chi": tx["description"].removeprefix("[Tu dong] "),
                    "Tài khoản": tx["account"], "Số tiền": money(tx["amount"]),
                } for tx in new_transactions]), width="stretch", hide_index=True)

                before_balances = {acc["name"]: acc["balance"] for acc in before["accounts"]}
                after_balances = {acc["name"]: acc["balance"] for acc in after["accounts"]}
                affected_accounts = dict.fromkeys(tx["account"] for tx in new_transactions)
                st.markdown("**Số dư tài khoản dự kiến**")
                st.dataframe(pd.DataFrame([{
                    "Tài khoản": name, "Trước": money(before_balances[name]),
                    "Sau": money(after_balances[name]),
                } for name in affected_accounts]), width="stretch", hide_index=True)

                if preview["applied"]:
                    if st.button("Xem trong lịch sử giao dịch", key="view_applied_recurring"):
                        go_to(PAGES[2])
                else:
                    confirmed = st.checkbox(
                        "Tôi muốn ghi các giao dịch dự kiến vào dữ liệu thật.",
                        key=f'confirm_recurring_{st.session_state.recurring_preview_revision}',
                    )
                    if st.button("Ghi nhận vào dữ liệu", disabled=not confirmed, key="apply_recurring_preview"):
                        try:
                            latest_data = get_data()
                            if latest_data != before:
                                st.session_state.finance_data = latest_data
                                st.session_state.pop("recurring_preview", None)
                                st.warning("Dữ liệu đã thay đổi. Hãy xem trước lại trước khi ghi nhận.")
                            else:
                                applied_result = simulate_recurring(preview["year"], preview["month"])
                                if applied_result.get("ok"):
                                    st.session_state.finance_data = applied_result["data"]
                                    preview["after"] = applied_result["data"]
                                    preview["applied"] = True
                                    st.session_state.recurring_notice = (
                                        f'Đã ghi nhận {applied_result["applied"]} khoản định kỳ '
                                        f'cho tháng {preview["month"]}/{preview["year"]}.'
                                    )
                                    st.rerun()
                                else:
                                    st.error(applied_result.get("message", "Không thể ghi nhận dữ liệu."))
                        except RuntimeError as e:
                            st.error(f"❌ {e}")
            else:
                st.info("Không có khoản mới để ghi. Kỳ này có thể đã được áp dụng, chưa đến hạn hoặc tài khoản không đủ số dư.")


# ══════════════════════════════════════════════════════════════════
#  TRANG 7: LOC & TIM KIEM
# ══════════════════════════════════════════════════════════════════
elif page == PAGES[6]:
    with page_header("Lọc & tìm kiếm giao dịch", "Tìm giao dịch theo ngày, danh mục hoặc tài khoản."):
        if st.button("Xem lịch sử", key="header_history_from_search"):
            go_to(PAGES[2])

    transactions = d()["transactions"]
    if not transactions:
        st.info("Chưa có giao dịch nào.")
        st.stop()

    tab1, tab2, tab3 = st.tabs(["Theo ngày", "Theo danh mục", "Theo tài khoản"])

    def show_tx_table(txs, cols=None):
        if not txs:
            st.info("Không có giao dịch phù hợp.")
            return
        df = pd.DataFrame(txs)
        if cols:
            df = df[cols]
        df["amount"] = df["amount"].apply(money)
        if "type" in df:
            df["type"] = df["type"].map({
                "Income": "Thu nhập", "Expense": "Chi tiêu",
                "RecurringExpense": "Định kỳ",
            }).fillna(df["type"])
        df = df.rename(columns={
            "date": "Ngày", "type": "Loại", "category": "Danh mục",
            "account": "Tài khoản", "amount": "Số tiền", "description": "Ghi chú",
        })
        st.dataframe(df, width="stretch", hide_index=True)

    with tab1:
        with st.container(border=True, key="panel_search_dates"):
            st.subheader("Khoảng ngày cần tìm")
            c1, c2 = st.columns(2)
            with c1: start = st.date_input("Từ ngày", value=date(date.today().year, 1, 1))
            with c2: end   = st.date_input("Đến ngày", value=date.today())
        res1 = [tx for tx in transactions if str(start) <= tx["date"] <= str(end)]
        with st.container(border=True, key="panel_search_date_results"):
            st.subheader(f"{len(res1)} kết quả phù hợp")
            show_tx_table(res1, ["date","type","category","account","amount","description"])

    with tab2:
        with st.container(border=True, key="panel_search_categories"):
            st.subheader("Danh mục cần tìm")
            cats = sorted(set(tx["category"] for tx in transactions))
            sel_cat = st.selectbox("Danh mục", cats)
        res2 = [tx for tx in transactions if tx["category"] == sel_cat]
        with st.container(border=True, key="panel_search_category_results"):
            st.subheader(f"{len(res2)} kết quả phù hợp")
            show_tx_table(res2, ["date","type","account","amount","description"])

    with tab3:
        with st.container(border=True, key="panel_search_accounts"):
            st.subheader("Tài khoản cần tìm")
            accs = sorted(set(tx["account"] for tx in transactions))
            sel_acc = st.selectbox("Tài khoản", accs)
        res3 = [tx for tx in transactions if tx["account"] == sel_acc]
        with st.container(border=True, key="panel_search_account_results"):
            st.subheader(f"{len(res3)} kết quả phù hợp")
            show_tx_table(res3, ["date","type","category","amount","description"])


# ══════════════════════════════════════════════════════════════════
#  TRANG 8: BIEU DO CHI TIEU
# ══════════════════════════════════════════════════════════════════
elif page == PAGES[7]:
    header_action = page_header("Biểu đồ chi tiêu", "Nhìn rõ xu hướng chi tiêu và so sánh với hạn mức.")

    transactions = d()["transactions"]
    budgets      = d()["budgets"]

    expense_txs = [tx for tx in transactions if tx["type"] != "Income"]
    if not expense_txs:
        st.info("Chưa có dữ liệu chi tiêu.")
        st.stop()

    chart_years = list(range(date.today().year - 2, date.today().year + 2))
    with header_action:
        chart_year = st.selectbox(
            "Chọn năm", chart_years, index=2,
            label_visibility="collapsed", key="chart_year",
        )

    tab1, tab2 = st.tabs(["Theo tháng", "Theo danh mục"])

    with tab1:
        monthly = {m: 0.0 for m in range(1,13)}
        for tx in expense_txs:
            if int(tx["date"][:4]) == chart_year:
                monthly[int(tx["date"][5:7])] += tx["amount"]
        fig = go.Figure(go.Bar(
            x=[f"T{m:02d}" for m in range(1, 13)],
            y=[monthly[m] for m in range(1, 13)],
            marker_color=[
                "#12a75d" if chart_year == date.today().year and m == date.today().month
                else "#2b2b2b" for m in range(1, 13)
            ],
            hovertemplate="%{x}: %{y:,.0f} VND<extra></extra>",
        ))
        fig.update_layout(showlegend=False, bargap=.5)
        with st.container(border=True, key="panel_monthly_spending"):
            st.subheader(f"Chi tiêu theo tháng · {chart_year}")
            st.caption("Tổng chi tiêu theo từng tháng trong năm")
            fig = polish_chart(fig, 220)
            
            # Đặt sau polish_chart để các thiết lập này được áp dụng cuối cùng.
            fig.update_layout(
                barmode="group",
                bargap=0.35,
                bargroupgap=0.15,
                barcornerradius=7,
                showlegend=False,
                plot_bgcolor="white",
                paper_bgcolor="white",
                margin=dict(l=10, r=10, t=10, b=25),
                )
            fig.update_traces(
                base=0,
                marker_line_width=0,
                selector=dict(type="bar"),
                )
            st.plotly_chart(polish_chart(fig, 340), width="stretch")

        months_with_spending = sum(amount > 0 for amount in monthly.values())
        by_category = {}
        for tx in expense_txs:
            if int(tx["date"][:4]) == chart_year:
                by_category[tx["category"]] = by_category.get(tx["category"], 0) + tx["amount"]
        c1, c2, c3 = st.columns(3)
        with c1:
            metric_card("CHI TIÊU CAO NHẤT", money(max(monthly.values())), f"Trong năm {chart_year}")
        with c2:
            metric_card(
                "BÌNH QUÂN / THÁNG",
                money(sum(monthly.values()) / max(1, months_with_spending)),
                f"Trong năm {chart_year}",
            )
        with c3:
            metric_card(
                "DANH MỤC LỚN NHẤT",
                max(by_category, key=by_category.get) if by_category else "—",
                "So với các danh mục khác",
            )

    with tab2:
        c1, c2 = st.columns(2)
        with c1:
            cat_month = st.selectbox("Tháng", [0]+list(range(1,13)),
                                     format_func=lambda x: "Tất cả" if x==0 else f"Tháng {x}")
        with c2:
            cat_year  = st.selectbox("Năm", [0] + chart_years,
                                     format_func=lambda x: "Tất cả" if x==0 else str(x))

        if cat_month == 0 and cat_year > 0:
            months_count = 12
        elif cat_month == 0 and cat_year == 0:
            months_count = max(1, len(set(tx["date"][:7] for tx in transactions)))
        else:
            months_count = 1

        def calc_spent_chart(cat, month, year):
            total = 0.0
            for tx in expense_txs:
                if tx["category"] != cat: continue
                ty = int(tx["date"][:4]); tm = int(tx["date"][5:7])
                if month > 0 and year > 0:
                    if ty == year and tm == month: total += tx["amount"]
                elif year > 0:
                    if ty == year: total += tx["amount"]
                else:
                    total += tx["amount"]
            return total

        cats_all = sorted(set(tx["category"] for tx in expense_txs))
        cat_data = [{"Danh mục": c,
                     "Đã chi":   calc_spent_chart(c, cat_month, cat_year),
                     "Hạn mức":  budgets.get(c, 0) * months_count}
                    for c in cats_all]
        df_cat = pd.DataFrame(cat_data)
        df_cat = df_cat[df_cat["Đã chi"] > 0]

        if df_cat.empty:
            st.info("Không có dữ liệu.")
        else:
            with st.container(border=True, key="panel_category_spending"):
                st.subheader("Chi tiêu so với hạn mức")
                st.caption("Mức sử dụng ngân sách theo từng danh mục")
                for row in df_cat.to_dict("records"):
                    limit = row["Hạn mức"]
                    pct = min(row["Đã chi"] / limit * 100, 100) if limit > 0 else 0
                    st.markdown(
                        '<div class="category-row">'
                        f'<strong>{escape(row["Danh mục"])}</strong>'
                        '<span class="category-track">'
                        f'<i style="width:{pct:.0f}%"></i></span>'
                        f'<small>{escape(money(row["Đã chi"]))} / '
                        f'{escape(money(limit))}</small></div>',
                        unsafe_allow_html=True,
                    )


# ══════════════════════════════════════════════════════════════════
#  TRANG 9: CAI DAT DU LIEU
# ══════════════════════════════════════════════════════════════════
elif page == PAGES[8]:
    with page_header("Cài đặt dữ liệu", "Nạp, xuất và quản lý dữ liệu tài chính."):
        header_link("Tải dữ liệu", "download-data")

    data = d()
    m1, m2, m3 = st.columns(3)
    with m1:
        metric_card("TÀI KHOẢN", f'{len(data["accounts"]):02d}', "Nguồn tiền đang theo dõi")
    with m2:
        metric_card("GIAO DỊCH", f'{len(data["transactions"]):02d}', "Trong dữ liệu hiện tại")
    with m3:
        metric_card("HẠN MỨC NGÂN SÁCH", f'{len(data["budgets"]):02d}', "Danh mục đã đặt hạn mức")

    st.markdown('<div class="settings-gap"></div>', unsafe_allow_html=True)
    with st.container(border=True, key="panel_import_data"):
        st.subheader("Nạp dữ liệu từ tệp CSV")
        st.caption("Chọn tệp finance_data.csv đã tải từ ứng dụng. Tệp gồm tài khoản, ngân sách và giao dịch.")
        if notice := st.session_state.pop("import_notice", None):
            st.success(notice)
        uploaded_file = st.file_uploader("Chọn tệp CSV", type=["csv"], key="finance_csv_upload")
        if uploaded_file is not None:
            content = uploaded_file.getvalue()
            try:
                counts, _ = validate_finance_csv(content)
            except ValueError as e:
                st.error(f"Không thể nạp tệp: {e}")
            else:
                st.info(
                    f'Tệp hợp lệ: {counts["accounts"]} tài khoản, '
                    f'{counts["budgets"]} hạn mức, {counts["transactions"]} giao dịch.'
                )
                file_hash = hashlib.sha256(content).hexdigest()[:12]
                confirmed = st.checkbox(
                    "Tôi hiểu dữ liệu hiện tại sẽ được thay thế. Ứng dụng sẽ lưu một bản sao trước khi nạp.",
                    key=f"confirm_import_{file_hash}",
                )
                if st.button("↑ Nạp dữ liệu từ tệp", disabled=not confirmed, key="import_data_button"):
                    try:
                        imported, backup = import_finance_csv(content)
                    except (OSError, RuntimeError, ValueError, KeyError) as e:
                        st.error(f"Nạp dữ liệu thất bại: {e}")
                    else:
                        st.session_state.finance_data = imported
                        st.session_state.import_notice = (
                            "Đã nạp dữ liệu mới. "
                            + (f"Bản sao dữ liệu cũ: data/{backup.name}." if backup else "")
                        )
                        st.rerun()

    st.markdown('<div id="download-data"></div>', unsafe_allow_html=True)
    col1, col2 = st.columns(2, gap="medium")
    with col1:
        with st.container(border=True, key="panel_sample_data"):
            st.subheader("Nạp dữ liệu mẫu")
            st.caption("Dữ liệu mẫu giúp bạn xem thử các chức năng của ứng dụng.")
            st.markdown(
                '<div class="notice"><strong>Lưu ý trước khi tiếp tục</strong>'
                '<p>Thao tác này sẽ thay thế dữ liệu hiện tại bằng bộ dữ liệu demo.</p></div>',
                unsafe_allow_html=True,
            )
            if st.button("⚡ Nạp dữ liệu mẫu", key="load_sample_button"):
                try:
                    result = load_sample()
                    if "accounts" in result:
                        st.session_state.finance_data = result
                        st.success("Đã nạp dữ liệu mẫu.")
                        st.rerun()
                    else:
                        st.error(result.get("message", "Không thể nạp dữ liệu mẫu."))
                except RuntimeError as e:
                    st.error(f"❌ {e}")

    with col2:
        with st.container(border=True, key="panel_export_data"):
            st.subheader("Tải dữ liệu hiện tại")
            st.caption("Xuất dữ liệu theo định dạng CSV dùng bởi ứng dụng C++.")
            st.markdown(
                '<div class="notice"><strong>finance_data.csv</strong>'
                '<p>Gồm tài khoản, ngân sách và giao dịch.</p></div>',
                unsafe_allow_html=True,
            )

            # Tao CSV tu du lieu hien tai
            import io
            buf = io.StringIO()
            buf.write("#ACCOUNTS\nName,Balance\n")
            for acc in data["accounts"]:
                buf.write(f"{acc['name']},{acc['balance']:.2f}\n")
            buf.write("#BUDGETS\nCategory,Limit\n")
            for cat, lim in data["budgets"].items():
                buf.write(f"{cat},{lim:.2f}\n")
            buf.write("#TRANSACTIONS\nType,Id,Date,Amount,Category,Account,Description,Frequency,DueDay\n")
            for tx in data["transactions"]:
                buf.write(f"{tx['type']},{tx['id']},{tx['date']},{tx['amount']:.2f},"
                          f"{tx['category']},{tx['account']},{tx['description']},"
                          f"{tx.get('frequency','')},{tx.get('due_day','')}\n")

            st.download_button(
                "↓ Tải file finance_data.csv",
                data=buf.getvalue().encode("utf-8"),
                file_name="finance_data.csv",
                mime="text/csv",
                key="download_data_button",
            )
