/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "simulator_common.h"
#include "blackcore/interpolator.h"
#include "blackcore/blackcorefreefunctions.h"
#include "blackmisc/logmessage.h"
#include "blackmisc/collection.h"

using namespace BlackMisc;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::Simulation;
using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Simulation;

namespace BlackCore
{
    CSimulatorCommon::CSimulatorCommon(const CSimulatorPluginInfo &info,
                                       BlackMisc::Simulation::IOwnAircraftProvider *ownAircraftProvider,
                                       BlackMisc::Simulation::IRemoteAircraftProvider *remoteAircraftProvider,
                                       QObject *parent)
        : ISimulator(parent),
          COwnAircraftAware(ownAircraftProvider),
          CRemoteAircraftAware(remoteAircraftProvider),
          m_simulatorPluginInfo(info)
    {
        this->setObjectName(info.getIdentifier());
        this->m_oneSecondTimer = new QTimer(this);
        this->m_oneSecondTimer->setObjectName(this->objectName().append(":m_oneSecondTimer"));
        connect(this->m_oneSecondTimer, &QTimer::timeout, this, &CSimulatorCommon::ps_oneSecondTimer);
        this->m_oneSecondTimer->start(1000);

        // provider signals
        bool c = remoteAircraftProvider->connectRemoteAircraftProviderSignals(
                     std::bind(&CSimulatorCommon::ps_remoteProviderAddAircraftSituation, this, std::placeholders::_1),
                     std::bind(&CSimulatorCommon::ps_remoteProviderAddAircraftParts, this, std::placeholders::_1),
                     std::bind(&CSimulatorCommon::ps_remoteProviderRemovedAircraft, this, std::placeholders::_1),
                     std::bind(static_cast<void(CSimulatorCommon::*)(const CAirspaceAircraftSnapshot &)>(&CSimulatorCommon::ps_recalculateRenderedAircraft), this, std::placeholders::_1)
                 );
        Q_ASSERT(c);
        Q_UNUSED(c);

        // info
        CLogMessage(this).info("Initialized simulator driver %1") << m_simulatorPluginInfo.toQString();
    }

    bool CSimulatorCommon::logicallyAddRemoteAircraft(const CSimulatedAircraft &remoteAircraft)
    {
        if (!remoteAircraft.isEnabled()) { return false; }

        // if not restriced, directly change
        if (!isRenderingRestricted()) { return this->physicallyAddRemoteAircraft(remoteAircraft); }

        // will be added with next snapshot
        return false;
    }

    bool CSimulatorCommon::logicallyRemoveRemoteAircraft(const CCallsign &callsign)
    {
        // if not restriced, directly change
        if (!isRenderingRestricted()) { return this->physicallyRemoveRemoteAircraft(callsign); }

        // will be added with next snapshot
        return false;
    }

    void CSimulatorCommon::blinkHighlightedAircraft()
    {
        if (m_highlightedAircraft.isEmpty() || m_highlightEndTimeMsEpoch < 1) { return; }
        m_blinkCycle = !m_blinkCycle;

        if (QDateTime::currentMSecsSinceEpoch() < m_highlightEndTimeMsEpoch)
        {
            // blink mode, toggle aircraft
            for (const CSimulatedAircraft &aircraft : m_highlightedAircraft)
            {
                if (m_blinkCycle)
                {
                    this->physicallyRemoveRemoteAircraft(aircraft.getCallsign());
                }
                else
                {
                    this->physicallyAddRemoteAircraft(aircraft);
                }
            }
        }
        else
        {
            // restore
            for (const CSimulatedAircraft &aircraft : m_highlightedAircraft)
            {
                // get the current state for this aircraft
                // it might has been removed in the mean time
                const CCallsign cs(aircraft.getCallsign());
                resetAircraftFromBacked(cs);
            }
            m_highlightedAircraft.clear();
            m_highlightEndTimeMsEpoch = 0;
        }
    }

