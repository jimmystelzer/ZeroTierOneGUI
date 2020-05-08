#include "ztonstatus.h"
#include <QDebug>


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

