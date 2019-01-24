/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKCORE_CONTEXT_CONTEXTSIMULATOR_IMPL_H
#define BLACKCORE_CONTEXT_CONTEXTSIMULATOR_IMPL_H

#include "blackcore/context/contextsimulator.h"
#include "blackcore/application/applicationsettings.h"
#include "blackcore/simulator.h"
#include "blackcore/corefacadeconfig.h"
#include "blackcore/aircraftmatcher.h"
#include "blackcore/blackcoreexport.h"
#include "blackcore/weathermanager.h"
#include "blackcore/network.h"
#include "blackmisc/simulation/data/modelcaches.h"
#include "blackmisc/simulation/settings/modelmatchersettings.h"
#include "blackmisc/simulation/settings/simulatorsettings.h"
#include "blackmisc/simulation/settings/interpolationrenderingsetupsettings.h"
#include "blackmisc/simulation/backgroundvalidation.h"
#include "blackmisc/simulation/aircraftmodellist.h"
#include "blackmisc/simulation/interpolationsetuplist.h"
#include "blackmisc/simulation/remoteaircraftprovider.h"
#include "blackmisc/simulation/simulatorplugininfolist.h"
#include "blackmisc/simulation/simulatorinternals.h"
#include "blackmisc/aviation/airportlist.h"
#include "blackmisc/network/textmessagelist.h"
#include "blackmisc/pq/length.h"
#include "blackmisc/pq/time.h"
#include "blackmisc/identifier.h"
#include "blackmisc/pixmap.h"
#include "blackmisc/settingscache.h"
#include "blackmisc/worker.h"

#include <QObject>
#include <QPair>
#include <QString>
#include <QPointer>

// clazy:excludeall=const-signal-or-slot

namespace BlackMisc
{
    class CDBusServer;
    namespace Aviation { class CCallsign; }
    namespace Simulation { class CSimulatedAircraft; }
}

namespace BlackCore
{
    class CCoreFacade;
    class CPluginManagerSimulator;