    void CSimulatorCommon::resetAircraftFromBacked(const CCallsign &callsign)
    {
        CSimulatedAircraft aircraft(this->getAircraftInRangeForCallsign(callsign));
        bool enabled = aircraft.isEnabled();
        if (enabled)
        {
            // are we already visible?
            if (!isPhysicallyRenderedAircraft(callsign))
            {
                this->physicallyAddRemoteAircraft(aircraft);
            }
        }
        else
        {
            this->physicallyRemoveRemoteAircraft(callsign);
        }
    }

    void CSimulatorCommon::setInitialAircraftSituationAndParts(CSimulatedAircraft &aircraft) const
    {
        if (!this->m_interpolator) { return; }

        const CCallsign callsign(aircraft.getCallsign());
        if (this->remoteAircraftSituationsCount(callsign) < 1) { return; }

        // with an interpolator the interpolated situation is used
        // to avoid position jittering
        qint64 time = QDateTime::currentMSecsSinceEpoch();
        IInterpolator::InterpolationStatus interpolationStatus;
        CAircraftSituation as(m_interpolator->getInterpolatedSituation(callsign, time, aircraft.isVtol(), interpolationStatus));
        if (interpolationStatus.interpolationSucceeded) { aircraft.setSituation(as); }
    }

    int CSimulatorCommon::getMaxRenderedAircraft() const
    {
        return (m_maxRenderedAircraft <= MaxAircraftInfinite) ? m_maxRenderedAircraft : MaxAircraftInfinite;
    }

    void CSimulatorCommon::setMaxRenderedAircraft(int maxRenderedAircraft)
    {
        if (maxRenderedAircraft == m_maxRenderedAircraft) { return; }
        if (maxRenderedAircraft < 1)
        {
            m_maxRenderedAircraft = 0;
        }
        else if (maxRenderedAircraft >= MaxAircraftInfinite)
        {
            m_maxRenderedAircraft = MaxAircraftInfinite;
        }
        else
        {
            m_maxRenderedAircraft = maxRenderedAircraft;
        }

        bool r = isRenderingRestricted();
        emit renderRestrictionsChanged(r, getMaxRenderedAircraft(), getMaxRenderedDistance(), getRenderedDistanceBoundary());
    }

    void CSimulatorCommon::setMaxRenderedDistance(CLength &distance)
    {
        if (distance == m_maxRenderedDistance) { return; }
        if (distance.isNull() || distance > getRenderedDistanceBoundary() || distance.isNegativeWithEpsilonConsidered())
        {
            m_maxRenderedDistance = CLength(0.0, CLengthUnit::nullUnit());
        }
        else
        {
            Q_ASSERT(!distance.isNegativeWithEpsilonConsidered());
            m_maxRenderedDistance = distance;
        }

        bool r = isRenderingRestricted();
        emit renderRestrictionsChanged(r, getMaxRenderedAircraft(), getMaxRenderedDistance(), getRenderedDistanceBoundary());
    }

    CLength CSimulatorCommon::getMaxRenderedDistance() const
    {
        if (m_maxRenderedDistance.isNull()) { return getRenderedDistanceBoundary(); }
        return m_maxRenderedDistance;
    }

    const CSimulatorPluginInfo &CSimulatorCommon::getSimulatorPluginInfo() const
    {
        return m_simulatorPluginInfo;
    }

    const CSimulatorSetup &CSimulatorCommon::getSimulatorSetup() const
    {
        return m_simulatorSetup;
    }

    CLength CSimulatorCommon::getRenderedDistanceBoundary() const
    {
        return CLength(20.0, CLengthUnit::NM());
    }

    bool CSimulatorCommon::isMaxAircraftRestricted() const
    {
        return m_maxRenderedAircraft < MaxAircraftInfinite;
    }

    bool CSimulatorCommon::isMaxDistanceRestricted() const
    {
        return !m_maxRenderedDistance.isNull();
    }

    void CSimulatorCommon::enableDebugMessages(bool driverMessages, bool interpolatorMessages)
    {
        this->m_debugMessages = driverMessages;
        Q_UNUSED(interpolatorMessages);
    }

    int CSimulatorCommon::getInstalledModelsCount() const
    {
        return getInstalledModels().size();
    }

