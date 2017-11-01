
#include "mainwindow.h"
#include "widgets/resolutionwidget.h"

#include <QApplication>
#include <QScreen>
#include <QKeyEvent>
#include <QDebug>
#include <QTimer>
#include <QPushButton>
#include <QLabel>

#include <DTitlebar>

MainWindow::MainWindow(QWidget *parent) :
    DMainWindow(parent),

    m_resolutions(ResolutionsBuilder(m_devInfo).build())
{
    m_toggleButton = new QPushButton;
    m_toggleButton->setText(tr("Toggle"));
    m_toggleButton->setFixedHeight(38);

    m_topTips = new QLabel;
    m_topTips->setAlignment(Qt::AlignHCenter);
    m_topTips->setVisible(false);
    m_topTips->setText("Top Tips");

    m_botTips = new QLabel;
    m_botTips->setAlignment(Qt::AlignHCenter);
    m_botTips->setVisible(false);
    m_botTips->setText("Bot Tips");

    m_okButton = new QPushButton;
    m_okButton->setText(tr("OK"));
    m_okButton->setFixedHeight(38);

    m_rebootButton = new QPushButton;
    m_rebootButton->setText(tr("Reboot"));
    m_rebootButton->setFixedHeight(38);
    m_rebootButton->setVisible(false);

    m_progress = new DWaterProgress;
    m_progress->setTextVisible(false);
    m_progress->setFixedSize(128, 128);
    m_progress->setValue(50);
    m_progress->setVisible(false);

    m_tipsIcon = new QLabel;
    m_tipsIcon->setAlignment(Qt::AlignCenter);
    m_tipsIcon->setPixmap(QPixmap(":/resources/icons/" + m_resolutions.iconName()));
    m_vendorsName = new QLabel;
    m_vendorsName->setWordWrap(true);
    m_vendorsName->setAlignment(Qt::AlignCenter);
    m_vendorsName->setText(m_devInfo.devices().toList().join('\n'));

    m_resolutionsLayout = new QVBoxLayout;
    m_resolutionsLayout->setContentsMargins(8, 8, 8, 8);
    m_resolutionsLayout->setSpacing(0);
    m_resolutionsWidget = new QWidget;
    m_resolutionsWidget->setLayout(m_resolutionsLayout);
    m_resolutionsWidget->setObjectName("ResolutionsWidget");
    m_resolutionsWidget->setStyleSheet("QWidget #ResolutionsWidget {"
                                       "border: 1px solid #eee;"
                                       "border-radius: 3px;"
                                       "}");

    QVBoxLayout *centralLayout = new QVBoxLayout;
    centralLayout->addWidget(m_topTips);
    centralLayout->addWidget(m_tipsIcon);
    centralLayout->addWidget(m_vendorsName);
    centralLayout->addStretch();
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

    DTitlebar *tbar = titlebar();
    tbar->setTitle(QString());

    setCentralWidget(new QWidget);
    centralWidget()->setLayout(centralLayout);

    setFixedSize(440, 600);
    move(qApp->primaryScreen()->geometry().center() - rect().center());

    connect(m_toggleButton, &QPushButton::clicked, this, &MainWindow::onToggleBtnClicked);
    connect(m_rebootButton, &QPushButton::clicked, this, &MainWindow::onRebootBtnClicked);
    connect(m_okButton, &QPushButton::clicked, qApp, &QApplication::quit);

    QTimer::singleShot(0, this, &MainWindow::loadResolutions);
}

MainWindow::~MainWindow()
{
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
#ifdef QT_DEBUG
    case Qt::Key_Escape:        qApp->quit();       break;
#endif
    default:;
    }

    return QWidget::keyPressEvent(e);
}

void MainWindow::loadResolutions()
{
    QList<ResolutionWidget *> rwList;
    for (const auto &r : m_resolutions.resolutions())
    {
        ResolutionWidget *rw = new ResolutionWidget(r);
        m_resolutionsLayout->addWidget(rw);
        rwList << rw;

        connect(rw, &ResolutionWidget::clicked, this, &MainWindow::onResolutionSelected);
    }

    const QString &script = m_resolutions.statusScript();
    if (script.isEmpty())
        return;

    QProcess *proc = new QProcess;
    QPROCESS_DUMP(proc);
    QPROCESS_DELETE_SELF(proc);

    EXECUTE_SCRIPT(proc, script);
    proc->waitForFinished();
    const int index = proc->exitCode();
    m_usedIndex = index;

    if (index >= 0 && index < rwList.size())
        emit rwList[index]->clicked();
}

void MainWindow::onResolutionSelected()
{
    ResolutionWidget *rw = static_cast<ResolutionWidget *>(sender());
    qDebug() << "Resolution selected: " << rw->resolution().name();

    const int idx = m_resolutionsLayout->indexOf(rw);
    Q_ASSERT(idx != -1);

    m_selectedIndex = idx;

    for (int i(0); i != m_resolutionsLayout->count(); ++i)
    {
        ResolutionWidget *w = static_cast<ResolutionWidget *>(m_resolutionsLayout->itemAt(i)->widget());
        w->setChecked(i == idx);
    }

    const bool changed = m_selectedIndex != m_usedIndex;
    m_toggleButton->setVisible(changed);
    m_okButton->setVisible(!changed);
}

void MainWindow::onToggleBtnClicked()
{
    Q_ASSERT(m_selectedIndex != m_usedIndex);

    ResolutionWidget *w = static_cast<ResolutionWidget *>(m_resolutionsLayout->itemAt(m_selectedIndex)->widget());
    w->prepareInstall();

    connect(w, &ResolutionWidget::prepareFinished, this, &MainWindow::onPrepareFinished);

    // toggle UI
    m_topTips->setText(tr("Install..."));
    m_topTips->setVisible(true);
    m_botTips->setText(tr("Please Wait..."));
    m_botTips->setVisible(true);
    m_tipsIcon->setVisible(false);
    m_vendorsName->setVisible(false);
    m_resolutionsWidget->setVisible(false);
    m_toggleButton->setVisible(false);
    m_progress->setVisible(true);
    m_progress->start();
}

void MainWindow::onRebootBtnClicked()
{
    QProcess::startDetached("dbus-send --print-reply --dest=com.deepin.dde.shutdownFront /com/deepin/dde/shutdownFront com.deepin.dde.shutdownFront.Shutdown");
}

void MainWindow::onPrepareFinished(const int exitCode)
{
    m_progress->setVisible(false);
    m_progress->stop();
    m_tipsIcon->setVisible(true);

    if (exitCode)
    {
        m_topTips->setText(tr("Fail"));
        m_botTips->setText(tr("Failed, please upload log file."));
        m_tipsIcon->setPixmap(QPixmap(":/resources/icons/fail.png"));
        m_okButton->setVisible(true);
    }
    else
    {
        m_topTips->setText(tr("Success"));
        m_botTips->setText(tr("Congratulations!"));
        m_tipsIcon->setPixmap(QPixmap(":/resources/icons/success.png"));
        m_rebootButton->setVisible(true);
    }
}
