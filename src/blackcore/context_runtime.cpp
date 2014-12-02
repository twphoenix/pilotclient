/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackcore/context_all_impl.h"
#include "blackcore/context_all_proxies.h"
#include "blackcore/context_all_empties.h"
#include "blackcore/blackcorefreefunctions.h"
#include "blacksim/blacksimfreefunctions.h"
#include "blackmisc/nwserver.h"
#include "blackmisc/statusmessagelist.h"
#include "blackmisc/avaircraft.h"
#include "blackmisc/blackmiscfreefunctions.h"
#include "blackmisc/logmessage.h"
#include "blackcore/context_runtime.h"

#include <QDebug>

namespace BlackCore
{
    /*
     * Constructor
     */
    CRuntime::CRuntime(const CRuntimeConfig &config, QObject *parent) : QObject(parent)
    {
        this->init(config);
    }

    /*
     * Init runtime
     */
    void CRuntime::init(const CRuntimeConfig &config)
    {
        if (m_init) return;
        BlackMisc::registerMetadata();
        BlackMisc::initResources();
        BlackSim::registerMetadata();
        BlackCore::registerMetadata();

        this->connect(QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &CRuntime::gracefulShutdown);

        // upfront reading of settings, as DBus server already relies on settings
        QString dbusAddress;
        QMap<QString, int> times;
        QTime time;
        time.start();

        // FIXME RW: We are allocating a full settings context in order to get the DBus address.
        // I wonder if this can be done cleaner.
        if (config.hasDBusAddress()) dbusAddress = config.getDBusAddress(); // bootstrap / explicit
        if (config.hasLocalSettings())
        {
            auto *settings = new CContextSettings(config.getModeSettings(), this);
            if (settings) settings->read();
            if (dbusAddress.isEmpty()) dbusAddress = settings->getNetworkSettings().getDBusServerAddress();

            settings->deleteLater();
        }

        // DBus
        if (config.requiresDBusSever()) this->initDBusServer(dbusAddress);
        if (config.requiresDBusConnection())
        {
            this->initDBusConnection(dbusAddress);
            if (!this->m_dbusConnection.isConnected())
            {
                QString notConnected("DBus connection failed");
                QString e = this->m_dbusConnection.lastError().message();
                if (!e.isEmpty()) notConnected.append(" ").append(e);
                Q_ASSERT_X(false, "CRuntime::init", notConnected.toUtf8().constData());
                qCritical() << notConnected;
            }
        }
        times.insert("DBus", time.restart());

        // contexts
        this->m_contextSettings = IContextSettings::create(this, config.getModeSettings(), this->m_dbusServer, this->m_dbusConnection);
        times.insert("Settings", time.restart());

        this->m_contextApplication = IContextApplication::create(this, config.getModeApplication(), this->m_dbusServer, this->m_dbusConnection);
        times.insert("Application", time.restart());

        this->m_contextOwnAircraft = IContextOwnAircraft::create(this, config.getModeOwnAircraft(), this->m_dbusServer, this->m_dbusConnection);
        times.insert("Own aircraft", time.restart());

        this->m_contextAudio = IContextAudio::create(this, config.getModeAudio(), this->m_dbusServer, this->m_dbusConnection);
        times.insert("Audio", time.restart());

        this->m_contextSimulator = IContextSimulator::create(this, config.getModeSimulator(), this->m_dbusServer, this->m_dbusConnection);
        times.insert("Simulator", time.restart());

        this->m_contextNetwork = IContextNetwork::create(this, config.getModeNetwork(), this->m_dbusServer, this->m_dbusConnection);
        times.insert("Network", time.restart());

        // checks --------------
        // 1. own aircraft and simulator should reside in same location
        Q_ASSERT(!this->m_contextSimulator || (this->m_contextOwnAircraft->isUsingImplementingObject() == this->m_contextSimulator->isUsingImplementingObject()));

        // 2. own aircraft and network should reside in same location
        Q_ASSERT(!this->m_contextNetwork || (this->m_contextOwnAircraft->isUsingImplementingObject() == this->m_contextNetwork->isUsingImplementingObject()));

        // post inits, wiring things among context (e.g. signal slots)
        this->initPostSetup();
        qDebug() << "Init times:" << times;

        // flag
        m_init = true;
    }

    bool CRuntime::hasRemoteApplicationContext() const
    {
        Q_ASSERT(this->m_contextApplication);
        return !this->m_contextApplication->isUsingImplementingObject();
    }

