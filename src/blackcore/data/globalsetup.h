/* Copyright (C) 2015
 * swift project community / contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution. No part of swift project, including this file, may be copied, modified, propagated,
 * or distributed except according to the terms contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_DATA_GLOBALSETUP_H
#define BLACKCORE_DATA_GLOBALSETUP_H

#include "blackcore/blackcoreexport.h"
#include "blackmisc/network/serverlist.h"
#include "blackmisc/network/url.h"
#include "blackmisc/network/urllist.h"
#include "blackmisc/identifiable.h"
#include "blackmisc/datacache.h"
#include "blackmisc/metaclass.h"
#include "blackmisc/propertyindex.h"
#include "blackmisc/timestampbased.h"
#include "blackmisc/valueobject.h"
#include "blackmisc/variant.h"

#include <QMetaType>
#include <QString>

BLACK_DECLARE_VALUEOBJECT_MIXINS(BlackCore::Data, CGlobalSetup)

namespace BlackCore::Data
{
    //! Global settings for readers, debug flags, etc.
    //! \note also called the bootstrap file as it is required once to get information where all the data are located
    class BLACKCORE_EXPORT CGlobalSetup :
        public BlackMisc::CValueObject<CGlobalSetup>,
        public BlackMisc::CIdentifiable,
        public BlackMisc::ITimestampBased
    {
    public:
        //! Properties by index
        enum ColumnIndex
        {
            IndexDbRootDirectory = BlackMisc::CPropertyIndexRef::GlobalIndexCGlobalSetup,
            IndexDbHttpPort,
            IndexDbHttpsPort,
            IndexDbLoginService,
            IndexDbClientPingService,
            IndexVatsimStatus,
            IndexVatsimBookings,
            IndexVatsimMetars,
            IndexVatsimData,
            IndexSwiftDbFiles,
            IndexBootstrapFileUrls,
            IndexUpdateInfoFileUrls,
            IndexOnlineHelpUrls,
            IndexCrashReportServerUrl,
            IndexWasLoadedFromWeb,
            IndexWasLoadedFromFile,
            IndexSharedUrls,
            IndexMappingMinimumVersion,
            IndexPredefinedServers
        };

        //! Add info when pinging
        enum PingTypeFlag
        {
            PingUnspecific = 0,
            PingLogoff     = 1 << 0,
            PingStarted    = 1 << 1,
            PingShutdown   = 1 << 2,
            PingCompleteShutdown = PingLogoff | PingShutdown
        };
        Q_DECLARE_FLAGS(PingType, PingTypeFlag)

        //! Default constructor
        CGlobalSetup();

        //! Has data loaded from web
        bool wasLoadedFromWeb() const { return m_wasLoadedFromWeb; }

        //! Has data loaded from file
        bool wasLoadedFromFile() const { return m_wasLoadedFromFile; }

        //! Loaded (web/file)
        bool wasLoaded() const;

        //! Mark as loaded from web
        void markAsLoadedFromWeb(bool loaded) { m_wasLoadedFromWeb = loaded; }

        //! Mark as loaded from file
        void markAsLoadedFromFile(bool loaded) { m_wasLoadedFromFile = loaded; }

        //! Http port
        int getDbHttpPort() const { return m_dbHttpPort; }

        //! Https port
        int getDbHttpsPort() const { return m_dbHttpsPort; }

        //! Debug flag
        bool dbDebugFlag() const;

        //! Set debug flag
        void setServerDebugFlag(bool debug);

        //! Same type?
        bool hasSameType(const CGlobalSetup &otherSetup) const;

        //! Crash report server url
        //! \deprecated NOT used anymore as by RR's info: https://discordapp.com/channels/539048679160676382/539925070550794240/586879411002015756
        BlackMisc::Network::CUrl getCrashReportServerUrl() const { return m_crashReportServerUrl; }

        //! Root directory of DB
        const BlackMisc::Network::CUrl &getDbRootDirectoryUrl() const { return m_dbRootDirectoryUrl; }

        //! ICAO reader URL
        //! \remark based on getDbRootDirectoryUrl
        BlackMisc::Network::CUrl getDbIcaoReaderUrl() const;

        //! Model reader URL
        //! \remark based on getDbRootDirectoryUrl
        BlackMisc::Network::CUrl getDbModelReaderUrl() const;

        //! Airport reader URL
        //! \remark based on getDbRootDirectoryUrl
        BlackMisc::Network::CUrl getDbAirportReaderUrl() const;

        //! Info data reader URL
        //! \remark based on getDbRootDirectoryUrl
        BlackMisc::Network::CUrl getDbInfoReaderUrl() const;

        //! Home page url
        //! \remark based on getDbRootDirectoryUrl
        BlackMisc::Network::CUrl getDbHomePageUrl() const;

        //! Legal directory URL
        //! \remark based on getDbRootDirectoryUrl
        BlackMisc::Network::CUrl getLegalDirectoryUrl() const;

        //! Login service
        //! \remark based on getDbRootDirectoryUrl
        BlackMisc::Network::CUrl getDbLoginServiceUrl() const;

        //! DB ping service
        //! \remark based on getDbRootDirectoryUrl
        BlackMisc::Network::CUrl getDbClientPingServiceUrl() const;

        //! Ping the DB server, fire and forget (no feedback etc)
        BlackMisc::Network::CUrl getDbClientPingServiceUrl(PingType type) const;

        //! Seconds between pings
        qint64 getDbClientPingIntervalSecs() const { return m_pingIntervalSecs; }

        //! alpha XSwiftBus files available
        BlackMisc::Network::CUrl getAlphaXSwiftBusFilesServiceUrl() const;

        //! Shared URLs
        const BlackMisc::Network::CUrlList &getSwiftSharedUrls() const;

        //! Get pure shared URL as in getSwiftSharedUrls from bootstrap, distribution or other shared URL
        //! \remark normally based on one of the getSwiftSharedUrls
        BlackMisc::Network::CUrl getCorrespondingSharedUrl(const BlackMisc::Network::CUrl &candidate) const;

        //! Bootstrap URLs
        //! \remark based on getSwiftSharedUrls
        BlackMisc::Network::CUrlList getSwiftBootstrapFileUrls() const;

        //! Distribution URLs
        //! \remark based on getSwiftSharedUrls
        BlackMisc::Network::CUrlList getSwiftUpdateInfoFileUrls() const;

        //! URL to read VATSIM bookings
        const BlackMisc::Network::CUrl &getVatsimBookingsUrl() const { return m_vatsimBookingsUrl; }

        //! VATSIM METAR URL
        const BlackMisc::Network::CUrlList &getVatsimMetarsUrls() const { return m_vatsimMetarsUrls; }

        //! VATSIM status file URLs
        const BlackMisc::Network::CUrlList &getVatsimStatusFileUrls() const { return m_vatsimStatusFileUrls; }

        //! VATSIM data file URLs
        const BlackMisc::Network::CUrlList &getVatsimDataFileUrls() const { return m_vatsimDataFileUrls; }

        //! Help page URL
        //! \remark working URL evaluated at runtime, based on getOnlineHelpUrls
        BlackMisc::Network::CUrl getHelpPageUrl(const QString &context = {}) const;

        //! Predefined servers
        const BlackMisc::Network::CServerList &getPredefinedServers() const { return m_predefinedServers; }

        //! Predefined plus hardcoded
        BlackMisc::Network::CServerList getPredefinedServersPlusHardcodedServers() const;

        //! Is server a development server?
        bool isDevelopment() const { return m_development; }

        //! Creating mappings requires at least this version or higher
        //! \remark only valid if wasLoaded() is \c true
        const QString &getMappingMinimumVersionString() const { return m_mappingMinimumVersion; }

        //! Meets the minimum mapping version
        //! \remark only valid if wasLoaded() is \c true
        bool isSwiftVersionMinimumMappingVersion() const;

        //! Productive settings?
        void setDevelopment(bool development) { m_development  = development; }

        //! NCEP GFS Forecasts (0.50 degree grid) data url
        BlackMisc::Network::CUrl getNcepGlobalForecastSystemUrl() const { return m_ncepGlobalForecastSystemUrl; }

        //! NCEP GFS Forecasts (0.25 degree grid) data url
        BlackMisc::Network::CUrl getNcepGlobalForecastSystemUrl25() const { return m_ncepGlobalForecastSystemUrl25; }

        //! \copydoc BlackMisc::Mixin::String::toQString
        QString convertToQString(bool i18n = false) const;

        //! To string with separator
        QString convertToQString(const QString &separator, bool i18n = false) const;

        //! \copydoc BlackMisc::Mixin::Index::propertyByIndex
        QVariant propertyByIndex(BlackMisc::CPropertyIndexRef index) const;

        //! \copydoc BlackMisc::Mixin::Index::setPropertyByIndex
        void setPropertyByIndex(BlackMisc::CPropertyIndexRef index, const QVariant &variant);

        //! Schema version (shared files, bootstrap file)
        static const QString &schemaVersionString();

        //! Build bootstrap file URL from shared URL
        static QString buildBootstrapFileUrl(const QString &candidate);

        //! Build the full dbdata directory URL from shared URL
        static BlackMisc::Network::CUrl buildDbDataDirectoryUrl(const BlackMisc::Network::CUrl &candidate);

        //! Object initialized by JSON file
        static CGlobalSetup fromJsonFile(const QString &fileNameAndPath, bool acceptCacheFormat);

    private:
        bool                            m_wasLoadedFromWeb = false;    //!< Loaded from web
        bool                            m_wasLoadedFromFile = false;   //!< Loaded from local file
        int                             m_dbHttpPort  = 80;            //!< port
        int                             m_dbHttpsPort = 443;           //!< SSL port
        qint64                          m_pingIntervalSecs = 180;      //!< seconds between datastore pings
        bool                            m_development = false;         //!< dev. version?
        QString                         m_mappingMinimumVersion;       //!< minimum version
        BlackMisc::Network::CUrl        m_crashReportServerUrl;        //!< crash report server
        BlackMisc::Network::CUrl        m_dbRootDirectoryUrl;          //!< Root directory of DB
        BlackMisc::Network::CUrl        m_vatsimBookingsUrl;           //!< ATC bookings
        BlackMisc::Network::CUrlList    m_vatsimMetarsUrls;            //!< METAR data
        BlackMisc::Network::CUrlList    m_vatsimStatusFileUrls;        //!< Status file, where to find the VATSIM files (METAR, data, ATIS, other status files)
        BlackMisc::Network::CUrlList    m_vatsimDataFileUrls;          //!< Overall VATSIM data file / merely for bootstrapping the first time
        BlackMisc::Network::CUrlList    m_sharedUrls;                  //!< where we can obtain shared info files such as bootstrap, ..
        BlackMisc::Network::CUrlList    m_onlineHelpUrls;              //!< online help URLs
        BlackMisc::Network::CServerList m_predefinedServers;           //!< Predefined servers loaded from setup file
        BlackMisc::Network::CUrl        m_ncepGlobalForecastSystemUrl; //!< NCEP GFS url 0.5 degree resolution
        BlackMisc::Network::CUrl        m_ncepGlobalForecastSystemUrl25; //!< NCEP GFS url 0.25 degree resolution

        // transient members, to be switched on/off via GUI or set from reader
        bool m_dbDebugFlag = false; //!< can trigger DEBUG on the server, so you need to know what you are doing

        //! Set the default URLs
        void initDefaultValues();

        BLACK_METACLASS(
            CGlobalSetup,
            BLACK_METAMEMBER(wasLoadedFromWeb),
            BLACK_METAMEMBER(wasLoadedFromFile),
            BLACK_METAMEMBER(timestampMSecsSinceEpoch),
            BLACK_METAMEMBER(crashReportServerUrl),
            BLACK_METAMEMBER(dbRootDirectoryUrl),
            BLACK_METAMEMBER(dbHttpPort),
            BLACK_METAMEMBER(dbHttpsPort),
            BLACK_METAMEMBER(pingIntervalSecs),
            BLACK_METAMEMBER(vatsimStatusFileUrls),
            BLACK_METAMEMBER(vatsimDataFileUrls),
            BLACK_METAMEMBER(vatsimBookingsUrl),
            BLACK_METAMEMBER(vatsimMetarsUrls),
            BLACK_METAMEMBER(sharedUrls),
            BLACK_METAMEMBER(onlineHelpUrls),
            BLACK_METAMEMBER(predefinedServers),
            BLACK_METAMEMBER(development),
            BLACK_METAMEMBER(mappingMinimumVersion),
            BLACK_METAMEMBER(ncepGlobalForecastSystemUrl),
            BLACK_METAMEMBER(ncepGlobalForecastSystemUrl25),
            BLACK_METAMEMBER(dbDebugFlag, BlackMisc::DisabledForJson)
        );
    };

    //! Trait for global setup data
    struct TGlobalSetup : public BlackMisc::TDataTrait<CGlobalSetup>
    {
        //! Key in data cache
        static const char *key() { return "bootstrap"; }

        //! First load is synchronous
        static constexpr bool isPinned() { return true; }
    };
} // ns

Q_DECLARE_METATYPE(BlackCore::Data::CGlobalSetup)
Q_DECLARE_METATYPE(BlackCore::Data::CGlobalSetup::PingTypeFlag)
Q_DECLARE_METATYPE(BlackCore::Data::CGlobalSetup::PingType)
Q_DECLARE_OPERATORS_FOR_FLAGS(BlackCore::Data::CGlobalSetup::PingType)

#endif // guard
