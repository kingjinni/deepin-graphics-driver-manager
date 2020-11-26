/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp ../com.deepin.daemon.GraphicsDriver.xml -i graphicsdriverinterface.h -a graphicsdriveradaptor
 *
 * qdbusxml2cpp is Copyright (C) 2017 The Qt Company Ltd.
 *
 * This is an auto-generated file.
 * This file may have been hand-edited. Look for HAND-EDIT comments
 * before re-generating it.
 */

#ifndef GRAPHICSDRIVERADAPTOR_H
#define GRAPHICSDRIVERADAPTOR_H

#include <QtCore/QObject>
#include <QtDBus/QtDBus>
#include "graphicsdriverinterface.h"
QT_BEGIN_NAMESPACE
class QByteArray;
template<class T> class QList;
template<class Key, class Value> class QMap;
class QString;
class QStringList;
class QVariant;
QT_END_NAMESPACE

/*
 * Adaptor class for interface com.deepin.daemon.GraphicsDriver
 */
class GraphicsDriverAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.daemon.GraphicsDriver")
    Q_CLASSINFO("D-Bus Introspection", ""
"  <interface name=\"com.deepin.daemon.GraphicsDriver\">\n"
"    <signal name=\"PreInstallState\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"state\"/>\n"
"    </signal>\n"
"    <signal name=\"RealInstallState\">\n"
"      <arg direction=\"out\" type=\"s\" name=\"state\"/>\n"
"    </signal>\n"
"    <method name=\"GetDevice\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"GetResolutionTitle\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"PrepareInstall\">\n"
"      <arg direction=\"in\" type=\"s\" name=\"name\"/>\n"
"    </method>\n"
"    <method name=\"TestInstall\"/>\n"
"    <method name=\"IsTestSuccess\">\n"
"      <arg direction=\"out\" type=\"b\"/>\n"
"    </method>\n"
"    <method name=\"RealInstaller\"/>\n"
"    <method name=\"GetOldDriverName\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"GetNewDriverName\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"    <method name=\"GetCurrDriverName\">\n"
"      <arg direction=\"out\" type=\"s\"/>\n"
"    </method>\n"
"  </interface>\n"
        "")
public:
    GraphicsDriverAdaptor(QObject *parent);
    virtual ~GraphicsDriverAdaptor();

public: // PROPERTIES
public Q_SLOTS: // METHODS
    QString GetCurrDriverName();
    QString GetDevice();
    QString GetNewDriverName();
    QString GetOldDriverName();
    QString GetResolutionTitle();
    bool IsTestSuccess();
    void PrepareInstall(const QString &name);
    void RealInstaller();
    void TestInstall();
Q_SIGNALS: // SIGNALS
    void PreInstallState(const QString &state);
    void RealInstallState(const QString &state);
};

#endif
