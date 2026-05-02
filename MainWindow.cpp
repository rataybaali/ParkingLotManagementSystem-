// ============================================================
//  MainWindow.cpp  - Qt GUI for Parking Lot Management System
// ============================================================
#include "MainWindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QInputDialog>
#include <QHeaderView>
#include <QSizePolicy>
#include <QScrollArea>
#include <QFont>
#include <QGroupBox>
#include <iomanip>
#include <sstream>
#include <QString>
#include <QSpacerItem>
#include <QSet>
#include <algorithm>

using namespace std;

// ─── Helpers ────────────────────────────────────────────────
static QString toQ(const string& s) { return QString::fromStdString(s); }
static string  toS(const QString& q) { return q.toStdString(); }

static QString fmtMoney(double v) {
    ostringstream ss; ss << fixed << setprecision(2) << v;
    return "Rs. " + QString::fromStdString(ss.str());
}

// ─── Constructor ────────────────────────────────────────────
MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle("Parking Lot Management System");
    setMinimumSize(900, 650);
    applyGlobalStyle();

    stack = new QStackedWidget(this);
    stack->addWidget(makeSetupPage());    // 0
    stack->addWidget(makeLoginPage());    // 1
    stack->addWidget(makeSignupPage());   // 2
    stack->addWidget(makeAdminPage());    // 3
    stack->addWidget(makeCustomerPage()); // 4

    setCentralWidget(stack);

    if (lot.isInitialized())
        stack->setCurrentIndex(PAGE_LOGIN);
    else
        stack->setCurrentIndex(PAGE_SETUP);
}

MainWindow::~MainWindow() { delete currentUser; }

