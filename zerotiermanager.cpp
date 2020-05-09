#include "zerotiermanager.h"
#include "ui_zerotiermanager.h"
#include "networkinformation.h"

#include <QMessageBox>
#include <QCloseEvent>
#include <QAction>
#include <QMenu>
#include <QList>
#include <QProcess>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QHostAddress>
#include <QTableWidget>
#include <QDebug>
#include <QSettings>


ZeroTierManager::ZeroTierManager(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ZeroTierManager)
{
    ui->setupUi(this);
    createActions();
    createTrayIcon();

    QIcon icon = QIcon(":/zerotier-one");
    trayIcon->setIcon(icon);


    QStringList networks;
    QSettings config("jimmy.stelzer","zerotier-one-gui");
    networks = config.value("networks").toStringList();

    foreach(auto network, networks){
        ztonStatus net;
        net.id = network;
        configuratedNetwork.push_back(net);
        addActionMenu(network, false);
    }
    updateTrayMenu();
    updateStatusTimer = new QTimer(this);
    connect(updateStatusTimer, &QTimer::timeout, this, QOverload<>::of(&ZeroTierManager::updateStatus));
    updateStatusTimer->start(500);

    ui->lstCurrentNetwork->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->lstCurrentNetwork->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    ui->lstCurrentNetwork->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    ui->lstCurrentNetwork->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->lstCurrentNetwork->setSelectionMode(QAbstractItemView::SingleSelection);


    ui->lstCurrentNetwork->verticalHeader()->hide();
    trayIcon->show();
}

ZeroTierManager::~ZeroTierManager()
{
    delete ui;
}

void ZeroTierManager::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
        hide();
        event->ignore();
    }
}

void ZeroTierManager::manageNetwork(bool checked)
{
    Q_UNUSED(checked);
    this->raise();
    this->show();
}

void ZeroTierManager::createActions()
{
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);

    addAction = new QAction(tr("&Manage network"), this);
    connect(addAction, &QAction::triggered, this, &ZeroTierManager::manageNetwork);
}

void ZeroTierManager::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(addAction);
    separatorAction = trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    connect(trayIcon, &QSystemTrayIcon::activated, this, &ZeroTierManager::trayActivated);
}

bool ZeroTierManager::checkServiceIsRunning()
{
    QProcess process;
    process.start("systemctl", QStringList() << "show" << "zerotier-one.service" << "--no-pager" << "-p" << "ActiveState" << "--value");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    if(output.startsWith("active")){
        return true;
    }
    return false;
}

bool ZeroTierManager::checkHavePkexec()
{
    QProcess process;
    process.start("whereis", QStringList() << "-b" << "pkexec");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    if(output.length() > 8){
        return true;
    }
    return false;
}

bool ZeroTierManager::checkUserToken()
{
    QStringList dir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if(dir.length() > 0){
        QString tokenFile = dir.first() + QDir::separator() + USER_TOKEN_FILE;
        if(QFileInfo::exists(tokenFile)){
            return true;
        }
    }
    return false;
}

void ZeroTierManager::copyAuthToken()
{
    QStringList dir = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    if(dir.length() > 0){
        QString tokenFilename = dir.first() + QDir::separator() + USER_TOKEN_FILE;
        QProcess process;
        process.start("sh", QStringList() << "-c" << "pkexec cat /var/lib/zerotier-one/authtoken.secret >> " + tokenFilename);
        process.waitForFinished();
        if(process.exitCode() == QProcess::NormalExit){
            QFile::setPermissions(tokenFilename, QFileDevice::ReadUser|QFileDevice::WriteUser);
            haveUserToken = checkUserToken();
        }
    }
}

void ZeroTierManager::addActionMenu(QString name, bool check)
{
    QAction *act = new QAction(name, this);
    act->setChecked(check);
    act->setCheckable(true);
    networkMeuActions.push_back(act);
    connect(act, &QAction::triggered, this, &ZeroTierManager::trayMenuTriggered);
}

