/* Copyright (C) 2014
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackmisc/blackmiscfreefunctions.h"
#include "blackmisc/aviation/aviation.h"

/*
 * Metadata for aviation
 *
 * In a separate file to workaround a limitation of MinGW:
 * http://stackoverflow.com/q/16596876/1639256
 */
void BlackMisc::Aviation::registerMetadata()
{
    CComSystem::registerMetadata();
    CNavSystem::registerMetadata();
    CAdfSystem::registerMetadata();
    CAltitude::registerMetadata();
    CTransponder::registerMetadata();
    CHeading::registerMetadata();
    CTrack::registerMetadata();
    CInformationMessage::registerMetadata();
    CCallsign::registerMetadata();
    CCallsignSet::registerMetadata();
    CAtcStation::registerMetadata();
    CAtcStationList::registerMetadata();
    CAirport::registerMetadata();
    CAirportList::registerMetadata();
    CAirportIcaoCode::registerMetadata();
    CAircraftSituation::registerMetadata();
    CAircraftSituationList::registerMetadata();
    CAircraftIcaoCode::registerMetadata();
    CAircraftIcaoCodeList::registerMetadata();
    CAirlineIcaoCode::registerMetadata();
    CAirlineIcaoCodeList::registerMetadata();
    CSelcal::registerMetadata();
    CFlightPlan::registerMetadata();
    CAircraftEngine::registerMetadata();
    CAircraftEngineList::registerMetadata();
    CAircraftLights::registerMetadata();
    CAircraftParts::registerMetadata();
    CAircraftPartsList::registerMetadata();
    CLivery::registerMetadata();
    CLiveryList::registerMetadata();
}
