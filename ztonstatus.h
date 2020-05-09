#ifndef ZTONSTATUS_H
#define ZTONSTATUS_H
#include <QString>

class ztonStatus
{
public:
    QString id;
    QString macAddr;
    QString name;
    QString ipv4;
    QString ipv6;
    bool status;
    bool operator==(const ztonStatus &ztons) const;
    void operator=(const ztonStatus &ztons);
    bool hasToUpdate;

};

#endif // ZTONSTATUS_H