void ZeroTierManager::updateTrayMenu()
{
    foreach(auto netAct, networkMeuActions){
        bool hasToInsert = true;
        int dynamicActionsSize = trayIconMenu->actions().size() - 3 + 1;
        for(int idx=1; idx < dynamicActionsSize; idx++){
            if(trayIconMenu->actions().at(idx)->text().startsWith(netAct->text().mid(0,16))){
                hasToInsert = false;
                break;
            }
        }
        if(hasToInsert){
            trayIconMenu->insertAction(separatorAction, netAct);
        }
    }
}

void ZeroTierManager::updateStatus()
{
    if(updateStatusTimer->interval() != 10000){
        updateStatusTimer->setInterval(10000);
    }
    serviceIsRunning = checkServiceIsRunning();
    havePkexec = checkHavePkexec();
    if(!havePkexec){
        QMessageBox::critical(nullptr, QObject::tr("Check pkexec"),
                              QObject::tr("I couldn't find pkexec "
                                          "on this system. This tool is required."));
        qApp->quit();
        return;
    }
    if(!serviceIsRunning){
        QMessageBox::critical(nullptr, QObject::tr("ZeroTier One Service"),
                              QObject::tr("The ZeroTier One is not runnning "
                                          "on this system. We will try to start the service."));
        for(int i=0; i < 3; i++){
            if(startService() && (serviceIsRunning = checkServiceIsRunning()) == true){
                break;
            }
        }
        if(!serviceIsRunning){
            QMessageBox::critical(nullptr, QObject::tr("ZeroTier One Service"),
                                  QObject::tr("Fail to start the ZeroTier One service."));
            qApp->quit();
            return;
        }
    }
    haveUserToken = checkUserToken();
    if(!haveUserToken){
        copyAuthToken();
    }

    updateNetworkStatus();
}

bool ZeroTierManager::startService()
{
    QProcess process;
    process.start("pkexec", QStringList() << "systemctl" << "start" << "zerotier-one.service");
    process.waitForFinished();
    if(process.exitCode() == QProcess::NormalExit){
        return true;
    }
    return false;
}

