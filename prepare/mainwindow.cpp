
#include "mainwindow.h"
#include "resolutionwidget.h"
#include "singleresolutionwidget.h"
#include "utils.h"
#include "resolution.h"
#include "../test_installer/mainwindow.h"


#include <QApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QTimer>
#include <QLabel>
#include <QAction>
#include <DTitlebar>
#include <DSvgRenderer>
#include <QJsonObject>
#include <QJsonDocument>
#include <DApplicationHelper>
#include <iostream>


const QString GraphicMangerServiceName = "com.deepin.graphicmanger";
const QString GraphicMangerPath = "/com/deepin/graphicmanger";


MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{

    m_graphicsDriver = new ComDeepinDaemonGraphicsDriverInterface(
                   GraphicMangerServiceName,
                   GraphicMangerPath,
                   QDBusConnection::systemBus());

    m_toggleButton = new QPushButton;
    m_toggleButton->setText(tr("Switch"));
    m_toggleButton->setFixedHeight(38);
    m_toggleButton->setVisible(false);


    m_warningTips = new QLabel;
    m_warningTips->setAlignment(Qt::AlignHCenter);
    m_warningTips->setVisible(false);
    m_warningTips->setWordWrap(true);
    m_warningTips->setObjectName("WarningTips");


    m_installTips = new QLabel;
    m_installTips->setVisible(false);
    m_installTips->setWordWrap(true);
    m_installTips->setAlignment(Qt::AlignHCenter);
    m_installTips->setContentsMargins(116, 0, 116, 0);
    m_installTips->setObjectName("installTips");


    m_tipsIcon = new QLabel;

    m_updateButton = new QPushButton;
    m_updateButton->setText(tr("Update"));
    m_updateButton->setFixedHeight(38);
    m_updateButton->setVisible(false);

    m_rebootButton = new DSuggestButton;
    m_rebootButton->setText(tr("Reboot Now"));
    m_rebootButton->setFixedHeight(38);
    m_rebootButton->setVisible(false);

    m_cancelButton = new QPushButton;
    m_cancelButton->setText(tr("Cancel"));
    m_cancelButton->setFixedHeight(38);
    m_cancelButton->setVisible(false);

    m_okButton = new QPushButton;
    m_okButton->setText(tr("OK"));
    m_okButton->setFixedHeight(38);
    m_okButton->setVisible(false);

    m_vendorIcon = new QLabel;
    m_vendorIcon->setAlignment(Qt::AlignCenter);
    m_vendorName = new QLabel;
    m_vendorName->setWordWrap(true);
    m_vendorName->setAlignment(Qt::AlignCenter);

    m_resolutionsLayout = new QVBoxLayout;
    m_resolutionsLayout->setContentsMargins(0, 8, 0, 8);
    m_resolutionsLayout->setSpacing(10);
    m_resolutionsWidget = new QWidget;
    m_resolutionsWidget->setLayout(m_resolutionsLayout);

    auto *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_vendorIcon);
    centralLayout->addWidget(m_vendorName);
    centralLayout->addStretch();
    centralLayout->addWidget(m_tipsIcon);
    centralLayout->setAlignment(m_tipsIcon, Qt::AlignHCenter);
    centralLayout->addWidget(m_resolutionsWidget);
    centralLayout->addWidget(m_installTips);
    centralLayout->setAlignment(m_installTips, Qt::AlignHCenter);
    centralLayout->addStretch();
    centralLayout->addWidget(m_warningTips);
    centralLayout->addSpacing(10);
    centralLayout->addWidget(m_toggleButton);
    centralLayout->addWidget(m_updateButton);
    centralLayout->addWidget(m_okButton);

    auto *hBoxLayout = new QHBoxLayout;
    hBoxLayout->addWidget(m_cancelButton);
    hBoxLayout->addSpacing(10);
    hBoxLayout->addWidget(m_rebootButton);


    centralLayout->addLayout(hBoxLayout);
    centralLayout->setSpacing(0);
    centralLayout->setContentsMargins(10, 79, 10, 10);
    titlebar()->setTitle(" ");
    titlebar()->setIcon(QIcon(":/resources/icons/deepin-graphics-driver-manager-64px.svg"));
    m_centerWidget = new QWidget;
    m_centerWidget->setObjectName("centerWidget");

    m_centerWidget->setLayout(centralLayout);

    auto *mainLayout = new QHBoxLayout;
    auto *mainWidget = new QWidget;
    mainWidget->setObjectName("mainWidget");
    mainLayout->addWidget(m_centerWidget);
    setCentralWidget(mainWidget);
    centralWidget()->setLayout(mainLayout);


    setFixedSize(484, 682);
    move(qApp->primaryScreen()->geometry().center() - rect().center());

    connect(m_toggleButton, &QPushButton::clicked, this, &MainWindow::onToggleBtnClicked);
    connect(m_updateButton, &QPushButton::clicked, this, &MainWindow::onToggleBtnClicked);
    connect(m_rebootButton, &QPushButton::clicked, this, &MainWindow::onRebootBtnClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &MainWindow::onCancelBtnClicked);
    connect(m_okButton, &QPushButton::clicked, qApp, &QApplication::quit);

    QTimer::singleShot(0, this, &MainWindow::loadResolutions);

}