    bool CRuntime::canPingApplicationContext() const
    {
        Q_ASSERT(this->m_contextApplication);
        if (this->m_contextApplication->isUsingImplementingObject()) return true;
        qint64 token = QDateTime::currentMSecsSinceEpoch();
        return (token == this->m_contextApplication->ping(token));
    }

    bool CRuntime::parseCommandLine(const QString commandLine)
    {
        bool handled = false;
        if (this->getIContextAudio()) { handled = handled || this->getIContextAudio()->parseCommandLine(commandLine); }
        if (this->getIContextNetwork()) { handled = handled || this->getIContextNetwork()->parseCommandLine(commandLine); }
        if (this->getIContextOwnAircraft()) { handled = handled || this->getIContextOwnAircraft()->parseCommandLine(commandLine); }
        return handled;
    }

    void CRuntime::initDBusServer(const QString &dBusAddress)
    {
        if (this->m_dbusServer) return;
        Q_ASSERT(!dBusAddress.isEmpty());
        this->m_dbusServer = new CDBusServer(dBusAddress, this);
    }

    void CRuntime::initPostSetup()
    {
        bool c = false;
        Q_UNUSED(c); // for release version

        if (this->m_contextSettings && this->m_contextApplication)
        {
            c = connect(this->m_contextSettings, &IContextSettings::changedSettings,
                        this->getIContextApplication(), &IContextApplication::changeSettings);
            Q_ASSERT(c);
        }

        // local simulator?
        if (this->m_contextSimulator && this->m_contextSimulator->isUsingImplementingObject())
        {
            // only connect if simulator runs locally, no round trips
            if (this->m_contextNetwork && this->m_contextNetwork->isUsingImplementingObject())
            {
                c = connect(this->m_contextNetwork, &IContextNetwork::textMessagesReceived,
                            this->getCContextSimulator(), &CContextSimulator::ps_textMessagesReceived);
                Q_ASSERT(c);
            }

            // only if own aircraft runs locally
            if (this->m_contextOwnAircraft && this->m_contextOwnAircraft->isUsingImplementingObject())
            {
                c = connect(this->m_contextOwnAircraft, &IContextOwnAircraft::changedAircraftCockpit,
                            this->getCContextSimulator(), &CContextSimulator::ps_updateSimulatorCockpitFromContext);
                Q_ASSERT(c);
            }

            // connect local simulator and settings and load plugin
            if (this->m_contextSettings)
            {
                connect(this->m_contextSettings, &IContextSettings::changedSettings, this->m_contextSimulator, &IContextSimulator::settingsChanged);
                if (!this->m_contextSimulator->loadSimulatorPluginFromSettings())
                {
                    CLogMessage(this).warning("No simulator plugin loaded");
                }
            }
        }

        // only where network and(!) own aircraft run locally
        if (this->m_contextNetwork && this->m_contextOwnAircraft && this->m_contextNetwork->isUsingImplementingObject() && this->m_contextOwnAircraft->isUsingImplementingObject())
        {
            c = this->connect(this->m_contextNetwork, &IContextNetwork::changedAtcStationOnlineConnectionStatus,
                              this->getCContextOwnAircraft(),  &CContextOwnAircraft::ps_changedAtcStationOnlineConnectionStatus);
            Q_ASSERT(c);

            // inject updated own aircraft to network
            c = this->connect(this->m_contextOwnAircraft, &IContextOwnAircraft::changedAircraft,
                              this->getCContextNetwork(),  &CContextNetwork::ps_changedOwnAircraft);
            Q_ASSERT(c);
        }
    }

