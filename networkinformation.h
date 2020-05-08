#ifndef NETWORKINFORMATION_H
#define NETWORKINFORMATION_H

#include <QDialog>
#include <QWidget>
#include <QJsonObject>

namespace Ui {
class NetworkInformation;
}

class NetworkInformation : public QDialog
{
    Q_OBJECT

public:
    explicit NetworkInformation(QJsonObject networdData, QWidget *parent = nullptr);
    ~NetworkInformation();

private:
    Ui::NetworkInformation *ui;
};

#endif // NETWORKINFORMATION_H
