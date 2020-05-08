#include "zerotiermanager.h"

#include <QApplication>
#include <QSystemTrayIcon>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(zerotieronegui);

    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, QObject::tr("Systray"),
                              QObject::tr("I couldn't detect any system tray "
                                          "on this system. This feature is required."));
        return 1;
    }
    QApplication::setQuitOnLastWindowClosed(false);

    ZeroTierManager w;

    return a.exec();
}