// ─── Style ──────────────────────────────────────────────────
void MainWindow::applyGlobalStyle() {
    setStyleSheet(R"(
        QMainWindow, QWidget { background: #0f1117; color: #e8eaf0; font-family: 'Segoe UI'; font-size: 14px; }
        QLineEdit {
            background: #1a1d27; border: 1.5px solid #2a2d3e; border-radius: 8px;
            padding: 8px 14px; color: #e8eaf0; font-size: 14px;
        }
        QLineEdit:focus { border-color: #4a7cf7; }
        QPushButton {
            border-radius: 8px; padding: 9px 22px; font-size: 14px;
            font-weight: 600; border: none; color: #fff;
        }
        QPushButton:hover { opacity: 0.85; }
        QPushButton:pressed { opacity: 0.7; }
        QTableWidget {
            background: #1a1d27; alternate-background-color: #1e2131;
            border: 1px solid #2a2d3e; border-radius: 8px;
            gridline-color: #2a2d3e; color: #e8eaf0;
        }
        QTableWidget::item { padding: 6px 10px; }
        QTableWidget::item:selected { background: #2d4fa3; }
        QHeaderView::section {
            background: #141720; color: #8899cc; font-weight: 700;
            border: none; border-bottom: 1px solid #2a2d3e; padding: 8px 10px;
        }
        QLabel { color: #e8eaf0; }
        QScrollArea { border: none; }
        QGroupBox {
            border: 1px solid #2a2d3e; border-radius: 10px;
            margin-top: 12px; padding: 14px;
            font-weight: 600; color: #8899cc;
        }
        QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }
        QSpinBox, QDoubleSpinBox {
            background: #1a1d27; border: 1.5px solid #2a2d3e;
            border-radius: 8px; padding: 7px 12px; color: #e8eaf0;
        }
        QComboBox {
            background: #1a1d27; border: 1.5px solid #2a2d3e;
            border-radius: 8px; padding: 7px 14px; color: #e8eaf0;
        }
        QComboBox::drop-down { border: none; }
        QComboBox QAbstractItemView { background: #1a1d27; border: 1px solid #2a2d3e; }
    )");
}

// ─── Common helpers ──────────────────────────────────────────
QLabel* MainWindow::makeTitle(const QString& text) {
    QLabel* lbl = new QLabel(text);
    lbl->setStyleSheet("font-size:24px; font-weight:700; color:#4a7cf7; margin-bottom:8px;");
    lbl->setAlignment(Qt::AlignCenter);
    return lbl;
}

QPushButton* MainWindow::makeBtn(const QString& label, const QString& color) {
    QPushButton* btn = new QPushButton(label);
    btn->setStyleSheet(QString("QPushButton { background:%1; } QPushButton:hover { background:%2; }")
        .arg(color, color + "cc"));
    return btn;
}

QLineEdit* MainWindow::makeInput(const QString& ph, bool pwd) {
    QLineEdit* e = new QLineEdit();
    e->setPlaceholderText(ph);
    if (pwd) e->setEchoMode(QLineEdit::Password);
    return e;
}

QFrame* MainWindow::makeSeparator() {
    QFrame* f = new QFrame();
    f->setFrameShape(QFrame::HLine);
    f->setStyleSheet("color:#2a2d3e;");
    return f;
}

void MainWindow::showMessage(const QString& msg, bool success) {
    QMessageBox mb(this);
    mb.setText(msg);
    mb.setWindowTitle(success ? "Success" : "Error");
    mb.setIcon(success ? QMessageBox::Information : QMessageBox::Warning);
    mb.setStyleSheet("QMessageBox { background:#1a1d27; color:#e8eaf0; }"
        "QLabel { color:#e8eaf0; } QPushButton { background:#2d6cdf; color:#fff; border-radius:6px; padding:6px 18px; }");
    mb.exec();
}

// ─── Setup Page ──────────────────────────────────────────────
QWidget* MainWindow::makeSetupPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* lay = new QVBoxLayout(page);
    lay->setAlignment(Qt::AlignCenter);
    lay->setSpacing(18);

    QLabel* icon = new QLabel("🅿");
    icon->setStyleSheet("font-size:72px;");
    icon->setAlignment(Qt::AlignCenter);

    QLabel* title = makeTitle("Parking Lot Management System");
    QLabel* sub = new QLabel("First-time setup: Enter total parking slots");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color:#8899cc; font-size:15px;");

    QSpinBox* spin = new QSpinBox();
    spin->setMinimum(1); spin->setMaximum(500); spin->setValue(20);
    spin->setFixedWidth(180);
    spin->setAlignment(Qt::AlignCenter);

    QPushButton* btn = makeBtn("Initialize Parking Lot", "#2d6cdf");
    btn->setFixedWidth(240);

    connect(btn, &QPushButton::clicked, [this, spin]() {
        lot.initialize(spin->value());
        stack->setCurrentIndex(PAGE_LOGIN);
        });

    lay->addStretch();
    lay->addWidget(icon);
    lay->addWidget(title);
    lay->addWidget(sub);
    lay->addSpacing(10);
    lay->addWidget(spin, 0, Qt::AlignCenter);
    lay->addWidget(btn, 0, Qt::AlignCenter);
    lay->addStretch();
    return page;
}

// ─── Login Page ──────────────────────────────────────────────
QWidget* MainWindow::makeLoginPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* outer = new QVBoxLayout(page);
    outer->setAlignment(Qt::AlignCenter);

    QFrame* card = new QFrame();
    card->setFixedWidth(400);
    card->setStyleSheet("QFrame { background:#141720; border-radius:14px; padding:30px; border:1px solid #2a2d3e; }");
    QVBoxLayout* lay = new QVBoxLayout(card);
    lay->setSpacing(14);

    QLabel* icon = new QLabel("🅿");
    icon->setStyleSheet("font-size:48px;");
    icon->setAlignment(Qt::AlignCenter);

    QLabel* title = makeTitle("Welcome Back");
    QLabel* sub = new QLabel("Sign in to your account");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color:#8899cc;");

    QLineEdit* edUser = makeInput("Username");
    QLineEdit* edPwd = makeInput("Password", true);

    QPushButton* btnLoginCustomer = makeBtn("Login as Customer", "#2d6cdf");
    QPushButton* btnLoginAdmin = makeBtn("Login as Admin", "#6a3ab0");
    btnLoginAdmin->setStyleSheet("QPushButton { background:#3d1f6e; color:#c9a0ff; border:1px solid #6a3ab0; border-radius:8px; padding:9px 22px; font-size:14px; font-weight:600; } QPushButton:hover { background:#4e28a0; }");

    QPushButton* btnSignup = makeBtn("Sign Up", "#1a4a1a");
    btnSignup->setStyleSheet("QPushButton { background:#1e3a1e; color:#5cba5c; border:1px solid #2d5a2d; border-radius:8px; padding:9px 22px; font-size:14px; font-weight:600; }");

    connect(btnLoginCustomer, &QPushButton::clicked, [this, edUser, edPwd]() {
        string u = toS(edUser->text().trimmed());
        string p = toS(edPwd->text());
        if (u.empty() || p.empty()) { showMessage("Please enter username and password.", false); return; }
        User* user = lot.loginUser(u, p);
        if (!user) { showMessage("Invalid credentials!", false); return; }
        if (user->getRole() == "Admin") {
            delete user;
            showMessage("This is an Admin account.\nPlease use 'Login as Admin'.", false);
            return;
        }
        delete currentUser;
        currentUser = user;
        edUser->clear(); edPwd->clear();
        stack->setCurrentIndex(PAGE_CUSTOMER);
        });

    connect(btnLoginAdmin, &QPushButton::clicked, [this, edUser, edPwd]() {
        string u = toS(edUser->text().trimmed());
        string p = toS(edPwd->text());
        if (u.empty() || p.empty()) { showMessage("Please enter username and password.", false); return; }
        User* user = lot.loginUser(u, p);
        if (!user) { showMessage("Invalid credentials!", false); return; }
        if (user->getRole() != "Admin") {
            delete user;
            showMessage("This is not an Admin account.\nPlease use 'Login as Customer'.", false);
            return;
        }
        delete currentUser;
        currentUser = user;
        edUser->clear(); edPwd->clear();
        refreshAdminStats();
        stack->setCurrentIndex(PAGE_ADMIN);
        });

    connect(btnSignup, &QPushButton::clicked, [this]() { stack->setCurrentIndex(PAGE_SIGNUP); });

    lay->addWidget(icon);
    lay->addWidget(title);
    lay->addWidget(sub);
    lay->addSpacing(8);
    lay->addWidget(edUser);
    lay->addWidget(edPwd);
    lay->addWidget(btnLoginCustomer);
    lay->addWidget(btnLoginAdmin);
    lay->addWidget(makeSeparator());
    lay->addWidget(btnSignup);

    outer->addWidget(card, 0, Qt::AlignCenter);
    return page;
}

// ─── Signup Page ─────────────────────────────────────────────
QWidget* MainWindow::makeSignupPage() {
    QWidget* page = new QWidget();
    QVBoxLayout* outer = new QVBoxLayout(page);
    outer->setAlignment(Qt::AlignCenter);

    QFrame* card = new QFrame();
    card->setFixedWidth(400);
    card->setStyleSheet("QFrame { background:#141720; border-radius:14px; padding:30px; border:1px solid #2a2d3e; }");
    QVBoxLayout* lay = new QVBoxLayout(card);
    lay->setSpacing(14);

    QLabel* title = makeTitle("Create Account");
    QLineEdit* edUser = makeInput("Username");
    QLineEdit* edPwd = makeInput("Password", true);
    QLineEdit* edPwd2 = makeInput("Confirm Password", true);

    QComboBox* cmbRole = new QComboBox();
    cmbRole->addItems({ "Customer", "Admin" });

    QPushButton* btnCreate = makeBtn("Create Account", "#2d6cdf");
    QPushButton* btnBack = makeBtn("Back to Login", "#222");
    btnBack->setStyleSheet("QPushButton { background:#1e1e2e; color:#8899cc; border:1px solid #2a2d3e; border-radius:8px; padding:9px 22px; font-size:14px; }");

    connect(btnCreate, &QPushButton::clicked, [this, edUser, edPwd, edPwd2, cmbRole]() {
        string u = toS(edUser->text().trimmed());
        string p = toS(edPwd->text());
        string p2 = toS(edPwd2->text());
        if (u.empty() || p.empty()) { showMessage("Username and password required.", false); return; }
        if (p != p2) { showMessage("Passwords do not match!", false); return; }
        string role = toS(cmbRole->currentText());
        string msg;
        if (lot.signupUser(u, p, role, &msg)) {
            showMessage(toQ(msg), true);
            edUser->clear(); edPwd->clear(); edPwd2->clear();
            stack->setCurrentIndex(PAGE_LOGIN);
        }
        else {
            showMessage(toQ(msg), false);
        }
        });

    connect(btnBack, &QPushButton::clicked, [this]() { stack->setCurrentIndex(PAGE_LOGIN); });

    lay->addWidget(title);
    lay->addSpacing(4);
    lay->addWidget(edUser);
    lay->addWidget(edPwd);
    lay->addWidget(edPwd2);
    lay->addWidget(cmbRole);
    lay->addSpacing(6);
    lay->addWidget(btnCreate);
    lay->addWidget(btnBack);

    outer->addWidget(card, 0, Qt::AlignCenter);
    return page;
}

// ─── Admin Page ──────────────────────────────────────────────
QWidget* MainWindow::makeAdminPage() {
    QWidget* page = new QWidget();
    QHBoxLayout* mainLay = new QHBoxLayout(page);
    mainLay->setSpacing(0);
    mainLay->setContentsMargins(0, 0, 0, 0);

    // Sidebar
    QFrame* sidebar = new QFrame();
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet("QFrame { background:#0a0c14; border-right:1px solid #1e2035; }");
    QVBoxLayout* sbLay = new QVBoxLayout(sidebar);
    sbLay->setContentsMargins(12, 24, 12, 24);
    sbLay->setSpacing(8);

    QLabel* logo = new QLabel("🅿 Admin");
    logo->setStyleSheet("font-size:22px; font-weight:700; color:#4a7cf7; padding:8px;");

    lblTotal = new QLabel("Total: --");
    lblOccupied = new QLabel("Occupied: --");
    lblFree = new QLabel("Free: --");
    lblRevenue = new QLabel("Revenue: --");
    for (QLabel* l : { lblTotal, lblOccupied, lblFree, lblRevenue })
        l->setStyleSheet("color:#8899cc; font-size:12px; padding:2px 8px;");

    auto makeSideBtn = [](const QString& label, const QString& icon) {
        QPushButton* b = new QPushButton(icon + "  " + label);
        b->setStyleSheet(R"(
            QPushButton { background:transparent; color:#c8d0e8; text-align:left;
                padding:10px 14px; border-radius:8px; font-size:13px; }
            QPushButton:hover { background:#1e2240; color:#fff; }
            QPushButton:pressed { background:#2d3560; }
        )");
        return b;
        };

    QPushButton* btnSlots = makeSideBtn("Slot Status", "🅿");
    QPushButton* btnHist = makeSideBtn("All History", "📋");
    QPushButton* btnRev = makeSideBtn("Revenue", "💰");
    QPushButton* btnRate = makeSideBtn("Update Rate", "⚙");
    QPushButton* btnSearch = makeSideBtn("Search Vehicle", "🔍");
    QPushButton* btnUsers = makeSideBtn("Manage Users", "👤");

    sbLay->addWidget(logo);
    sbLay->addWidget(makeSeparator());
    sbLay->addWidget(lblTotal);
    sbLay->addWidget(lblOccupied);
    sbLay->addWidget(lblFree);
    sbLay->addWidget(lblRevenue);
    sbLay->addWidget(makeSeparator());
    sbLay->addWidget(btnSlots);
    sbLay->addWidget(btnHist);
    sbLay->addWidget(btnRev);
    sbLay->addWidget(btnRate);
    sbLay->addWidget(btnSearch);
    sbLay->addWidget(btnUsers);
    sbLay->addStretch();

    QPushButton* btnLogout = makeBtn("Logout", "#4a1a1a");
    btnLogout->setStyleSheet("QPushButton { background:#2d1010; color:#e05555; border:1px solid #4a2020; border-radius:8px; padding:9px; font-size:13px; } QPushButton:hover { background:#3d1818; }");
    sbLay->addWidget(btnLogout);
    connect(btnLogout, &QPushButton::clicked, this, &MainWindow::onLogout);

    // Content area (stacked)
    QStackedWidget* content = new QStackedWidget();
    content->setContentsMargins(24, 24, 24, 24);

    QWidget* panSlots = makeAdminSlotsPanel();
    QWidget* panHist = makeAdminHistoryPanel();
    QWidget* panRev = makeAdminRevenuePanel();
    QWidget* panRate = makeAdminRatePanel();
    QWidget* panSearch = makeAdminSearchPanel();
    QWidget* panUsers = makeAdminUsersPanel();

    content->addWidget(panSlots);   // 0
    content->addWidget(panHist);    // 1
    content->addWidget(panRev);     // 2
    content->addWidget(panRate);    // 3
    content->addWidget(panSearch);  // 4
    content->addWidget(panUsers);   // 5

    connect(btnSlots, &QPushButton::clicked, [this, content]() { refreshAdminStats(); content->setCurrentIndex(0); onAdminViewSlots(); });
    connect(btnHist, &QPushButton::clicked, [this, content]() { content->setCurrentIndex(1); onAdminViewHistory(); });
    connect(btnRev, &QPushButton::clicked, [this, content]() { refreshAdminStats(); content->setCurrentIndex(2); });
    connect(btnRate, &QPushButton::clicked, [content]() { content->setCurrentIndex(3); });
    connect(btnSearch, &QPushButton::clicked, [content]() { content->setCurrentIndex(4); });
    connect(btnUsers, &QPushButton::clicked, [this, content]() { content->setCurrentIndex(5); onAdminDeleteUser(); });

    mainLay->addWidget(sidebar);
    mainLay->addWidget(content, 1);
    return page;
}

void MainWindow::refreshAdminStats() {
    if (!lblTotal) return;
    lblTotal->setText("Total: " + QString::number(lot.getTotalSlots()));
    lblOccupied->setText("Occupied: " + QString::number(lot.getOccupiedSlots()));
    lblFree->setText("Free: " + QString::number(lot.getAvailableSlots()));
    lblRevenue->setText("Revenue: " + fmtMoney(lot.getTotalRevenue()));
}

// ─── Admin Panels ────────────────────────────────────────────
QWidget* MainWindow::makeAdminSlotsPanel() {
    QWidget* w = new QWidget();
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->addWidget(makeTitle("🅿 Slot Status"));

    QScrollArea* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    slotsGridWidget = new QWidget();
    scroll->setWidget(slotsGridWidget);
    lay->addWidget(scroll);

    QPushButton* btnRefresh = makeBtn("Refresh", "#1e4d1e");
    connect(btnRefresh, &QPushButton::clicked, [this]() {
        refreshAdminStats();
        refreshSlotsGrid(slotsGridWidget);
        });
    lay->addWidget(btnRefresh, 0, Qt::AlignLeft);
    return w;
}

void MainWindow::refreshSlotsGrid(QWidget* container) {
    // Clear old layout
    QLayout* old = container->layout();
    if (old) {
        QLayoutItem* item;
        while ((item = old->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete old;
    }

    QGridLayout* grid = new QGridLayout(container);
    grid->setSpacing(10);

    const int MAX_SLOTS = 1000;
    SlotSnapshot snaps[MAX_SLOTS];
    int count = lot.getAllSlotsSnapshot(snaps, MAX_SLOTS);

    int cols = 5;
    for (int i = 0; i < count; i++) {
        SlotSnapshot snap = snaps[i];
        QFrame* cell = new QFrame();
        cell->setFixedSize(130, 90);
        QString bg = snap.occupied ? "#2d1010" : "#0d2d0d";
        QString border = snap.occupied ? "#c0392b" : "#27ae60";
        cell->setStyleSheet(QString("QFrame { background:%1; border:2px solid %2; border-radius:10px; }").arg(bg, border));

        QVBoxLayout* cl = new QVBoxLayout(cell);
        cl->setSpacing(3);

        QLabel* num = new QLabel("Slot " + QString::number(snap.slotNumber));
        num->setStyleSheet("font-weight:700; font-size:13px;");
        num->setAlignment(Qt::AlignCenter);

        QLabel* status = new QLabel(snap.occupied ? "OCCUPIED" : "FREE");
        status->setStyleSheet(QString("color:%1; font-size:11px; font-weight:600;").arg(snap.occupied ? "#e05555" : "#55cc77"));
        status->setAlignment(Qt::AlignCenter);

        cl->addWidget(num);
        cl->addWidget(status);

        if (snap.occupied) {
            QLabel* vnum = new QLabel(toQ(snap.vehicleNumber));
            vnum->setStyleSheet("color:#cc9944; font-size:11px;");
            vnum->setAlignment(Qt::AlignCenter);
            vnum->setWordWrap(true);
            cl->addWidget(vnum);
        }

        grid->addWidget(cell, i / cols, i % cols);
    }
}

QWidget* MainWindow::makeAdminHistoryPanel() {
    QWidget* w = new QWidget();
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->addWidget(makeTitle("📋 All Parking History"));

    // ── Date filter row ──
    QHBoxLayout* filterRow = new QHBoxLayout();
    QLabel* lblEntry = new QLabel("Entry Date:");
    lblEntry->setStyleSheet("color:#8899cc; font-size:13px;");
    QComboBox* cmbEntry = new QComboBox();
    cmbEntry->setMinimumWidth(180);
    QLabel* lblExit = new QLabel("Exit Date:");
    lblExit->setStyleSheet("color:#8899cc; font-size:13px;");
    QComboBox* cmbExit = new QComboBox();
    cmbExit->setMinimumWidth(180);
    QPushButton* btnApply = makeBtn("Apply Filter", "#2d6cdf");
    btnApply->setFixedWidth(120);
    QPushButton* btnClear = makeBtn("Clear", "#333");
    btnClear->setStyleSheet("QPushButton { background:#1e1e2e; color:#8899cc; border:1px solid #2a2d3e; border-radius:8px; padding:9px 16px; font-size:13px; }");
    btnClear->setFixedWidth(80);
    filterRow->addWidget(lblEntry);
    filterRow->addWidget(cmbEntry);
    filterRow->addSpacing(16);
    filterRow->addWidget(lblExit);
    filterRow->addWidget(cmbExit);
    filterRow->addSpacing(12);
    filterRow->addWidget(btnApply);
    filterRow->addWidget(btnClear);
    filterRow->addStretch();
    lay->addLayout(filterRow);

    QTableWidget* tbl = new QTableWidget(0, 8);
    tbl->setHorizontalHeaderLabels({ "User","Vehicle","Type","Slot","Entry","Exit","Hours","Fee (Rs.)" });
    tbl->horizontalHeader()->setStretchLastSection(true);
    tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    tbl->setColumnWidth(0, 100);  // User
    tbl->setColumnWidth(1, 110);  // Vehicle
    tbl->setColumnWidth(2, 70);  // Type
    tbl->setColumnWidth(3, 55);  // Slot
    tbl->setColumnWidth(4, 160);  // Entry  — wide enough for full timestamp
    tbl->setColumnWidth(5, 160);  // Exit
    tbl->setColumnWidth(6, 70);  // Hours
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->setAlternatingRowColors(true);
    tbl->setObjectName("histTable");
    lay->addWidget(tbl);

    // Helper: extract date part (first 10 chars of "YYYY-MM-DD ...")
    auto dateOf = [](const QString& dt) -> QString {
        return dt.length() >= 10 ? dt.left(10) : dt;
        };

    // Load all records and populate table + dropdowns
    auto loadAll = [this, tbl, cmbEntry, cmbExit, dateOf]() {
        const int MAX_RECS = 2000;
        ParkingRecord recs[MAX_RECS];
        int count = lot.getAllHistory(recs, MAX_RECS);

        tbl->setRowCount(0);
        QSet<QString> entryDates, exitDates;

        for (int i = 0; i < count; i++) {
            const ParkingRecord& r = recs[i];
            int row = tbl->rowCount();
            tbl->insertRow(row);
            ostringstream fee; fee << fixed << setprecision(2) << r.fee;
            ostringstream dur; dur << fixed << setprecision(2) << r.duration;
            tbl->setItem(row, 0, new QTableWidgetItem(toQ(r.username)));
            tbl->setItem(row, 1, new QTableWidgetItem(toQ(r.vehicleNumber)));
            tbl->setItem(row, 2, new QTableWidgetItem(toQ(r.vehicleType)));
            tbl->setItem(row, 3, new QTableWidgetItem(QString::number(r.slotNumber)));
            tbl->setItem(row, 4, new QTableWidgetItem(toQ(r.entryTime)));
            tbl->setItem(row, 5, new QTableWidgetItem(toQ(r.exitTime)));
            tbl->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(dur.str())));
            tbl->setItem(row, 7, new QTableWidgetItem(QString::fromStdString(fee.str())));
            entryDates.insert(dateOf(toQ(r.entryTime)));
            exitDates.insert(dateOf(toQ(r.exitTime)));
        }

        // Rebuild combo boxes
        QString prevEntry = cmbEntry->currentText();
        QString prevExit = cmbExit->currentText();
        cmbEntry->blockSignals(true); cmbExit->blockSignals(true);
        cmbEntry->clear(); cmbExit->clear();
        cmbEntry->addItem("All Dates"); cmbExit->addItem("All Dates");
        QList<QString> eList = entryDates.values(); std::sort(eList.begin(), eList.end());
        QList<QString> xList = exitDates.values();  std::sort(xList.begin(), xList.end());
        for (const auto& d : eList) cmbEntry->addItem(d);
        for (const auto& d : xList) cmbExit->addItem(d);
        int ei = cmbEntry->findText(prevEntry); if (ei >= 0) cmbEntry->setCurrentIndex(ei);
        int xi = cmbExit->findText(prevExit);   if (xi >= 0) cmbExit->setCurrentIndex(xi);
        cmbEntry->blockSignals(false); cmbExit->blockSignals(false);
        };

    // Apply date filter
    auto applyFilter = [tbl, cmbEntry, cmbExit, dateOf]() {
        QString entryFilter = cmbEntry->currentText();
        QString exitFilter = cmbExit->currentText();
        for (int row = 0; row < tbl->rowCount(); ++row) {
            bool entryMatch = (entryFilter == "All Dates") ||
                (tbl->item(row, 4) && dateOf(tbl->item(row, 4)->text()) == entryFilter);
            bool exitMatch = (exitFilter == "All Dates") ||
                (tbl->item(row, 5) && dateOf(tbl->item(row, 5)->text()) == exitFilter);
            tbl->setRowHidden(row, !(entryMatch && exitMatch));
        }
        };

    QPushButton* btnRefresh = makeBtn("Refresh", "#1e3d4d");
    connect(btnRefresh, &QPushButton::clicked, loadAll);
    connect(btnApply, &QPushButton::clicked, applyFilter);
    connect(btnClear, &QPushButton::clicked, [cmbEntry, cmbExit, applyFilter]() {
        cmbEntry->setCurrentIndex(0);
        cmbExit->setCurrentIndex(0);
        applyFilter();
        });

    lay->addWidget(btnRefresh, 0, Qt::AlignLeft);
    return w;
}

QWidget* MainWindow::makeAdminRevenuePanel() {
    QWidget* w = new QWidget();
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setAlignment(Qt::AlignCenter);
    lay->addWidget(makeTitle("💰 Revenue Overview"));
    lay->addStretch();

    QFrame* card = new QFrame();
    card->setFixedSize(400, 260);
    card->setStyleSheet("QFrame { background:#141720; border:2px solid #2d5a2d; border-radius:16px; }");
    QVBoxLayout* cl = new QVBoxLayout(card);
    cl->setAlignment(Qt::AlignCenter);
    cl->setSpacing(8);

    QLabel* lbl1 = new QLabel("Total Revenue Collected");
    lbl1->setStyleSheet("color:#8899cc; font-size:15px;");
    lbl1->setAlignment(Qt::AlignCenter);

    QLabel* lblAmount = new QLabel(fmtMoney(lot.getTotalRevenue()));
    lblAmount->setStyleSheet("color:#27ae60; font-size:38px; font-weight:700;");
    lblAmount->setAlignment(Qt::AlignCenter);

    QFrame* sep = makeSeparator();
    QLabel* lblRates = new QLabel(
        "🚗 Car: " + fmtMoney(lot.getRateForType("Car")) + "/hr    "
        "🏍 Bike: " + fmtMoney(lot.getRateForType("Bike")) + "/hr    "
        "🚚 Truck: " + fmtMoney(lot.getRateForType("Truck")) + "/hr"
    );
    lblRates->setStyleSheet("color:#cc9944; font-size:13px;");
    lblRates->setAlignment(Qt::AlignCenter);

    cl->addWidget(lbl1);
    cl->addWidget(lblAmount);
    cl->addWidget(sep);
    cl->addWidget(lblRates);

    lay->addWidget(card, 0, Qt::AlignCenter);
    lay->addStretch();
    return w;
}

QWidget* MainWindow::makeAdminRatePanel() {
    QWidget* w = new QWidget();
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->setAlignment(Qt::AlignCenter);
    lay->addWidget(makeTitle("⚙ Update Hourly Rates"));
    lay->addStretch();

    // Info note
    QLabel* note = new QLabel("ℹ  Rate changes only apply to NEW vehicles.\n    Vehicles already parked keep the rate locked at entry time.");
    note->setStyleSheet("color:#cc9944; font-size:13px; background:#1a1500; border:1px solid #4a3a00; border-radius:8px; padding:10px 14px;");
    note->setAlignment(Qt::AlignLeft);
    note->setWordWrap(true);
    lay->addWidget(note);
    lay->addSpacing(10);

    // Helper to build one rate row card
    auto makeRateCard = [&](const QString& type, const QString& icon, const QString& color, double currentRate) -> QFrame* {
        QFrame* card = new QFrame();
        card->setFixedWidth(420);
        card->setStyleSheet(QString(".QFrame { background:#141720; border:1px solid %1; border-radius:14px; padding:20px; }").arg(color));
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setSpacing(10);

        QHBoxLayout* titleRow = new QHBoxLayout();
        QLabel* lIcon = new QLabel(icon + "  " + type);
        lIcon->setStyleSheet(QString("color:%1; font-size:17px; font-weight:700;").arg(color));
        QLabel* lCurr = new QLabel("Current: " + fmtMoney(currentRate) + "/hr");
        lCurr->setStyleSheet("color:#8899cc; font-size:13px;");
        lCurr->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        titleRow->addWidget(lIcon);
        titleRow->addStretch();
        titleRow->addWidget(lCurr);
        cl->addLayout(titleRow);

        QHBoxLayout* inputRow = new QHBoxLayout();
        QLineEdit* edRate = makeInput("New rate (Rs./hr)");
        edRate->setObjectName("rate_" + type);
        QPushButton* btnSet = makeBtn("Update", color);
        btnSet->setFixedWidth(100);

        connect(btnSet, &QPushButton::clicked, [this, type, edRate, lCurr]() {
            bool ok;
            double rate = edRate->text().toDouble(&ok);
            if (!ok || rate <= 0) { showMessage("Enter a valid rate for " + type + ".", false); return; }
            lot.setRateForType(toS(type), rate);
            lCurr->setText("Current: " + fmtMoney(rate) + "/hr");
            edRate->clear();
            showMessage(type + " rate updated to " + fmtMoney(rate) + "/hr\n(Only affects new vehicles parked after this change.)", true);
            });

        inputRow->addWidget(edRate);
        inputRow->addWidget(btnSet);
        cl->addLayout(inputRow);
        return card;
        };

    lay->addWidget(makeRateCard("Car", "🚗", "#2d6cdf", lot.getRateForType("Car")), 0, Qt::AlignCenter);
    lay->addSpacing(10);
    lay->addWidget(makeRateCard("Bike", "🏍", "#27ae60", lot.getRateForType("Bike")), 0, Qt::AlignCenter);
    lay->addSpacing(10);
    lay->addWidget(makeRateCard("Truck", "🚚", "#cc9944", lot.getRateForType("Truck")), 0, Qt::AlignCenter);

    lay->addStretch();
    return w;
}

QWidget* MainWindow::makeAdminSearchPanel() {
    QWidget* w = new QWidget();
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->addWidget(makeTitle("🔍 Search Vehicle"));

    QHBoxLayout* row = new QHBoxLayout();
    QLineEdit* edVnum = makeInput("Enter Vehicle Number");
    QPushButton* btnSearch = makeBtn("Search", "#2d6cdf");
    row->addWidget(edVnum);
    row->addWidget(btnSearch);
    lay->addLayout(row);

    QFrame* resultCard = new QFrame();
    resultCard->setStyleSheet("QFrame { background:#141720; border:1px solid #2a2d3e; border-radius:12px; padding:20px; }");
    resultCard->setVisible(false);
    QVBoxLayout* rl = new QVBoxLayout(resultCard);

    QLabel* lblResult = new QLabel();
    lblResult->setWordWrap(true);
    lblResult->setStyleSheet("font-size:14px; line-height:1.8;");
    rl->addWidget(lblResult);
    lay->addWidget(resultCard);
    lay->addStretch();

    connect(btnSearch, &QPushButton::clicked, [this, edVnum, resultCard, lblResult]() {
        string vnum = toS(edVnum->text().trimmed());
        if (vnum.empty()) { showMessage("Enter vehicle number.", false); return; }
        SlotSnapshot found;
        string msg;
        bool ok = lot.searchVehicle(vnum, found, &msg);
        resultCard->setVisible(true);
        if (ok) {
            lblResult->setText(
                "<b style='color:#4a7cf7;'>Vehicle Found!</b><br>"
                "<b>Slot:</b> " + QString::number(found.slotNumber) + "<br>"
                "<b>Vehicle No:</b> " + toQ(found.vehicleNumber) + "<br>"
                "<b>Type:</b> " + toQ(found.vehicleType) + "<br>"
                "<b>Entry:</b> " + toQ(found.entryTime)
            );
        }
        else {
            lblResult->setText("<span style='color:#e05555;'>❌ " + toQ(msg) + "</span>");
        }
        });

    return w;
}

QWidget* MainWindow::makeAdminUsersPanel() {
    QWidget* w = new QWidget();
    QVBoxLayout* lay = new QVBoxLayout(w);
    lay->addWidget(makeTitle("👤 Manage Users"));

    QTableWidget* tbl = new QTableWidget(0, 1);
    tbl->setHorizontalHeaderLabels({ "Username" });
    tbl->horizontalHeader()->setStretchLastSection(true);
    tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    tbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    lay->addWidget(tbl);

    // Populate
    auto refreshUsers = [tbl, this]() {
        const int MAX_U = 1000;
        string users[MAX_U];
        int count = lot.getAllUsernames(users, MAX_U);

        tbl->setRowCount(0);
        for (int i = 0; i < count; i++) {
            int r = tbl->rowCount(); tbl->insertRow(r);
            tbl->setItem(r, 0, new QTableWidgetItem(toQ(users[i])));
        }
        };
    refreshUsers();

    QHBoxLayout* btnRow = new QHBoxLayout();
    QPushButton* btnRefresh = makeBtn("Refresh", "#1e3d4d");
    QPushButton* btnDelete = makeBtn("Delete Selected", "#4a1a1a");
    btnDelete->setStyleSheet("QPushButton { background:#2d1010; color:#e05555; border:1px solid #4a2020; border-radius:8px; padding:9px 22px; font-size:14px; font-weight:600; }");

    connect(btnRefresh, &QPushButton::clicked, refreshUsers);
    connect(btnDelete, &QPushButton::clicked, [this, tbl, refreshUsers]() {
        auto sel = tbl->selectedItems();
        if (sel.isEmpty()) { showMessage("Select a user first.", false); return; }
        QString uname = sel[0]->text();
        string msg;
        if (lot.deleteUser(toS(uname), &msg)) {
            showMessage(toQ(msg), true);
            refreshUsers();
        }
        else {
            showMessage(toQ(msg), false);
        }
        });

    btnRow->addWidget(btnRefresh);
    btnRow->addWidget(btnDelete);
    btnRow->addStretch();
    lay->addLayout(btnRow);
    return w;
}

// ─── Customer Page ───────────────────────────────────────────
QWidget* MainWindow::makeCustomerPage() {
    QWidget* page = new QWidget();
    QHBoxLayout* mainLay = new QHBoxLayout(page);
    mainLay->setSpacing(0);
    mainLay->setContentsMargins(0, 0, 0, 0);

    // Sidebar
    QFrame* sidebar = new QFrame();
    sidebar->setFixedWidth(220);
    sidebar->setStyleSheet("QFrame { background:#0a0c14; border-right:1px solid #1e2035; }");
    QVBoxLayout* sbLay = new QVBoxLayout(sidebar);
    sbLay->setContentsMargins(12, 24, 12, 24);
    sbLay->setSpacing(8);

    QLabel* logo = new QLabel("🅿 Customer");
    logo->setStyleSheet("font-size:20px; font-weight:700; color:#27ae60; padding:8px;");

    auto makeSideBtn = [](const QString& label, const QString& icon) {
        QPushButton* b = new QPushButton(icon + "  " + label);
        b->setStyleSheet(R"(
            QPushButton { background:transparent; color:#c8d0e8; text-align:left;
                padding:10px 14px; border-radius:8px; font-size:13px; }
            QPushButton:hover { background:#1e2240; color:#fff; }
        )");
        return b;
        };

    QPushButton* btnSlots = makeSideBtn("Available Slots", "🅿");
    QPushButton* btnPark = makeSideBtn("Park Vehicle", "🚗");
    QPushButton* btnCheckout = makeSideBtn("Checkout", "💳");
    QPushButton* btnHistory = makeSideBtn("My History", "📋");

    sbLay->addWidget(logo);
    sbLay->addWidget(makeSeparator());
    sbLay->addWidget(btnSlots);
    sbLay->addWidget(btnPark);
    sbLay->addWidget(btnCheckout);
    sbLay->addWidget(btnHistory);
    sbLay->addStretch();

    QPushButton* btnLogout = makeBtn("Logout", "#4a1a1a");
    btnLogout->setStyleSheet("QPushButton { background:#2d1010; color:#e05555; border:1px solid #4a2020; border-radius:8px; padding:9px; font-size:13px; } QPushButton:hover { background:#3d1818; }");
    sbLay->addWidget(btnLogout);
    connect(btnLogout, &QPushButton::clicked, this, &MainWindow::onLogout);

    // Content
    QStackedWidget* content = new QStackedWidget();
    content->setContentsMargins(24, 24, 24, 24);

    // 0: Available slots
    QWidget* panSlots = new QWidget();
    {
        QVBoxLayout* lay = new QVBoxLayout(panSlots);
        lay->addWidget(makeTitle("🅿 Available Slots"));
        QTableWidget* tbl = new QTableWidget(0, 2);
        tbl->setHorizontalHeaderLabels({ "Slot #", "Status" });
        tbl->horizontalHeader()->setStretchLastSection(true);
        tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
        lay->addWidget(tbl);
        QPushButton* btnRef = makeBtn("Refresh", "#1e3d4d");
        connect(btnRef, &QPushButton::clicked, [this, tbl]() {
            const int MAX_S = 1000;
            SlotSnapshot snaps[MAX_S];
            int count = lot.getAllSlotsSnapshot(snaps, MAX_S);

            tbl->setRowCount(0);
            for (int i = 0; i < count; i++) {
                SlotSnapshot sn = snaps[i];
                if (!sn.occupied) {
                    int r = tbl->rowCount(); tbl->insertRow(r);
                    tbl->setItem(r, 0, new QTableWidgetItem("Slot " + QString::number(sn.slotNumber)));
                    QTableWidgetItem* st = new QTableWidgetItem("FREE");
                    st->setForeground(QColor("#27ae60"));
                    tbl->setItem(r, 1, st);
                }
            }
            });
        lay->addWidget(btnRef, 0, Qt::AlignLeft);
    }

    // 1: Park vehicle
    QWidget* panPark = new QWidget();
    {
        QVBoxLayout* lay = new QVBoxLayout(panPark);
        lay->setAlignment(Qt::AlignTop);
        lay->addWidget(makeTitle("🚗 Park My Vehicle"));

        QFrame* card = new QFrame();
        card->setFixedWidth(400);
        card->setStyleSheet("QFrame { background:#141720; border:1px solid #2a2d3e; border-radius:14px; padding:24px; }");
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setSpacing(12);

        QLineEdit* edVnum = makeInput("Vehicle Number (e.g. ABC-1234)");
        QComboBox* cmbType = new QComboBox();
        cmbType->addItems({ "Car", "Bike", "Truck" });

        QPushButton* btnParkNow = makeBtn("Park Vehicle ✓", "#1e5c1e");
        btnParkNow->setStyleSheet("QPushButton { background:#1e5c1e; color:#7eff7e; border-radius:8px; padding:10px 22px; font-size:14px; font-weight:600; } QPushButton:hover { background:#2a7a2a; }");

        QLabel* lblResult2 = new QLabel();
        lblResult2->setWordWrap(true);
        lblResult2->setAlignment(Qt::AlignCenter);

        connect(btnParkNow, &QPushButton::clicked, [this, edVnum, cmbType, lblResult2]() {
            if (!currentUser) return;
            string vnum = toS(edVnum->text().trimmed());
            string vtype = toS(cmbType->currentText());
            if (vnum.empty()) { showMessage("Enter vehicle number.", false); return; }
            if (lot.getAvailableSlots() == 0) { showMessage("Parking lot is full!", false); return; }
            int slot = -1;
            string msg;
            if (lot.parkVehicle(currentUser->getUsername(), vnum, vtype, slot, &msg)) {
                lblResult2->setText("<span style='color:#27ae60;'>✓ " + toQ(msg) + "<br>Please remember your slot number!</span>");
                edVnum->clear();
            }
            else {
                lblResult2->setText("<span style='color:#e05555;'>❌ " + toQ(msg) + "</span>");
            }
            });

        cl->addWidget(new QLabel("Vehicle Number:")); cl->addWidget(edVnum);
        cl->addWidget(new QLabel("Vehicle Type:")); cl->addWidget(cmbType);
        cl->addSpacing(6);
        cl->addWidget(btnParkNow);
        cl->addWidget(lblResult2);
        lay->addWidget(card, 0, Qt::AlignHCenter);
    }

    // 2: Checkout
    QWidget* panCheckout = new QWidget();
    {
        QVBoxLayout* lay = new QVBoxLayout(panCheckout);
        lay->setAlignment(Qt::AlignTop);
        lay->addWidget(makeTitle("💳 Checkout"));

        QFrame* card = new QFrame();
        card->setFixedWidth(440);
        card->setStyleSheet("QFrame { background:#141720; border:1px solid #2a2d3e; border-radius:14px; padding:24px; }");
        QVBoxLayout* cl = new QVBoxLayout(card);
        cl->setSpacing(12);

        QLineEdit* edSlot = makeInput("Enter Slot Number");

        QPushButton* btnCO = makeBtn("Checkout & Get Receipt", "#2d6cdf");
        QLabel* receipt = new QLabel();
        receipt->setWordWrap(true);
        receipt->setStyleSheet("background:#0a0c14; border:1px solid #2a2d3e; border-radius:8px; padding:14px; font-size:13px; line-height:1.9;");
        receipt->setVisible(false);

        connect(btnCO, &QPushButton::clicked, [this, edSlot, receipt]() {
            if (!currentUser) return;
            bool ok;
            int slotNum = edSlot->text().toInt(&ok);
            if (!ok || slotNum <= 0) { showMessage("Enter a valid slot number.", false); return; }
            ParkingRecord rec;
            double fee = 0;
            string msg;
            if (lot.checkoutVehicle(currentUser->getUsername(), slotNum, rec, fee, &msg)) {
                ostringstream feeStr; feeStr << fixed << setprecision(2) << rec.fee;
                ostringstream durStr; durStr << fixed << setprecision(2) << rec.duration;
                ostringstream rateStr; rateStr << fixed << setprecision(2) << rec.lockedRate;
                receipt->setText(
                    "<b style='color:#4a7cf7;'>━━━ RECEIPT ━━━</b><br>"
                    "<b>Vehicle:</b> " + toQ(rec.vehicleNumber) + "<br>"
                    "<b>Type:</b> " + toQ(rec.vehicleType) + "<br>"
                    "<b>Slot:</b> " + QString::number(rec.slotNumber) + "<br>"
                    "<b>Entry:</b> " + toQ(rec.entryTime) + "<br>"
                    "<b>Exit:</b> " + toQ(rec.exitTime) + "<br>"
                    "<b>Duration:</b> " + QString::fromStdString(durStr.str()) + " hrs<br>"
                    "<b>Rate (locked at entry):</b> Rs. " + QString::fromStdString(rateStr.str()) + "/hr<br>"
                    "─────────────────<br>"
                    "<b style='color:#27ae60; font-size:16px;'>TOTAL: Rs. " + QString::fromStdString(feeStr.str()) + "</b>"
                );
                receipt->setVisible(true);
                edSlot->clear();
            }
            else {
                showMessage(toQ(msg), false);
            }
            });

        cl->addWidget(new QLabel("Slot Number:"));
        cl->addWidget(edSlot);
        cl->addSpacing(6);
        cl->addWidget(btnCO);
        cl->addWidget(receipt);
        lay->addWidget(card, 0, Qt::AlignHCenter);
    }

    // 3: My History
    QWidget* panHistory = new QWidget();
    {
        QVBoxLayout* lay = new QVBoxLayout(panHistory);
        lay->addWidget(makeTitle("📋 My Parking History"));

        // ── Date filter row ──
        QHBoxLayout* filterRow = new QHBoxLayout();
        QLabel* lblEntry = new QLabel("Entry Date:");
        lblEntry->setStyleSheet("color:#8899cc; font-size:13px;");
        QComboBox* cmbEntry = new QComboBox();
        cmbEntry->setMinimumWidth(160);
        QLabel* lblExit = new QLabel("Exit Date:");
        lblExit->setStyleSheet("color:#8899cc; font-size:13px;");
        QComboBox* cmbExit = new QComboBox();
        cmbExit->setMinimumWidth(160);
        QPushButton* btnApply = makeBtn("Apply Filter", "#2d6cdf");
        btnApply->setFixedWidth(120);
        QPushButton* btnClear2 = makeBtn("Clear", "#333");
        btnClear2->setStyleSheet("QPushButton { background:#1e1e2e; color:#8899cc; border:1px solid #2a2d3e; border-radius:8px; padding:9px 16px; font-size:13px; }");
        btnClear2->setFixedWidth(80);
        filterRow->addWidget(lblEntry);
        filterRow->addWidget(cmbEntry);
        filterRow->addSpacing(16);
        filterRow->addWidget(lblExit);
        filterRow->addWidget(cmbExit);
        filterRow->addSpacing(12);
        filterRow->addWidget(btnApply);
        filterRow->addWidget(btnClear2);
        filterRow->addStretch();
        lay->addLayout(filterRow);

        QTableWidget* tbl = new QTableWidget(0, 7);
        tbl->setHorizontalHeaderLabels({ "Vehicle","Type","Slot","Entry","Exit","Hours","Fee (Rs.)" });
        tbl->horizontalHeader()->setStretchLastSection(true);
        tbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        tbl->setColumnWidth(0, 120);  // Vehicle
        tbl->setColumnWidth(1, 70);  // Type
        tbl->setColumnWidth(2, 55);  // Slot
        tbl->setColumnWidth(3, 160);  // Entry — full timestamp
        tbl->setColumnWidth(4, 160);  // Exit
        tbl->setColumnWidth(5, 70);  // Hours
        tbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
        tbl->setAlternatingRowColors(true);
        lay->addWidget(tbl);

        auto dateOf = [](const QString& dt) -> QString {
            return dt.length() >= 10 ? dt.left(10) : dt;
            };

        auto applyFilter = [tbl, cmbEntry, cmbExit, dateOf]() {
            QString entryFilter = cmbEntry->currentText();
            QString exitFilter = cmbExit->currentText();
            for (int row = 0; row < tbl->rowCount(); ++row) {
                bool entryMatch = (entryFilter == "All Dates") ||
                    (tbl->item(row, 3) && dateOf(tbl->item(row, 3)->text()) == entryFilter);
                bool exitMatch = (exitFilter == "All Dates") ||
                    (tbl->item(row, 4) && dateOf(tbl->item(row, 4)->text()) == exitFilter);
                tbl->setRowHidden(row, !(entryMatch && exitMatch));
            }
            };

        QPushButton* btnRef = makeBtn("Load My History", "#1e3d4d");
        connect(btnRef, &QPushButton::clicked, [this, tbl, cmbEntry, cmbExit, dateOf]() {
            if (!currentUser) return;

            const int MAX_R = 1000;
            ParkingRecord recs[MAX_R];
            int count = lot.getUserHistory(currentUser->getUsername(), recs, MAX_R);

            tbl->setRowCount(0);
            QSet<QString> entryDates, exitDates;
            for (int i = 0; i < count; i++) {
                const ParkingRecord& r = recs[i];
                int row = tbl->rowCount(); tbl->insertRow(row);
                ostringstream fee; fee << fixed << setprecision(2) << r.fee;
                ostringstream dur; dur << fixed << setprecision(2) << r.duration;
                tbl->setItem(row, 0, new QTableWidgetItem(toQ(r.vehicleNumber)));
                tbl->setItem(row, 1, new QTableWidgetItem(toQ(r.vehicleType)));
                tbl->setItem(row, 2, new QTableWidgetItem(QString::number(r.slotNumber)));
                tbl->setItem(row, 3, new QTableWidgetItem(toQ(r.entryTime)));
                tbl->setItem(row, 4, new QTableWidgetItem(toQ(r.exitTime)));
                tbl->setItem(row, 5, new QTableWidgetItem(QString::fromStdString(dur.str())));
                tbl->setItem(row, 6, new QTableWidgetItem(QString::fromStdString(fee.str())));
                entryDates.insert(dateOf(toQ(r.entryTime)));
                exitDates.insert(dateOf(toQ(r.exitTime)));
            }
            // Rebuild dropdowns
            cmbEntry->blockSignals(true); cmbExit->blockSignals(true);
            cmbEntry->clear(); cmbExit->clear();
            cmbEntry->addItem("All Dates"); cmbExit->addItem("All Dates");
            QList<QString> eList = entryDates.values(); std::sort(eList.begin(), eList.end());
            QList<QString> xList = exitDates.values();  std::sort(xList.begin(), xList.end());
            for (const auto& d : eList) cmbEntry->addItem(d);
            for (const auto& d : xList) cmbExit->addItem(d);
            cmbEntry->blockSignals(false); cmbExit->blockSignals(false);
            });

        connect(btnApply, &QPushButton::clicked, applyFilter);
        connect(btnClear2, &QPushButton::clicked, [cmbEntry, cmbExit, applyFilter]() {
            cmbEntry->setCurrentIndex(0);
            cmbExit->setCurrentIndex(0);
            applyFilter();
            });

        lay->addWidget(btnRef, 0, Qt::AlignLeft);
    }

    content->addWidget(panSlots);
    content->addWidget(panPark);
    content->addWidget(panCheckout);
    content->addWidget(panHistory);

    connect(btnSlots, &QPushButton::clicked, [content]() { content->setCurrentIndex(0); });
    connect(btnPark, &QPushButton::clicked, [content]() { content->setCurrentIndex(1); });
    connect(btnCheckout, &QPushButton::clicked, [content]() { content->setCurrentIndex(2); });
    connect(btnHistory, &QPushButton::clicked, [content]() { content->setCurrentIndex(3); });

    mainLay->addWidget(sidebar);
    mainLay->addWidget(content, 1);
    return page;
}

// ─── Logout ──────────────────────────────────────────────────
void MainWindow::onLogout() {
    delete currentUser;
    currentUser = nullptr;
    stack->setCurrentIndex(PAGE_LOGIN);
}

// ─── Stubs for unused slots ───────────────────────────────────
void MainWindow::onSetupConfirm() {}
void MainWindow::onLogin() {}
void MainWindow::onSignup() {}
void MainWindow::onAdminViewSlots() { if (slotsGridWidget) refreshSlotsGrid(slotsGridWidget); refreshAdminStats(); }
void MainWindow::onAdminViewHistory() {}
void MainWindow::onAdminViewRevenue() {}
void MainWindow::onAdminUpdateRate() {}
void MainWindow::onAdminSearch() {}
void MainWindow::onAdminDeleteUser() {}
void MainWindow::onCustomerViewSlots() {}
void MainWindow::onCustomerPark() {}
void MainWindow::onCustomerCheckout() {}
void MainWindow::onCustomerHistory() {}