    void CRuntime::gracefulShutdown()
    {
        if (!this->m_init) return;
        this->m_init = false;

        // disable all signals towards runtime
        disconnect(this);

        // unregister all from DBus
        if (this->m_dbusServer) this->m_dbusServer->unregisterAllObjects();

        // handle contexts
        if (this->getIContextSimulator())
        {
            disconnect(this->getIContextSimulator());
            this->getIContextSimulator()->disconnectFrom();
            this->getIContextSimulator()->deleteLater();
            this->m_contextSimulator = nullptr;
        }

        // log off from network, if connected
        if (this->getIContextNetwork())
        {
            disconnect(this->getIContextNetwork());
            this->getIContextNetwork()->disconnectFromNetwork();
            if (this->m_contextNetwork->isUsingImplementingObject())
            {
                this->getCContextNetwork()->gracefulShutdown(); // for threads
            }
            this->getIContextNetwork()->deleteLater();
            // replace by dummy object avoiding nullptr issues during shutdown phase
            this->m_contextNetwork = IContextNetwork::create(this, CRuntimeConfig::NotUsed, nullptr, this->m_dbusConnection);
        }

        if (this->getIContextAudio())
        {
            disconnect(this->getIContextAudio());
            this->getIContextAudio()->deleteLater();
            this->m_contextAudio = nullptr;
        }

        if (this->getIContextOwnAircraft())
        {
            disconnect(this->getIContextOwnAircraft());
            this->getIContextOwnAircraft()->deleteLater();
            this->m_contextOwnAircraft = nullptr;
        }

        if (this->getIContextSettings())
        {
            disconnect(this->getIContextSettings());
            this->getIContextSettings()->deleteLater();
            this->m_contextSettings = nullptr;
        }

        if (this->getIContextApplication())
        {
            disconnect(this->getIContextApplication());
            this->getIContextApplication()->deleteLater();
            this->m_contextApplication = nullptr;
        }
    }

    void CRuntime::initDBusConnection(const QString &address)
    {
        if (this->m_initDBusConnection) return;
        if (address.isEmpty() || address == CDBusServer::sessionDBusServer())
            this->m_dbusConnection = QDBusConnection::sessionBus();
        else if (address == CDBusServer::systemDBusServer())
            this->m_dbusConnection = QDBusConnection::sessionBus();
        else
            this->m_dbusConnection = QDBusConnection::connectToPeer(address, "BlackBoxRuntime");
    }

    const IContextApplication *CRuntime::getIContextApplication() const
    {
        return this->m_contextApplication;
    }

    IContextApplication *CRuntime::getIContextApplication()
    {
        return this->m_contextApplication;
    }

    IContextAudio *CRuntime::getIContextAudio()
    {
        return this->m_contextAudio;
    }

    const IContextAudio *CRuntime::getIContextAudio() const
    {
        return this->m_contextAudio;
    }

    IContextNetwork *CRuntime::getIContextNetwork()
    {
        return this->m_contextNetwork;
    }

    const IContextNetwork *CRuntime::getIContextNetwork() const
    {
        return this->m_contextNetwork;
    }

    IContextOwnAircraft *CRuntime::getIContextOwnAircraft()
    {
        return this->m_contextOwnAircraft;
    }

    const IContextOwnAircraft *CRuntime::getIContextOwnAircraft() const
    {
        return this->m_contextOwnAircraft;
    }

    IContextSettings *CRuntime::getIContextSettings()
    {
        return this->m_contextSettings;
    }

    const IContextSettings *CRuntime::getIContextSettings() const
    {
        return this->m_contextSettings;
    }

    const IContextSimulator *CRuntime::getIContextSimulator() const
    {
        return this->m_contextSimulator;
    }

    IContextSimulator *CRuntime::getIContextSimulator()
    {
        return this->m_contextSimulator;
    }

    CContextAudio *CRuntime::getCContextAudio()
    {
        Q_ASSERT_X(this->m_contextAudio && this->m_contextAudio->isUsingImplementingObject(), "CCoreRuntime", "Cannot downcast to local object");
        return static_cast<CContextAudio *>(this->m_contextAudio);
    }

    const CContextAudio *CRuntime::getCContextAudio() const
    {
        Q_ASSERT_X(this->m_contextAudio && this->m_contextAudio->isUsingImplementingObject(), "CCoreRuntime", "Cannot downcast to local object");
        return static_cast<CContextAudio *>(this->m_contextAudio);
    }

    CContextApplication *CRuntime::getCContextApplication()
    {
        Q_ASSERT_X(this->m_contextApplication && this->m_contextApplication->isUsingImplementingObject(), "CCoreRuntime", "Cannot downcast to local object");
        return static_cast<CContextApplication *>(this->m_contextApplication);
    }

    const CContextApplication *CRuntime::getCContextApplication() const
    {
        Q_ASSERT_X(this->m_contextApplication && this->m_contextApplication->isUsingImplementingObject(), "CCoreRuntime", "Cannot downcast to local object");
        return static_cast<CContextApplication *>(this->m_contextApplication);
    }

