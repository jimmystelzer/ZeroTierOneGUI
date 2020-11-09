#include "ztonstatus.h"
#include <QDebug>

ztonStatus::ztonStatus(){

}

ztonStatus::ztonStatus(const ztonStatus &ztons)
{
    id = ztons.id;
    macAddr = ztons.macAddr;
    name = ztons.name;
    ipv4 = ztons.ipv4;
    ipv6 = ztons.ipv6;
    status = ztons.status;
    hasToUpdate = ztons.hasToUpdate;
}

bool ztonStatus::operator==(const ztonStatus &ztons) const
{
    return (id.compare(ztons.id) == 0);
}

void ztonStatus::operator=(const ztonStatus &ztons)
{
    name = ztons.name;
    macAddr = ztons.macAddr;
    ipv4 = ztons.ipv4;
    ipv6 = ztons.ipv6;
    status = ztons.status;
}

