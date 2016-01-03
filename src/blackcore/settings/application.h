/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_SETTINGS_APPLICATION_H
#define BLACKCORE_SETTINGS_APPLICATION_H

#include "blackmisc/settingscache.h"
#include "blackmisc/input/actionhotkeylist.h"
#include <QStringList>

namespace BlackCore
{
    namespace Settings
    {
        namespace Application
        {
            //! User configured hotkeys
            struct ActionHotkeys : public BlackMisc::CSettingTrait<BlackMisc::Input::CActionHotkeyList>
            {
                //! \copydoc BlackMisc::CSettingTrait::key
                static const char *key() { return "application/actionhotkeys"; }

                //! \copydoc BlackMisc::CSettingTrait::isValid
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
            struct EnabledSimulators : public BlackMisc::CSettingTrait<QStringList>
            {
                //! \copydoc BlackCore::CSettingTrait::key
                static const char *key() { return "application/enabledsimulators"; }

                //! \copydoc BlackCore::CSettingTrait::defaultValue
                static const QStringList &defaultValue()
                {
                    // All default simulators
                    static const QStringList enabledSimulators
                    {
                        QStringLiteral("org.swift-project.plugins.simulator.fs9"),
                        QStringLiteral("org.swift-project.plugins.simulator.fsx"),
                        QStringLiteral("org.swift-project.plugins.simulator.xplane")
                    };

                    return enabledSimulators;
                }
            };
        } // ns
    } // ns
} // ns

#endif
