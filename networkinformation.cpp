#include "networkinformation.h"
#include "ui_networkinformation.h"
#include <QJsonArray>

NetworkInformation::NetworkInformation(QJsonObject networdData, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetworkInformation)
{
    ui->setupUi(this);

    QStringList assignedAddressesStr;
    foreach(auto objIp, networdData.value("assignedAddresses").toArray()){
        assignedAddressesStr.push_back(objIp.toString());
    }


    ui->ID->setText(networdData.value("id").toString());
    ui->MAC->setText(networdData.value("mac").toString());

    ui->name->setText(networdData.value("name").toString());
    ui->assignedAddresses->setText(assignedAddressesStr.join(", "));
    ui->type->setText(networdData.value("type").toString());

    ui->allowGlobal->setChecked(networdData.value("allowGlobal").toBool(false));
    ui->allowDefault->setChecked(networdData.value("allowDefault").toBool(false));
    ui->allowManaged->setChecked(networdData.value("allowManaged").toBool(false));

}

NetworkInformation::~NetworkInformation()
{
    delete ui;
}