MainWindow::~MainWindow()
= default;

void MainWindow::keyPressEvent(QKeyEvent *e)
{

}

void MainWindow::noResolutions()
{
    m_tipsIcon->setVisible(true);
    m_tipsIcon->setPixmap(Utils::hidpiPixmap(":/resources/icons/fail.svg", QSize(128, 128)));
    m_warningTips->setText(tr("Your hardware is not supported currently, please wait for future version."));
    m_warningTips->setVisible(true);
    m_resolutionsWidget->setVisible(false);
    m_toggleButton->setVisible(false);
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
       qInfo() << devices;
    }
#else
    QDBusPendingReply<QString> getDeviceReply = m_graphicsDriver->GetDevice();
    getDeviceReply.waitForFinished();
    if (!getDeviceReply.isValid()) {
        qCritical() << getDeviceReply.error();
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
       iconPath = ":/resources/icons/Intel.svg";
    }

    if (bAmd && !bIntel && !bNvidia) {
        iconPath = ":/resources/icons/AMD.svg";
    }

    if (bNvidia && !bIntel && !bAmd) {
        iconPath = ":/resources/icons/NVIDIA.svg";
    }

    if (bIntel && bNvidia && !bAmd) {
        iconPath = ":/resources/icons/Intel-NVIDIA.svg";
    }

    m_vendorIcon->setPixmap(Utils::hidpiPixmap(iconPath, QSize(128, 128)));
    m_vendorName->setText(devInfo.join('\n'));

}

