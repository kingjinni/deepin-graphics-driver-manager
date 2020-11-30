
#include "mainwindow.h"
#include "resolutionwidget.h"
#include "utils.h"

#include <QApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QDebug>
#include <QTimer>
#include <QLabel>
#include <QAction>
#include <DTitlebar>
#include <DSvgRenderer>
#include <DThemeManager>
#include <QJsonObject>
#include <QJsonDocument>

const QString GraphicMangerServiceName = "com.deepin.graphicmanger";
const QString GraphicMangerPath = "/com/deepin/graphicmanger";


MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{

    m_graphicsDriver = new ComDeepinDaemonGraphicsDriverInterface(
                   GraphicMangerServiceName,
                   GraphicMangerPath,
                   QDBusConnection::systemBus());

    m_toggleButton = new DSuggestButton;
    m_toggleButton->setText(tr("Switch"));
    m_toggleButton->setFixedHeight(38);

    m_topTips = new QLabel;
    m_topTips->setAlignment(Qt::AlignHCenter);
    m_topTips->setVisible(false);
    m_topTips->setObjectName("TopTips");

    m_botTips = new QLabel;
    m_botTips->setAlignment(Qt::AlignHCenter);
    m_botTips->setVisible(false);
    m_botTips->setWordWrap(true);
    m_botTips->setObjectName("BottomTips");

    m_tipsIcon = new QLabel;

    m_okButton = new DSuggestButton;
    m_okButton->setText(tr("OK"));
    m_okButton->setFixedHeight(38);

    m_rebootButton = new DSuggestButton;
    m_rebootButton->setText(tr("Reboot"));
    m_rebootButton->setFixedHeight(38);
    m_rebootButton->setVisible(false);

    m_progress = new DWaterProgress;
    m_progress->setTextVisible(true);
    m_progress->setFixedSize(100, 100);
    m_progress->setValue(0);
    m_progress->setVisible(false);

    m_vendorIcon = new QLabel;
    m_vendorIcon->setAlignment(Qt::AlignCenter);
    m_vendorName = new QLabel;
    m_vendorName->setWordWrap(true);
    m_vendorName->setAlignment(Qt::AlignCenter);
    //m_vendorName->setText(m_devInfo.devices().toList().join('\n'));

    m_resolutionsLayout = new QVBoxLayout;
    m_resolutionsLayout->setContentsMargins(8, 8, 8, 8);
    m_resolutionsLayout->setSpacing(1);
    m_resolutionsWidget = new QWidget;
    m_resolutionsWidget->setLayout(m_resolutionsLayout);
    m_resolutionsWidget->setObjectName("ResolutionsWidget");

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_topTips);
    centralLayout->addWidget(m_vendorIcon);
    centralLayout->addWidget(m_vendorName);
    centralLayout->addStretch();
    centralLayout->addWidget(m_tipsIcon);
    centralLayout->setAlignment(m_tipsIcon, Qt::AlignHCenter);
    centralLayout->addWidget(m_resolutionsWidget);
    centralLayout->addWidget(m_progress);
    centralLayout->setAlignment(m_progress, Qt::AlignHCenter);
    centralLayout->addWidget(m_botTips);
    centralLayout->addStretch();
    centralLayout->addWidget(m_toggleButton);
    centralLayout->addWidget(m_okButton);
    centralLayout->addWidget(m_rebootButton);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(40, 0, 40, 30);

    titlebar()->setTitle(" ");
    setCentralWidget(new QWidget);
    centralWidget()->setLayout(centralLayout);

    setFixedSize(440, 600);
    move(qApp->primaryScreen()->geometry().center() - rect().center());

    //connect(m_toggleButton, &DSuggestButton::clicked, this, &MainWindow::onToggleBtnClicked);
    //connect(m_rebootButton, &DSuggestButton::clicked, this, &MainWindow::onRebootBtnClicked);
    connect(m_okButton, &DSuggestButton::clicked, qApp, &QApplication::quit);

    QTimer::singleShot(0, this, &MainWindow::loadResolutions);
}

MainWindow::~MainWindow()
{

}

void MainWindow::keyPressEvent(QKeyEvent *e)
{

}

void MainWindow::noResolutions()
{

}

void MainWindow::loadDevice()
{
    QString devices;
#ifdef TEST_UI
    QString path = RESOURCES_DIR"/test/device.json";
    QFile file(path);
    if (file.exists()) {
       file.open(QIODevice::ReadOnly);
       devices = QString(file.readAll());
    }
#else
    QDBusPendingReply<QString> getDeviceReply = m_graphicsDriver->GetDevice();
    getDeviceReply.waitForFinished();
    if (!getDeviceReply.isValid()) {
        qDebug() << getDeviceReply.error();
        return;
    }
    devices = getDeviceReply.value();
#endif
    const QJsonObject devObj = Utils::QStringToJson(devices);
    for (const auto d : devObj["devices"].toArray()) {
        Device device(d.toObject());
        if (device.name().isEmpty() || device.info().isEmpty()) {
            return;
        }
        m_devices.push_back(device);
    }
    setVendorIcon();
}

void MainWindow::setVendorIcon()
{
    bool bIntel = false;
    bool bAmd = false;
    bool bNvidia = false;

    QStringList devInfo;
    foreach(const auto dev, m_devices) {
        if (dev.name().contains("intel", Qt::CaseInsensitive)) {
            bIntel = true;
        }

        if (dev.name().contains("amd", Qt::CaseInsensitive)) {
            bAmd = true;
        }

        if (dev.name().contains("nvidia", Qt::CaseInsensitive)) {
            bNvidia = true;
        }
        devInfo << dev.info();
    }

    QString iconPath;
    if (bIntel && !bAmd && !bNvidia) {
       iconPath = RESOURCES_DIR"/icons/Intel.svg";
    }

    if (bAmd && !bIntel && !bNvidia) {
        iconPath = RESOURCES_DIR"/icons/AMD.svg";
    }

    if (bNvidia && !bIntel && !bAmd) {
        iconPath = RESOURCES_DIR"/icons/NVIDIA.svg";
    }

    if (bIntel && bNvidia && !bAmd) {
        iconPath = RESOURCES_DIR"/icons/Intel-NVIDIA.svg";
    }

    m_vendorIcon->setPixmap(Utils::hidpiPixmap(iconPath, QSize(128, 128)));
    m_vendorName->setText(devInfo.join('\n'));

}

void MainWindow::loadResolutions()
{
    loadDevice();
    QString strResolution;
    QDBusPendingReply<QString> resolutionReply = m_graphicsDriver->GetResolutionTitle();
    resolutionReply.waitForFinished();
    if (!resolutionReply.isValid()) {
        qDebug() << resolutionReply.error();
    }

    strResolution = resolutionReply.value();
    qDebug() << strResolution;
}

void MainWindow::onResolutionSelected()
{

}

void MainWindow::onToggleBtnClicked()
{

}

void MainWindow::onRebootBtnClicked()
{

}

void MainWindow::onPolicyKitPassed()
{

}

void MainWindow::onPrepareFinished(bool success)
{

}