    CContextNetwork *CRuntime::getCContextNetwork()
    {
        Q_ASSERT_X(this->m_contextNetwork && this->m_contextNetwork->isUsingImplementingObject(), "CCoreRuntime", "Cannot downcast to local object");
        return static_cast<CContextNetwork *>(this->m_contextNetwork);
    }

    const CContextNetwork *CRuntime::getCContextNetwork() const
    {
        Q_ASSERT_X(this->m_contextNetwork && this->m_contextNetwork->isUsingImplementingObject(), "CCoreRuntime", "Cannot downcast to local object");
        return static_cast<CContextNetwork *>(this->m_contextNetwork);
    }

    CContextOwnAircraft *CRuntime::getCContextOwnAircraft()
    {
        Q_ASSERT_X(this->m_contextOwnAircraft && this->m_contextOwnAircraft->isUsingImplementingObject(), "CCoreRuntime", "Cannot downcast to local object");
        return static_cast<CContextOwnAircraft *>(this->m_contextOwnAircraft);
    }

    const CContextOwnAircraft *CRuntime::getCContextOwnAircraft() const
    {
        Q_ASSERT_X(this->m_contextOwnAircraft && this->m_contextOwnAircraft->isUsingImplementingObject(), "CCoreRuntime", "Cannot downcast to local object");
        return static_cast<CContextOwnAircraft *>(this->m_contextOwnAircraft);
    }

    CContextSimulator *CRuntime::getCContextSimulator()
    {
        Q_ASSERT_X(this->m_contextSimulator && this->m_contextSimulator->isUsingImplementingObject(), "CCoreRuntime", "Cannot downcast to local object");
        return static_cast<CContextSimulator *>(this->m_contextSimulator);
    }

    const CContextSimulator *CRuntime::getCContextSimulator() const
    {
        Q_ASSERT_X(this->m_contextSimulator && this->m_contextSimulator->isUsingImplementingObject(), "CCoreRuntime", "Cannot downcast to local object");
        return static_cast<CContextSimulator *>(this->m_contextSimulator);
    }

    bool CRuntimeConfig::requiresDBusSever() const
    {
        return (this->m_application == LocalInDbusServer ||
                this->m_audio == LocalInDbusServer ||
                this->m_network == LocalInDbusServer ||
                this->m_ownAircraft == LocalInDbusServer ||
                this->m_settings == LocalInDbusServer ||
                this->m_simulator == LocalInDbusServer);
    }

    bool CRuntimeConfig::requiresDBusConnection() const
    {
        return (this->m_application == Remote ||
                this->m_audio == Remote ||
                this->m_network == Remote ||
                this->m_ownAircraft == Remote ||
                this->m_settings == Remote ||
                this->m_simulator == Remote);
    }

    const CRuntimeConfig &CRuntimeConfig::forCoreAllLocalInDBus(const QString &dbusBootstrapAddress)
    {
        static CRuntimeConfig cfg = CRuntimeConfig(CRuntimeConfig(CRuntimeConfig::LocalInDbusServer, dbusBootstrapAddress));
        return cfg;
    }

    const CRuntimeConfig &CRuntimeConfig::forCoreAllLocalInDBusNoAudio(const QString &dbusBootstrapAddress)
    {
        static CRuntimeConfig cfg = CRuntimeConfig(CRuntimeConfig(CRuntimeConfig::LocalInDbusServer, dbusBootstrapAddress));
        cfg.m_audio = CRuntimeConfig::NotUsed;
        return cfg;
    }

    const CRuntimeConfig &CRuntimeConfig::local(const QString &dbusBootstrapAddress)
    {
        static CRuntimeConfig cfg = CRuntimeConfig(CRuntimeConfig(CRuntimeConfig::Local, dbusBootstrapAddress));
        return cfg;
    }

    const CRuntimeConfig &CRuntimeConfig::remote(const QString &dbusBootstrapAddress)
    {
        static CRuntimeConfig cfg = CRuntimeConfig(CRuntimeConfig(CRuntimeConfig::Remote, dbusBootstrapAddress));
        return cfg;
    }

    const CRuntimeConfig &CRuntimeConfig::remoteLocalAudio(const QString &dbusBootstrapAddress)
    {
        static CRuntimeConfig cfg = CRuntimeConfig(CRuntimeConfig(CRuntimeConfig::Remote, dbusBootstrapAddress));
        cfg.m_audio = CRuntimeConfig::Local;
        return cfg;
    }
} // namespace