void ZeroTierManager::updateNetworkStatus()
{
    QJsonDocument jsonDoc = getNetworkInformation();
    QStringList networks;
    for(int i=0; i < configuratedNetwork.length(); i++){
        networks << configuratedNetwork[i].id;
        configuratedNetwork[i].hasToUpdate = (configuratedNetwork[i].status == false);
        configuratedNetwork[i].status = false;
        configuratedNetwork[i].ipv4 = "";
        configuratedNetwork[i].ipv6 = "";
    }
    QSettings config("jimmy.stelzer","zerotier-one-gui");
    config.setValue("networks", networks);
    if(!jsonDoc.isNull()){
        if(jsonDoc.isArray()){
            foreach(auto obj, jsonDoc.array()){
                if(obj.isObject()){
                    QJsonObject jsonObj = obj.toObject();
                    ztonStatus nztonStatus;

                    nztonStatus.id = jsonObj.value("id").toString();
                    nztonStatus.macAddr = jsonObj.value("mac").toString();
                    nztonStatus.status = (jsonObj.value("status").toString() == "OK");
                    nztonStatus.name = jsonObj.value("name").toString();
                    foreach(auto objIp, jsonObj.value("assignedAddresses").toArray()){
                        QHostAddress checkIpVersion(objIp.toString());

                        if(checkIpVersion.protocol() == QAbstractSocket::NetworkLayerProtocol::IPv4Protocol){
                            nztonStatus.ipv4 = objIp.toString();
                        }else if(checkIpVersion.protocol() == QAbstractSocket::NetworkLayerProtocol::IPv6Protocol){
                            nztonStatus.ipv6 = objIp.toString();
                        }
                    }

                    int idx = configuratedNetwork.indexOf(nztonStatus);
                    if(idx == -1){
                        configuratedNetwork.push_back(nztonStatus);
                    }else{
                        nztonStatus.hasToUpdate = (configuratedNetwork[idx].hasToUpdate && !nztonStatus.status);
                        configuratedNetwork[idx] = nztonStatus;
                    }

                }
            }
        }
    }

    foreach(auto action, networkMeuActions){
        bool hasToRemove = true;
        for(int i=0; i < configuratedNetwork.length(); i++){
            if(action->text().startsWith(configuratedNetwork[i].id)){
                hasToRemove = false;
                if(configuratedNetwork[i].name.length() > 0){
                    action->setText(configuratedNetwork[i].id + " (" + configuratedNetwork[i].name + ")");
                }
                action->setChecked(configuratedNetwork[i].status);
            }
        }
        if(hasToRemove){
            trayIconMenu->removeAction(action);
            networkMeuActions.removeAll(action);
            action->deleteLater();
        }
    }

    bool restoreSelection = false;
    QItemSelectionModel *select = ui->lstCurrentNetwork->selectionModel();
    ui->btnToogleConnection->setEnabled(false);
    ui->btnInformation->setEnabled(false);
    ui->btnRemove->setEnabled(false);
    int currentSelectedRow = -1;
    if((restoreSelection = select->hasSelection()) == true){
        QModelIndexList indexList = select->selectedIndexes();
        currentSelectedRow = indexList.at(0).row();
    }
    ui->lstCurrentNetwork->setRowCount(0);
    for(int i=0; i < configuratedNetwork.length(); i++){
        ui->lstCurrentNetwork->insertRow(ui->lstCurrentNetwork->rowCount());
        ui->lstCurrentNetwork->setItem(i,0, new QTableWidgetItem(configuratedNetwork[i].id));
        ui->lstCurrentNetwork->setItem(i,1, new QTableWidgetItem(configuratedNetwork[i].name));
        ui->lstCurrentNetwork->setItem(i,2, new QTableWidgetItem((configuratedNetwork[i].status) ? QString(tr("OK")) : QString("-")));
    }

    if(restoreSelection && currentSelectedRow > -1){
        ui->lstCurrentNetwork->selectRow(currentSelectedRow);
        ui->btnToogleConnection->setEnabled(true);
        ui->btnInformation->setEnabled(true);
        ui->btnRemove->setEnabled(true);
    }

}

int ZeroTierManager::selectedNetwork()
{
    QItemSelectionModel *select = ui->lstCurrentNetwork->selectionModel();
    int currentSelectedRow = -1;
    if(select->hasSelection() == true){
        QModelIndexList indexList = select->selectedIndexes();
        currentSelectedRow = indexList.at(0).row();
    }
    return currentSelectedRow;
}

QJsonDocument ZeroTierManager::getNetworkInformation()
{
    QProcess process;
    process.start("zerotier-cli", QStringList() << "-j" << "listnetworks");
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(output.toUtf8());
    return jsonDoc;
}

