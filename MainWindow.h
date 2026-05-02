#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>
#include <QComboBox>
#include <QSpinBox>
#include <QFrame>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QScrollArea>
#include "ParkingLot.h"
#include "User.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    ParkingLot  lot;
    User* currentUser = nullptr;

    QStackedWidget* stack;

    // Pages
    QWidget* makeSetupPage();
    QWidget* makeLoginPage();
    QWidget* makeSignupPage();
    QWidget* makeAdminPage();
    QWidget* makeCustomerPage();

    // Admin sub-widgets
    QWidget* makeAdminSlotsPanel();
    QWidget* makeAdminHistoryPanel();
    QWidget* makeAdminRevenuePanel();
    QWidget* makeAdminRatePanel();
    QWidget* makeAdminSearchPanel();
    QWidget* makeAdminUsersPanel();

    // Common helpers
    QLabel* makeTitle(const QString& text);
    QPushButton* makeBtn(const QString& label, const QString& color = "#2d6cdf");
    QLineEdit* makeInput(const QString& placeholder, bool password = false);
    QFrame* makeSeparator();
    void         applyGlobalStyle();
    void         showMessage(const QString& msg, bool success = true);

    // Stacked page indices
    enum Pages { PAGE_SETUP = 0, PAGE_LOGIN, PAGE_SIGNUP, PAGE_ADMIN, PAGE_CUSTOMER };

    // Slot display grid
    QWidget* slotsGridWidget = nullptr;
    void refreshSlotsGrid(QWidget* container);

    // Live stat labels (admin)
    QLabel* lblTotal = nullptr;
    QLabel* lblOccupied = nullptr;
    QLabel* lblFree = nullptr;
    QLabel* lblRevenue = nullptr;
    void    refreshAdminStats();

private slots:
    void onSetupConfirm();
    void onLogin();
    void onSignup();
    void onLogout();

    void onAdminViewSlots();
    void onAdminViewHistory();
    void onAdminViewRevenue();
    void onAdminUpdateRate();
    void onAdminSearch();
    void onAdminDeleteUser();

    void onCustomerViewSlots();
    void onCustomerPark();
    void onCustomerCheckout();
    void onCustomerHistory();
};

#endif // MAINWINDOW_H