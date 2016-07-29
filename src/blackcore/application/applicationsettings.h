/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_APPLICATION_SETTINGS_H
#define BLACKCORE_APPLICATION_SETTINGS_H

#include "blackmisc/settingscache.h"
#include "blackconfig/buildconfig.h"
#include "blackmisc/input/actionhotkeylist.h"
#include <QStringList>

namespace BlackCore
{
    namespace Application
    {
        //! DBus server address
        struct TDBusServerAddress : public BlackMisc::TSettingTrait<QString>
        {
            //! \copydoc BlackMisc::TSettingTrait::key
            static const char *key() { return "network/dbusserver"; }

            //! \copydoc BlackMisc::TSettingTrait::defaultValue
            static const QString &defaultValue() { static const QString dv("session"); return dv; }
        };

        //! User configured hotkeys
        struct TActionHotkeys : public BlackMisc::TSettingTrait<BlackMisc::Input::CActionHotkeyList>
        {
            //! \copydoc BlackMisc::TSettingTrait::key
            static const char *key() { return "application/actionhotkeys"; }

            //! \copydoc BlackMisc::TSettingTrait::isValid
            static bool isValid(const BlackMisc::Input::CActionHotkeyList &value)
            {
                for (const auto &actionHotkey : value)
                {
                    if (actionHotkey.getApplicableMachine().getMachineName().isEmpty() ||
                            actionHotkey.getAction().isEmpty() ||
                            actionHotkey.getCombination().isEmpty()) { return false; }
                }
                return true;
            }
        };

        //! Selected simulator plugins
        struct TEnabledSimulators : public BlackMisc::TSettingTrait<QStringList>
        {
            //! \copydoc BlackCore::TSettingTrait::key
            static const char *key() { return "application/enabledsimulators"; }

            //! \copydoc BlackCore::TSettingTrait::defaultValue
            static const QStringList &defaultValue()
            {
                // All default simulators
                static const QStringList enabledSimulators(defaultValueImpl());
                return enabledSimulators;
            }

        private:
            //! Determine default value
            static QStringList defaultValueImpl()
            {
                if (BlackConfig::CBuildConfig::isRunningOnWindows10())
                {
                    // On WIN10 we have no direct play, so disable FS9 as per default
                    return QStringList
                    {
                        QStringLiteral("org.swift-project.plugins.simulator.fsx"),
                        QStringLiteral("org.swift-project.plugins.simulator.xplane")
                    };
                }
                else if (BlackConfig::CBuildConfig::isRunningOnUnixPlatform())
                {
                    // On UNIX we likely run XP
                    return QStringList { QStringLiteral("org.swift-project.plugins.simulator.xplane") };
                }

                return QStringList
                {
                    QStringLiteral("org.swift-project.plugins.simulator.fs9"),
                    QStringLiteral("org.swift-project.plugins.simulator.fsx"),
                    QStringLiteral("org.swift-project.plugins.simulator.xplane")
                };
            }
        };

        //! Uploading of crash dumps is enabled or disabled
        struct TCrashDumpUploadEnabled : public BlackMisc::TSettingTrait<bool>
        {
            //! \copydoc BlackCore::TSettingTrait::key
            static const char *key() { return "application/crashdumpuploadenabled"; }

            //! \copydoc BlackCore::TSettingTrait::defaultValue
            static bool defaultValue() { return true; }
        };

    } // ns
} // ns

#endif
