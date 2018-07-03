/* Copyright (C) 2016
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_NETWORK_SETTINGS_SERVERS_H
#define BLACKMISC_NETWORK_SETTINGS_SERVERS_H

#include "blackmisc/settingscache.h"
#include "blackmisc/valueobject.h"
#include "blackmisc/network/serverlist.h"

namespace BlackMisc
{
    namespace Network
    {
        namespace Settings
        {
            /*!
             * Virtual air traffic servers
             */
            struct TTrafficServers : public TSettingTrait<CServerList>
            {
                //! \copydoc BlackMisc::TSettingTrait::key
                static const char *key() { return "network/trafficservers"; }

                //! \copydoc BlackCore::TSettingTrait::humanReadable
                static const QString &humanReadable() { static const QString name("Traffic servers"); return name; }
            };

            /*!
             * Currently selected virtual air traffic server
             */
            struct TCurrentTrafficServer : public TSettingTrait<CServer>
            {
                //! \copydoc BlackMisc::TSettingTrait::key
                static const char *key() { return "network/currenttrafficserver"; }

                //! \copydoc BlackCore::TSettingTrait::humanReadable
                static const QString &humanReadable() { static const QString name("Current traffic servers"); return name; }

                //! \copydoc BlackMisc::TSettingTrait::defaultValue
                static const BlackMisc::Network::CServer &defaultValue()
                {
                    static const CServer dv = CServer::swiftFsdTestServer();
                    return dv;
                }
            };
        } // ns
    } // ns
} // ns

#endif
