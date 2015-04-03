/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackmisc/simulation/simulatedaircraftlist.h"
#include "blackmisc/nwuser.h"
#include "blackmisc/predicates.h"
#include "blackmisc/propertyindexallclasses.h"

using namespace BlackMisc;
using namespace BlackMisc::Aviation;
using namespace BlackMisc::PhysicalQuantities;
using namespace BlackMisc::Network;

namespace BlackMisc
{
    namespace Simulation
    {

        CSimulatedAircraftList::CSimulatedAircraftList() { }

        CSimulatedAircraftList::CSimulatedAircraftList(const CSequence<CSimulatedAircraft> &other) :
            CSequence<CSimulatedAircraft>(other)
        { }

        void CSimulatedAircraftList::registerMetadata()
        {
            qRegisterMetaType<BlackMisc::CSequence<CSimulatedAircraft>>();
            qDBusRegisterMetaType<BlackMisc::CSequence<CSimulatedAircraft>>();
            qRegisterMetaType<BlackMisc::CCollection<CSimulatedAircraft>>();
            qDBusRegisterMetaType<BlackMisc::CCollection<CSimulatedAircraft>>();
            qRegisterMetaType<CSimulatedAircraftList>();
            qDBusRegisterMetaType<CSimulatedAircraftList>();
            registerMetaValueType<CSimulatedAircraftList>();
        }

        CUserList CSimulatedAircraftList::getPilots() const
        {
            return this->findBy(Predicates::MemberValid(&CSimulatedAircraft::getPilot)).transform(Predicates::MemberTransform(&CSimulatedAircraft::getPilot));
        }

        CCallsignList CSimulatedAircraftList::getCallsignsWithSyncronizedParts() const
        {
            CCallsignList csl;
            for (const CSimulatedAircraft &aircraft : (*this))
            {
                if (!aircraft.isPartsSynchronized()) { continue; }
                csl.push_back(aircraft.getCallsign());
            }
            return csl;
        }

        void CSimulatedAircraftList::markAllAsNotRendered()
        {
            for (CSimulatedAircraft &aircraft : (*this))
            {
                if (!aircraft.isRendered()) { continue; }
                aircraft.setRendered(false);
            }
        }

        int CSimulatedAircraftList::setRendered(const CCallsign &callsign, bool rendered)
        {
            int c = 0;
            for (CSimulatedAircraft &aircraft : (*this))
            {
                if (aircraft.getCallsign() != callsign) { continue; }
                aircraft.setRendered(rendered);
                c++;
            }
            return c;
        }

        int CSimulatedAircraftList::setAircraftParts(const CCallsign &callsign, const CAircraftParts &parts)
        {
            int c = 0;
            for (CSimulatedAircraft &aircraft : (*this))
            {
                if (aircraft.getCallsign() != callsign) { continue; }
                aircraft.setParts(parts);
                aircraft.setPartsSynchronized(true);
                c++;
            }
            return c;
        }

        bool CSimulatedAircraftList::isEnabled(const CCallsign &callsign) const
        {
            for (const CSimulatedAircraft &aircraft : (*this))
            {
                if (aircraft.getCallsign() != callsign) { continue; }
                return aircraft.isEnabled();
            }
            return false;
        }

        bool CSimulatedAircraftList::isRendered(const CCallsign &callsign) const
        {
            for (const CSimulatedAircraft &aircraft : (*this))
            {
                if (aircraft.getCallsign() != callsign) { continue; }
                return aircraft.isRendered();
            }
            return false;
        }

        CAircraftList CSimulatedAircraftList::toAircraftList() const
        {
            CAircraftList al;
            for (const CSimulatedAircraft &aircraft : (*this))
            {
                al.push_back(aircraft);
            }
            return al;
        }

    } // namespace
} // namespace