    namespace Context
    {
        //! Network simulator concrete implementation
        class BLACKCORE_EXPORT CContextSimulator :
            public IContextSimulator,
            public BlackMisc::Simulation::CRemoteAircraftAware, // gain access to in memory remote aircraft data
            public BlackMisc::CIdentifiable
        {
            Q_OBJECT
            Q_CLASSINFO("D-Bus Interface", BLACKCORE_CONTEXTSIMULATOR_INTERFACENAME)
            friend class BlackCore::CCoreFacade;
            friend class IContextSimulator;

        public slots:
            // ----------------------------- context interface -----------------------------
            //! \publicsection
            //! @{
            virtual BlackMisc::Simulation::CSimulatorPluginInfo getSimulatorPluginInfo() const override;
            virtual BlackMisc::Simulation::CSimulatorPluginInfoList getAvailableSimulatorPlugins() const override;
            virtual bool startSimulatorPlugin(const BlackMisc::Simulation::CSimulatorPluginInfo &simulatorInfo) override;
            virtual void stopSimulatorPlugin(const BlackMisc::Simulation::CSimulatorPluginInfo &simulatorInfo) override;
            virtual int checkListeners() override;
            virtual int getSimulatorStatus() const override;
            virtual BlackMisc::Simulation::CSimulatorInternals getSimulatorInternals() const override;
            virtual BlackMisc::Aviation::CAirportList getAirportsInRange(bool recalculateDistance) const override;
            virtual BlackMisc::Simulation::CAircraftModelList getModelSet() const override;
            virtual BlackMisc::Simulation::CSimulatorInfo getModelSetLoaderSimulator() const override;
            virtual void setModelSetLoaderSimulator(const BlackMisc::Simulation::CSimulatorInfo &simulator) override;
            virtual BlackMisc::Simulation::CSimulatorInfo simulatorsWithInitializedModelSet() const override;
            virtual BlackMisc::CStatusMessageList verifyPrerequisites() const override;
            virtual QStringList getModelSetStrings() const override;
            virtual QStringList getModelSetCompleterStrings(bool sorted) const override;
            virtual bool isKnownModelInSet(const QString &modelString) const override;
            virtual int getModelSetCount() const override;
            virtual void disableModelsForMatching(const BlackMisc::Simulation::CAircraftModelList &removedModels, bool incremental) override;
            virtual BlackMisc::Simulation::CAircraftModelList getDisabledModelsForMatching() const override;
            virtual void restoreDisabledModels() override;
            virtual bool isValidationInProgress() const override;
            virtual bool triggerModelSetValidation(const BlackMisc::Simulation::CSimulatorInfo &simulator) override;
            virtual BlackMisc::Simulation::CAircraftModelList getModelSetModelsStartingWith(const QString &modelString) const override;
            virtual BlackMisc::PhysicalQuantities::CTime getTimeSynchronizationOffset() const override;
            virtual bool setTimeSynchronization(bool enable, const BlackMisc::PhysicalQuantities::CTime &offset) override;
            virtual bool isTimeSynchronized() const override;
            virtual BlackMisc::Simulation::CInterpolationAndRenderingSetupGlobal getInterpolationAndRenderingSetupGlobal() const override;
            virtual BlackMisc::Simulation::CInterpolationSetupList getInterpolationAndRenderingSetupsPerCallsign() const override;
            virtual BlackMisc::Simulation::CInterpolationAndRenderingSetupPerCallsign getInterpolationAndRenderingSetupPerCallsignOrDefault(const BlackMisc::Aviation::CCallsign &callsign) const override;
            virtual bool setInterpolationAndRenderingSetupsPerCallsign(const BlackMisc::Simulation::CInterpolationSetupList &setups, bool ignoreSameAsGlobal) override;
            virtual void setInterpolationAndRenderingSetupGlobal(const BlackMisc::Simulation::CInterpolationAndRenderingSetupGlobal &setup) override;
            virtual BlackMisc::CStatusMessageList getInterpolationMessages(const BlackMisc::Aviation::CCallsign &callsign) const override;
            virtual BlackMisc::CPixmap iconForModel(const QString &modelString) const override;
            virtual void highlightAircraft(const BlackMisc::Simulation::CSimulatedAircraft &aircraftToHighlight, bool enableHighlight, const BlackMisc::PhysicalQuantities::CTime &displayTime) override;
            virtual bool followAircraft(const BlackMisc::Aviation::CCallsign &callsign) override;
            virtual void recalculateAllAircraft() override;
            virtual bool resetToModelMatchingAircraft(const BlackMisc::Aviation::CCallsign &callsign) override;
            virtual void setWeatherActivated(bool activated) override;
            virtual void requestWeatherGrid(const BlackMisc::Weather::CWeatherGrid &weatherGrid, const BlackMisc::CIdentifier &identifier) override;
            virtual int doMatchingsAgain() override;
            virtual bool doMatchingAgain(const BlackMisc::Aviation::CCallsign &callsign) override;
            virtual BlackMisc::CStatusMessageList getMatchingMessages(const BlackMisc::Aviation::CCallsign &callsign) const override;
            virtual bool isMatchingMessagesEnabled() const override;
            virtual void enableMatchingMessages(bool enabled) override;
            virtual BlackMisc::Simulation::CMatchingStatistics getCurrentMatchingStatistics(bool missingOnly) const override;
            virtual void setMatchingSetup(const BlackMisc::Simulation::CAircraftMatcherSetup &setup) override;
            virtual BlackMisc::Simulation::CAircraftMatcherSetup getMatchingSetup() const override;
            virtual BlackMisc::CStatusMessageList copyFsxTerrainProbe(const BlackMisc::Simulation::CSimulatorInfo &simulator) override;
            //! @}

            //! \addtogroup swiftdotcommands
            //! @{
            //! <pre>
            //! .plugin           forwarded to plugin, see details there
            //! .driver .drv      forwarded to plugin (same as above)
            //! .ris show         show interpolation setup in console
            //! .ris debug on|off interpolation/rendering setup, debug messages
            //! .ris parts on|off interpolation/rendering setup, aircraft parts
            //! </pre>
            //! @}
            //! \copydoc IContextSimulator::parseCommandLine
            virtual bool parseCommandLine(const QString &commandLine, const BlackMisc::CIdentifier &originator) override;
            // ----------------------------- context interface -----------------------------

        public:
            //! Destructor
            virtual ~CContextSimulator() override;

            //! Gracefully shut down, e.g. for plugin unloading
            void gracefulShutdown();

            //! Access to simulator (i.e. the plugin)
            QPointer<ISimulator> simulator() const;

            //! Simulator available?
            bool hasSimulator() const { return this->simulator(); }

            //! Register dot commands
            static void registerHelp()
            {
                if (BlackMisc::CSimpleCommandParser::registered("BlackCore::CContextSimulator")) { return; }
                BlackMisc::CSimpleCommandParser::registerCommand({".ris", "rendering/interpolation setup (global setup)"});
                BlackMisc::CSimpleCommandParser::registerCommand({".ris show", "display rendering/interpolation setup on console (global setup)"});
                BlackMisc::CSimpleCommandParser::registerCommand({".ris debug on|off", "rendering/interpolation debug messages (global setup)"});
                BlackMisc::CSimpleCommandParser::registerCommand({".ris parts on|off", "aircraft parts (global setup)"});
            }

        protected:
            //! Constructor
            CContextSimulator(CCoreFacadeConfig::ContextMode, CCoreFacade *runtime);

            //! Register myself in DBus
            CContextSimulator *registerWithDBus(BlackMisc::CDBusServer *server);

        private:
            static constexpr int MatchingLogMaxModelSetSize  = 250; //!< default value for switching matching log on
            static constexpr int MaxModelAddedFailoverTrials = 3;   //!< if model cannot be added, try again max <n> times

            //  ------------ slots connected with network or other contexts ---------
            //! \ingroup crosscontextfunction
            //! @{

            //! Remote aircraft added and ready for model matching
            void xCtxAddedRemoteAircraftReadyForModelMatching(const BlackMisc::Simulation::CSimulatedAircraft &remoteAircraft);

            //! Remove remote aircraft
            void xCtxRemovedRemoteAircraft(const BlackMisc::Aviation::CCallsign &callsign);

            //! Changed remote aircraft model
            void xCtxChangedRemoteAircraftModel(const BlackMisc::Simulation::CSimulatedAircraft &aircraft, const BlackMisc::CIdentifier &originator);

            //! Enable / disable aircraft
            void xCtxChangedRemoteAircraftEnabled(const BlackMisc::Simulation::CSimulatedAircraft &aircraft);

            //! Network connection status
            void xCtxNetworkConnectionStatusChanged(INetwork::ConnectionStatus from, INetwork::ConnectionStatus to);

            //! Update simulator cockpit from context, because someone else has changed cockpit (e.g. GUI, 3rd party)
            void xCtxUpdateSimulatorCockpitFromContext(const BlackMisc::Simulation::CSimulatedAircraft &ownAircraft, const BlackMisc::CIdentifier &originator);

            //! Update simulator SELCAL from context, because someone else has changed cockpit (e.g. GUI, 3rd party)
            void xCtxUpdateSimulatorSelcalFromContext(const BlackMisc::Aviation::CSelcal &selcal, const BlackMisc::CIdentifier &originator);

            //! Raw data when a new aircraft was requested, used for statistics
            void xCtxNetworkRequestedNewAircraft(const BlackMisc::Aviation::CCallsign &callsign, const QString &aircraftIcao, const QString &airlineIcao, const QString &livery);

            //! Text message received
            void xCtxTextMessagesReceived(const BlackMisc::Network::CTextMessageList &textMessages);
            //! @}
            //  ------------ slots connected with network or other contexts ---------

            //! Handle new connection status of simulator
            void onSimulatorStatusChanged(ISimulator::SimulatorStatus status);

            //! Listener reports the simulator has started
            void onSimulatorStarted(const BlackMisc::Simulation::CSimulatorPluginInfo &info);

            //! Simulator model has been changed
            void onOwnSimulatorModelChanged(const BlackMisc::Simulation::CAircraftModel &model);

            //! Failed adding remote aircraft
            void onAddingRemoteAircraftFailed(const BlackMisc::Simulation::CSimulatedAircraft &remoteAircraft, bool disabled, bool requestFailover, const BlackMisc::CStatusMessage &message);

            //! Relay status message to simulator under consideration of settings
            void relayStatusMessageToSimulator(const BlackMisc::CStatusMessage &message);

            //! Handle a change in enabled simulators
            void changeEnabledSimulators();

            //! Reads list of enabled simulators, starts listeners
            void restoreSimulatorPlugins();

            //! Load plugin and connect
            bool loadSimulatorPlugin(const BlackMisc::Simulation::CSimulatorPluginInfo &simulatorPluginInfo);

            //! Unload plugin, if desired restart listeners
            void unloadSimulatorPlugin();

            //! Listen for single simulator
            bool listenForSimulator(const BlackMisc::Simulation::CSimulatorPluginInfo &simulatorInfo);

            //! Listen for all simulators
            void listenForAllSimulators();

            //! Call stop() on all loaded listeners
            void stopSimulatorListeners();

            //! Add to message list for matching
            void addMatchingMessages(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::CStatusMessageList &messages);

            //! Clear matching messages
            void clearMatchingMessages(const BlackMisc::Aviation::CCallsign &callsign);

            //! Reverse lookup of a model
            BlackMisc::Simulation::CAircraftModel reverseLookupModel(const BlackMisc::Simulation::CAircraftModel &model);

            //! Load the last know model set
            void initByLastUsedModelSet();

            QPair<BlackMisc::Simulation::CSimulatorPluginInfo, QPointer<ISimulator>> m_simulatorPlugin; //!< Currently loaded simulator plugin
            QMap<BlackMisc::Aviation::CCallsign, BlackMisc::CStatusMessageList> m_matchingMessages;     //!< all matching log messages per callsign
            QMap<BlackMisc::Aviation::CCallsign, int> m_failoverAddingCounts;
            CPluginManagerSimulator  *m_plugins = nullptr; //!< plugin manager
            BlackMisc::CRegularThread m_listenersThread;   //!< waiting for plugin
            CWeatherManager  m_weatherManager  { this };   //!< weather management
            CAircraftMatcher m_aircraftMatcher { this };   //!< model matcher

            bool m_wasSimulating          = false;
            bool m_initallyAddAircraft    = false;
            bool m_enableMatchingMessages = true;
            bool m_isWeatherActivated     = false;

            QString m_networkSessionId; //!< Network session of CServer::getServerSessionId, if not connected empty (for statistics, ..)
            BlackMisc::Simulation::CBackgroundValidation *m_validator = nullptr;

            // settings
            BlackMisc::CSettingReadOnly<Application::TEnabledSimulators>           m_enabledSimulators { this, &CContextSimulator::changeEnabledSimulators };
            BlackMisc::CSetting<BlackMisc::Simulation::Settings::TModelMatching>   m_matchingSettings  { this }; //!< settings
            BlackMisc::CData<BlackMisc::Simulation::Data::TSimulatorLastSelection> m_modelSetSimulator { this }; //!< current simulator (used with radio buttons)
            BlackMisc::Simulation::Settings::CMultiSimulatorSettings               m_simulatorSettings { this }; //!< for directories of XPlane
            BlackMisc::CSetting<BlackMisc::Simulation::Settings::TInterpolationAndRenderingSetupGlobal> m_renderSettings  { this }; //!< rendering/interpolation settings
            BlackMisc::CSettingReadOnly<BlackMisc::Simulation::Settings::TSimulatorMessages>            m_messageSettings { this }; //!< settings for messages
        };
    } // namespace
} // namespace

#endif // guard