void MainWindow::loadResolutions()
{
    loadDevice();
    QString strResolution;
#ifdef TEST_UI
    QString path = RESOURCES_DIR"/test/intel_nvidia.json";
    QFile file(path);
    if (file.exists()) {
       file.open(QIODevice::ReadOnly);
       strResolution = QString(file.readAll());
    }
#else

    QDBusPendingReply<QString> resolutionReply = m_graphicsDriver->GetResolutionTitle();
    resolutionReply.waitForFinished();
    if (!resolutionReply.isValid()) {
        qCritical() << resolutionReply.error();
    }
    strResolution = resolutionReply.value();
#endif

    qInfo() << strResolution;
    QJsonObject resolutionRoot = Utils::QStringToJson(strResolution);

    QJsonArray resolutions = resolutionRoot["resolutions"].toArray();
    if (resolutions.empty()) {
        noResolutions();
        return;
    }

    if (resolutionRoot["type"].toInt() == INTEL_NVIDIA_USE_INTEL) {
        m_warningTips->setVisible(true);
        m_warningTips->setText(tr("Switching to the discrete graphics interface may cause a black screen."));

    } else if (resolutionRoot["type"].toInt() == INTEL_NVIDIA_USE_NVIDIA) {
        m_warningTips->setVisible(true);
        m_warningTips->setText(tr("Switching to the integrated graphics interface may cause a black screen."));
    }

    if (resolutions.size() == 1) {
        m_usedIndex = 0;
        Resolution solution(resolutions[0].toObject());
        ResolutionWidget *rw = new SingleResolutionWidget(m_graphicsDriver, solution);
        rw->initUI();
        m_resolutionsLayout->addWidget(rw);
        connect(rw, &ResolutionWidget::clicked, this, &MainWindow::onResolutionSelected);
        if (rw->canUpdate()) {
            m_updateButton->setVisible(true);
            m_toggleButton->setVisible(false);
        } else {
            m_okButton->setVisible(true);
        }
    } else {
        int index = 0;
        for (const auto res : resolutions) {
            Resolution solution(res.toObject());
            auto *rw = new ResolutionWidget(m_graphicsDriver, solution);
            rw->initUI();
            m_resolutionsLayout->addWidget(rw);
            connect(rw, &ResolutionWidget::clicked, this, &MainWindow::onResolutionSelected);
            if (solution.enable()) {
                m_usedIndex = index;
                qInfo() << "m_usedIndex = " << m_usedIndex;
                if (rw->canUpdate()) {
                    m_updateButton->setVisible(true);
                    m_toggleButton->setVisible(false);
                }
            }
            qInfo() << "index = " << index;
            index++;
        }

    }
}

void MainWindow::onResolutionSelected()
{
    auto *rw = dynamic_cast<ResolutionWidget *>(sender());
    qInfo() << "Resolution selected: " << rw->resolution().name();

    const int idx = m_resolutionsLayout->indexOf(rw);
    Q_ASSERT(idx != -1);

    m_selectedIndex = idx;

    for (int i = 0; i != m_resolutionsLayout->count(); ++i)
    {
        auto *w = dynamic_cast<ResolutionWidget *>(m_resolutionsLayout->itemAt(i)->widget());
        w->setChecked(i == idx);
    }
    qInfo() << "m_usedIndex = " << m_usedIndex;
    qInfo() << "m_selectedIndex = " << m_selectedIndex;
    const bool changed = m_selectedIndex != m_usedIndex;

    if (changed) {
        if (rw->resolution().enable() && rw->canUpdate()) {
            m_toggleButton->setVisible(false);
            m_updateButton->setVisible(true);
        } else {
            m_toggleButton->setVisible(true);
            m_updateButton->setVisible(false);
        }

    } else {
        if (rw->canUpdate()) {
            m_toggleButton->setVisible(false);
            m_updateButton->setVisible(true);
        } else {
            m_toggleButton->setVisible(false);
            m_updateButton->setVisible(false);
        }

    }
}

void MainWindow::onUpdateBtnClicked()
{
//    auto *new_driver_widget = dynamic_cast<ResolutionWidget *>(m_resolutionsLayout->itemAt(m_usedIndex)->widget());
//    new_driver_widget->prepareInstall();
//    m_toggleButton->setVisible(false);
//    m_updateButton->setVisible(false);
}

void MainWindow::onToggleBtnClicked()
{
    Q_ASSERT(m_selectedIndex != m_usedIndex);
    m_updateButton->setEnabled(false);
    m_toggleButton->setEnabled(false);
    auto *new_driver_widget = dynamic_cast<ResolutionWidget *>(m_resolutionsLayout->itemAt(m_selectedIndex)->widget());
    connect(new_driver_widget, &ResolutionWidget::preInstallProgress, this, &MainWindow::onPreInstallProgress);
    new_driver_widget->prepareInstall();
}

void MainWindow::onRebootBtnClicked()
{
    qInfo() << "onRebootBtnClicked";
    QProcess::startDetached("dbus-send --system --print-reply --dest=org.freedesktop.login1 /org/freedesktop/login1 org.freedesktop.login1.Manager.Reboot boolean:true");
}