    void CSimulatorCommon::highlightAircraft(const BlackMisc::Simulation::CSimulatedAircraft &aircraftToHighlight, bool enableHighlight, const BlackMisc::PhysicalQuantities::CTime &displayTime)
    {
        CCallsign cs(aircraftToHighlight.getCallsign());
        this->m_highlightedAircraft.removeByCallsign(cs);
        if (enableHighlight)
        {
            qint64 deltaT = displayTime.valueRounded(CTimeUnit::ms(), 0);
            this->m_highlightEndTimeMsEpoch = QDateTime::currentMSecsSinceEpoch() + deltaT;
            this->m_highlightedAircraft.push_back(aircraftToHighlight);
        }
    }

    bool CSimulatorCommon::isRenderingEnabled() const
    {
        if (m_maxRenderedAircraft < 1)  { return false; }
        if (!isMaxDistanceRestricted()) { return true; }
        return m_maxRenderedDistance.isPositiveWithEpsilonConsidered();
    }

    bool CSimulatorCommon::isRenderingRestricted() const
    {
        return this->isMaxDistanceRestricted() || this->isMaxAircraftRestricted();
    }

    void CSimulatorCommon::deleteAllRenderingRestrictions()
    {
        this->m_maxRenderedDistance = CLength(0, CLengthUnit::nullUnit());
        this->m_maxRenderedAircraft = MaxAircraftInfinite;
        emit renderRestrictionsChanged(false, getMaxRenderedAircraft(), getMaxRenderedDistance(), getRenderedDistanceBoundary());
    }

    bool CSimulatorCommon::physicallyRemoveMultipleRemoteAircraft(const CCallsignSet &callsigns)
    {
        int removed = 0;
        for (const CCallsign &callsign : callsigns)
        {
            if (physicallyRemoveRemoteAircraft(callsign)) { removed++; }
        }
        return removed > 0;
    }

    void CSimulatorCommon::ps_oneSecondTimer()
    {
        blinkHighlightedAircraft();
    }

    void CSimulatorCommon::ps_recalculateRenderedAircraft()
    {
        this->ps_recalculateRenderedAircraft(getLatestAirspaceAircraftSnapshot());
    }

    void CSimulatorCommon::ps_recalculateRenderedAircraft(const CAirspaceAircraftSnapshot &snapshot)
    {
        Q_ASSERT(BlackCore::isCurrentThreadCreatingThread(this));

        if (!snapshot.isValidSnapshot()) { return;}

        // for unrestricted values all add/remove actions are directly linked
        // when changing back from restricted->unrestricted an one time update is required
        if (!snapshot.isRestricted() && !snapshot.isRestrictionChanged()) { return; }

        // we will handle snapshot
        emit airspaceSnapshotHandled();


        // restricted snapshot values
        if (!snapshot.isRenderingEnabled())
        {
            this->physicallyRemoveAllRemoteAircraft();
            return;
        }

        CCallsignSet callsignsInSimulator(physicallyRenderedAircraft());
        CCallsignSet callsignsToBeRemoved(callsignsInSimulator.difference(snapshot.getEnabledAircraftCallsignsByDistance()));
        CCallsignSet callsignsToBeAdded(snapshot.getEnabledAircraftCallsignsByDistance().difference(callsignsInSimulator));
        this->physicallyRemoveMultipleRemoteAircraft(callsignsToBeRemoved);

        if (!callsignsToBeAdded.isEmpty())
        {
            CSimulatedAircraftList aircraftToBeAdded(getAircraftInRange().findByCallsigns(callsignsToBeAdded)); // thread safe copy
            for (const CSimulatedAircraft &aircraft : aircraftToBeAdded)
            {
                Q_ASSERT_X(aircraft.isEnabled(), Q_FUNC_INFO, "Disabled aircraft detected as to be added");
                this->physicallyAddRemoteAircraft(aircraft);
            }
        }
    }

    void CSimulatorCommon::ps_remoteProviderAddAircraftSituation(const CAircraftSituation &situation)
    {
        Q_UNUSED(situation);
    }

    void CSimulatorCommon::ps_remoteProviderAddAircraftParts(const CAircraftParts &parts)
    {
        Q_UNUSED(parts);
    }

    void CSimulatorCommon::ps_remoteProviderRemovedAircraft(const CCallsign &callsign)
    {
        Q_UNUSED(callsign);
    }

} // namespace
