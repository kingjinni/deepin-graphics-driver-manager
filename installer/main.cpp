
#include <DApplication>
#include <DLog>
#include <DDialog>
#include <QDebug>
#include "graphicsdriverproxy.h"
#include <QObject>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

#define CONFIG "/usr/lib/deepin-graphics-driver-manager/working-dir/config.conf"
#define ROOT_RESKTOP_FILE "etc/xdg/autostart/deepin-gradvrmgr-installer.desktop"


const QString GraphicsDriverDbusService = "com.deepin.daemon.GraphicsDriver";

ComDeepinDaemonGraphicsDriverInterface *g_graphicsDriver = nullptr;

DDialog *dialog(const QString &message, const QString &iconName)
{
    const auto ratio = qApp->devicePixelRatio();
    QPixmap iconPix = QIcon::fromTheme(iconName).pixmap(QSize(64, 64) * ratio);
    iconPix.setDevicePixelRatio(ratio);

    DDialog *d = new DDialog;
    d->setMessage(message);
    d->setIcon(iconPix, QSize(64, 64));
    return d;
}

void show_success_dialog()
{
    QString new_driver = "new_driver";
    QDBusPendingReply<bool> reply = g_graphicsDriver->newDriver();
    reply.waitForFinished();
    if (reply.isValid())
    {
        new_driver = reply.value();
    }

    const QString &message = qApp->translate("main", "Congratulations, you have switched to %1, please reboot to take effect.");

    DDialog *d = dialog(message.arg(new_driver), "://resources/icons/deepin-graphics-driver-manager-success.svg");
    d->addButton(qApp->translate("main", "Cancel"));
    d->addButton(qApp->translate("main", "Reboot"));
    d->setDefaultButton(1);

    QObject::connect(d, &DDialog::buttonClicked, [=](int index, const QString &text) {
        Q_UNUSED(text);
        if (index == 1)
        {
            g_graphicsDriver->reboot();
        }
    });

    d->exec();
}

void show_fail_dialog()
{
    QString old_driver = "old_driver";
    QString new_driver = "new_driver";
    QDBusPendingReply<QString> oldDriverReply = g_graphicsDriver->OldDriver();
    oldDriverReply.waitForFinished();
    if (oldDriverReply.isValid())
    {
        old_driver = oldDriverReply.value();
    }

    QDBusPendingReply<QString> newDriverReply = g_graphicsDriver->newDriver();
    if (newDriverReply.isValid())
    {
        new_driver = newDriverReply.value();
    }

    const QString &message = qApp->translate("main", "Auto restore to %2 after failed to switch to %1");

    DDialog *d = dialog(message.arg(new_driver).arg(old_driver), "dialog-warning");
    d->addButton(qApp->translate("main", "Submit Feedback"));

    QObject::connect(d, &DDialog::buttonClicked, [=] { QProcess::startDetached("deepin-feedback"); });

    d->exec();
}

int show_install_dialog() {
    DDialog *installDialog = dialog(qApp->translate("main", "Updating the driver, please wait..."), "://resources/icons/deepin-graphics-driver-manager-installing.svg");

    g_graphicsDriver->removeDriver();
    QObject::connect(g_graphicsDriver, &ComDeepinDaemonGraphicsDriverInterface::removeDriverResult, [=](int result) {
        qDebug() << "remove driver result " << result;
        if (result != 0)
        {
            installDialog->done(result);
            return;
        }

        QObject::connect(g_graphicsDriver, &ComDeepinDaemonGraphicsDriverInterface::installDriverResult, [=](int result) {
            installDialog->done(result);
        });
        g_graphicsDriver->installDriver();

    });
    return installDialog->exec();
}

void removeDesktopFile()
{
    QFile desktopFile(QDir::rootPath() + ROOT_RESKTOP_FILE);
    if (desktopFile.exists())
        desktopFile.remove();
    else
        qDebug() << desktopFile.fileName() << "do not exists!";
}

void init()
{
    bool testSuccess = false;
    QDBusPendingReply<bool> reply = g_graphicsDriver->isTestSuccess();
    reply.waitForFinished();
    if (reply.isValid())
    {
        testSuccess = reply.value();
    }

    qDebug() << "testSuccess is:" << testSuccess;
    if (testSuccess)
    {
        const int exitCode = show_install_dialog();
        qDebug() << "show_install_dialog exitCode" << exitCode;
        if (exitCode == 0)
        {
            qDebug() << "show_success_dialog";
            show_success_dialog();
        }
        else
        {
            qDebug() << "show_fail_dialog1";
            show_fail_dialog();
        }
    }
    else
    {
        qDebug() << "show_fail_dialog2";
        show_fail_dialog();
    }
}

int main(int argc, char *args[])
{
    DApplication dapp(argc, args);
    dapp.setQuitOnLastWindowClosed(true);
    dapp.setOrganizationName("deepin");
    dapp.setApplicationName("deepin-graphics-driver-manager-installer");

    QTranslator translator;
    translator.load(QString("/usr/share/deepin-graphics-driver-manager/translations/deepin-graphics-driver-manager_%1.qm").arg(QLocale::system().name()));
    dapp.installTranslator(&translator);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    g_graphicsDriver = new ComDeepinDaemonGraphicsDriverInterface(
                "com.deepin.daemon.GraphicsDriver",
                "/GraphicsDriver",
                QDBusConnection::sessionBus());

    QTimer::singleShot(1, nullptr, init);

    return dapp.exec();
}