void MainWindow::onCancelBtnClicked()
{
    qInfo() << "onCancelBtnClicked";
    QDBusPendingReply<void> reply = m_graphicsDriver->CancelInstall();
    reply.waitForFinished();
    qApp->quit();
}


void MainWindow::paintEvent(QPaintEvent *event)
{
    if (DGuiApplicationHelper::LightType == DApplicationHelper::instance()->themeType()) {
        m_centerWidget->setStyleSheet("QWidget#centerWidget{"
                                      "border-radius: 8px;"
                                      "padding:2px 4px;"
                                      "background-color: rgba(255, 255, 255, 1);"
                                      "}");

        m_vendorName->setStyleSheet("QLabel {"
                                     "font-size: 12px;"
                                     "color: #001a2e;"
                                     "}");


        m_warningTips->setStyleSheet("QLabel {"
                                  "font-size: 12px;"
                                  "color: #526a7f;"
                                  "}");

        m_installTips->setStyleSheet("QLabel {"
                                     "font-size: 14px;"
                                     "font-weight: 500;"
                                     "color: rgba(0, 0, 0, 0.9);"
                                     "}");

    } else if (DGuiApplicationHelper::DarkType == DApplicationHelper::instance()->themeType()) {
        m_centerWidget->setStyleSheet("QWidget#centerWidget{"
                                      "border-radius: 8px;"
                                      "padding:2px 4px;"
                                      "background-color: rgba(255, 255, 255, 0.05);"
                                      "}");

        m_vendorName->setStyleSheet("QLabel {"
                                    "font-size: 12px;"
                                    "color: #c0c6d4;"
                                    "}");

        m_warningTips->setStyleSheet("QLabel {"
                                  "font-size: 12px;"
                                  "color: #6d7c88;"
                                  "}");

        m_installTips->setStyleSheet("QLabel {"
                                     "font-size: 14px;"
                                     "font-weight: 500;"
                                     "}");
    }
    QWidget::paintEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QDBusPendingReply<void> reply = m_graphicsDriver->CancelInstall();
    reply.waitForFinished();
    qApp->quit();
}

void MainWindow::onPreInstallProgress(int progress)
{
    qInfo() << "preInstallerProgress = " << progress;

    if (progress >= 0 && progress < 100) {
        if (!m_startPreInstall) {
            m_resolutionsWidget->setVisible(false);
            m_toggleButton->setVisible(false);
            m_updateButton->setVisible(false);
            m_warningTips->setVisible(false);
            m_rebootButton->setVisible(true);
            m_rebootButton->setEnabled(false);
            m_cancelButton->setEnabled(false);
            m_cancelButton->setVisible(true);
            m_installTips->setText(tr("Please reboot for switching to another driver, which may cost several minutes, please wait patiently"));
            m_installTips->setVisible(true);
            m_startPreInstall = true;
        }
    } else if (progress < 0){
        m_resolutionsWidget->setVisible(false);
        m_toggleButton->setVisible(false);
        m_updateButton->setVisible(false);
        m_warningTips->setVisible(false);
        m_rebootButton->setVisible(false);
        m_cancelButton->setVisible(true);
        m_installTips->setText(tr("Preparation for installation failed"));
        m_installTips->setVisible(true);
        Utils::resetDisablePluginList();
        qCritical() << "prepareInstall failed";
    } else {
        m_resolutionsWidget->setVisible(false);
        m_toggleButton->setVisible(false);
        m_updateButton->setVisible(false);
        m_warningTips->setVisible(false);
        m_rebootButton->setVisible(true);
        m_cancelButton->setVisible(true);
        m_rebootButton->setEnabled(true);
        m_cancelButton->setEnabled(true);
        m_installTips->setText(tr("Please reboot for switching to another driver, which may cost several minutes, please wait patiently"));
        m_installTips->setVisible(true);
        Utils::setDisablePluginList();
    }
}