bool ZeroTierManager::isNetworkJoinedInZerotier(QString networkId)
{
    QJsonDocument currentNetworks = getNetworkInformation();
    if(!currentNetworks.isNull()){
        if(currentNetworks.isArray()){
            foreach(auto obj, currentNetworks.array()){
                if(obj.isObject()){
                    QJsonObject jsonObj = obj.toObject();
                    ztonStatus nztonStatus;

                    if(jsonObj.value("id").toString() == networkId){
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void ZeroTierManager::zerotierJoinNetwork(QString networkId)
{
    QProcess process;
    process.start("zerotier-cli", QStringList() << "join" << networkId);
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
}

void ZeroTierManager::zerotierLeaveNetwork(QString networkId)
{
    QProcess process;
    process.start("zerotier-cli", QStringList() << "leave" << networkId);
    process.waitForFinished();
    QString output = process.readAllStandardOutput();
}


void ZeroTierManager::on_btnOk_clicked()
{
    hide();
}

void ZeroTierManager::on_lstCurrentNetwork_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    Q_UNUSED(currentRow) Q_UNUSED(currentColumn) Q_UNUSED(previousRow) Q_UNUSED(previousColumn)
    ui->btnToogleConnection->setEnabled(true);
    ui->btnInformation->setEnabled(true);
    ui->btnRemove->setEnabled(true);
}

void ZeroTierManager::on_btnToogleConnection_clicked()
{
    int currentSelectedRow = selectedNetwork();

    if(currentSelectedRow >= 0){

        QTableWidgetItem *item = ui->lstCurrentNetwork->item(currentSelectedRow, 0);
        if(isNetworkJoinedInZerotier(item->text())){
            zerotierLeaveNetwork(item->text());
        }else{
            zerotierJoinNetwork(item->text());
        }
    }
    updateNetworkStatus();
}

void ZeroTierManager::on_btnRemove_clicked()
{
    int currentSelectedRow = selectedNetwork();

    if(currentSelectedRow >= 0){
        QTableWidgetItem *item = ui->lstCurrentNetwork->item(currentSelectedRow, 0);
        if(isNetworkJoinedInZerotier(item->text())){
            zerotierLeaveNetwork(item->text());

            ztonStatus nztonStatus;
            nztonStatus.id = item->text();
            int idx = configuratedNetwork.indexOf(nztonStatus);
            if(idx > -1){
                configuratedNetwork.removeAt(idx);
            }
        }
        ui->lstCurrentNetwork->clearSelection();
    }
    updateNetworkStatus();
}

void ZeroTierManager::on_btnInformation_clicked()
{
    int currentSelectedRow = selectedNetwork();
    bool hasInfo = false;
    if(currentSelectedRow >= 0){
        QTableWidgetItem *item = ui->lstCurrentNetwork->item(currentSelectedRow, 0);
        QJsonDocument jsonDoc = getNetworkInformation();
        if(!jsonDoc.isNull()){
            if(jsonDoc.isArray()){
                foreach(auto obj, jsonDoc.array()){
                    if(obj.isObject()){
                        QJsonObject jsonObj = obj.toObject();
                        if(jsonObj.value("id").toString() == item->text()){
                            hasInfo = true;
                            NetworkInformation *info = new NetworkInformation(jsonObj, nullptr);
                            info->exec();

                            info->deleteLater();
                            break;
                        }
                    }
                }
            }
        }
    }
    if(!hasInfo){
        QMessageBox::information(this,tr("Network Information"), tr("There is no information about this network at this moment. You must connect to this network to see this information."));
    }
}

void ZeroTierManager::on_btnJoin_clicked()
{
    QString networkId = ui->edtNetworkId->text();
    if(!isNetworkJoinedInZerotier(networkId)){
        ztonStatus nztonStatus;

        nztonStatus.id = networkId;

        int idx = configuratedNetwork.indexOf(nztonStatus);
        if(idx == -1){
            configuratedNetwork.push_back(nztonStatus);
        }
        addActionMenu(nztonStatus.id, false);
        updateTrayMenu();
        zerotierJoinNetwork(networkId);
    }
    updateNetworkStatus();
}

void ZeroTierManager::trayActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::DoubleClick){
        manageNetwork(false);
    }
}

void ZeroTierManager::trayMenuTriggered(bool status)
{
    QAction* caller = qobject_cast<QAction*>(sender());
    Q_ASSERT(caller != nullptr);
    QString id = caller->text().mid(0,16);

    if(isNetworkJoinedInZerotier(id) && status == false){
        zerotierLeaveNetwork(id);
    }else if (!isNetworkJoinedInZerotier(id) && status == true){
        zerotierJoinNetwork(id);
    }
}
