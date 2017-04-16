/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackcore/application.h"
#include "blackcore/pluginmanagersimulator.h"
#include "blackcore/simulator.h"
#include "blackmisc/directoryutils.h"
#include "blackmisc/logmessage.h"

#include <QJsonObject>
#include <QJsonValue>
#include <QStringBuilder>
#include <QtGlobal>

namespace BlackCore
{

    using namespace BlackMisc;
    using namespace BlackMisc::Simulation;

    CPluginManagerSimulator::CPluginManagerSimulator(QObject *parent) : IPluginManager(parent)
    {
    }

    ISimulatorFactory *CPluginManagerSimulator::getFactory(const QString &pluginId)
    {
        return getPluginById<ISimulatorFactory>(pluginId);
    }

    ISimulatorListener *CPluginManagerSimulator::createListener(const QString &pluginId)
    {
        if (!m_plugins.contains(pluginId))
        {
            return nullptr;
        }

        PluginExtended &plugin = m_plugins[pluginId];
        if (!plugin.listener)
        {
            ISimulatorFactory *factory = getFactory(pluginId);
            if (!factory)
            {
                CLogMessage(this).warning("Could not load plugin %1.") << pluginId;
                m_plugins.remove(pluginId);
                return nullptr;
            }

            ISimulatorListener *listener = factory->createListener(plugin.info);
            connect(qApp, &QCoreApplication::aboutToQuit, listener, &QObject::deleteLater);
            plugin.listener = listener;
        }

        return plugin.listener;
    }

    ISimulatorListener *CPluginManagerSimulator::getListener(const QString &pluginId)
    {
        if (!m_plugins.contains(pluginId))
        {
            return nullptr;
        }

        PluginExtended &plugin = m_plugins[pluginId];
        return plugin.listener;
    }

    CSimulatorPluginInfoList CPluginManagerSimulator::getAvailableSimulatorPlugins() const
    {
        CSimulatorPluginInfoList list;
        for (const auto &i : m_plugins)
        {
            list.push_back(i.info);
        }
        return list;
    }

    void CPluginManagerSimulator::collectPlugins()
    {
        IPluginManager::collectPlugins();

        const CSequence<QJsonObject> &plugins = getPlugins();
        for (const QJsonObject &json : plugins)
        {
            QString iid = json["IID"].toString();
            if (iid == QStringLiteral("org.swift-project.blackcore.simulatorinterface")) {
                auto it = m_plugins.insert(pluginIdentifier(json), {});
                it->info.convertFromJson(json);
            }
        }
    }

    BlackMisc::CSequence<QString> CPluginManagerSimulator::acceptedIids() const
    {
        return {
            QStringLiteral("org.swift-project.blackcore.simulatorinterface"),
            QStringLiteral("org.swift-project.blackgui.pluginconfiginterface")
        };
    }

    QString CPluginManagerSimulator::pluginDirectory() const
    {
        return CDirectoryUtils::applicationDirectoryPath() % QStringLiteral("/plugins/simulator");
    }

}
