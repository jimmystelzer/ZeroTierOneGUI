#ifndef ZEROTIERMANAGER_H
#define ZEROTIERMANAGER_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QList>
#include <QTimer>
#include "ztonstatus.h"

QT_BEGIN_NAMESPACE
namespace Ui { class ZeroTierManager; }
QT_END_NAMESPACE

class ZeroTierManager : public QMainWindow
{
    Q_OBJECT

public:
    ZeroTierManager(QWidget *parent = nullptr);
    ~ZeroTierManager();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void manageNetwork(bool checked);
    void on_btnOk_clicked();
    void on_lstCurrentNetwork_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_btnToogleConnection_clicked();
    void on_btnRemove_clicked();
    void on_btnInformation_clicked();
    void on_btnJoin_clicked();
    void trayActivated(QSystemTrayIcon::ActivationReason reason);
    void trayMenuTriggered(bool status);
    void openCentral(bool checked);

private:
    void createActions();
    void createTrayIcon();
    bool checkServiceIsRunning();
    bool checkHavePkexec();
    bool checkUserToken();
    void copyAuthToken();
    void addActionMenu(QString name, bool check);
    void updateTrayMenu();
    void updateStatus();
    bool startService();
    void updateNetworkStatus();
    int selectedNetwork();
    QJsonDocument getNetworkInformation();
    bool isNetworkJoinedInZerotier(QString networkId);
    void zerotierJoinNetwork(QString networkId);
    void zerotierLeaveNetwork(QString networkId);

    Ui::ZeroTierManager *ui;
    QAction *quitAction;
    QAction *addAction;
    QAction *separatorAction;
    QAction *quitSeparatorAction;
    QAction *openCentralAction;
    QList<QAction *> networkMeuActions;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    bool serviceIsRunning;
    bool havePkexec;
    bool haveUserToken;
    const QString USER_TOKEN_FILE = ".zeroTierOneAuthToken";
    QTimer *updateStatusTimer;
    QList<ztonStatus> configuratedNetwork;
};
#endif // ZEROTIERMANAGER_H
