/* Copyright (C) 2013 VATSIM Community / authors
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "avairportlist.h"
#include "predicates.h"

using namespace BlackMisc::PhysicalQuantities;

namespace BlackMisc
{
    namespace Aviation
    {
        /*
         * Empty constructor
         */
        CAirportList::CAirportList() { }

        /*
         * Construct from base class object
         */
        CAirportList::CAirportList(const CSequence<CAirport> &other) :
            CSequence<CAirport>(other)
        { }

        /*
         * Register metadata
         */
        void CAirportList::registerMetadata()
        {
            qRegisterMetaType<BlackMisc::CSequence<CAirport>>();
            qDBusRegisterMetaType<BlackMisc::CSequence<CAirport>>();
            qRegisterMetaType<BlackMisc::CCollection<CAirport>>();
            qDBusRegisterMetaType<BlackMisc::CCollection<CAirport>>();
            qRegisterMetaType<CAirportList>();
            qDBusRegisterMetaType<CAirportList>();
        }

        /*
         * Find by ICAO
         */
        CAirportList CAirportList::findByIcao(const CAirportIcao &icao) const
        {
            return this->findBy(&CAirport::getIcao, icao);
        }

        void CAirportList::replaceOrAddByIcao(const CAirport &addedOrReplacedAirport)
        {
            if (!addedOrReplacedAirport.hasValidIcaoCode()) return; // ignore invalid airport
            this->replaceOrAdd(&CAirport::getIcao, addedOrReplacedAirport.getIcao(), addedOrReplacedAirport);
        }

        /*
         * Find first by ICAO
         */
        CAirport CAirportList::findFirstByIcao(const CAirportIcao &icao, const CAirport &ifNotFound) const
        {
            return this->findByIcao(icao).frontOrDefault(ifNotFound);
        }

        /*
         * Airports within range
         */
        CAirportList CAirportList::findWithinRange(const BlackMisc::Geo::ICoordinateGeodetic &coordinate, const PhysicalQuantities::CLength &range) const
        {
            return this->findBy([&](const CAirport & atcairport)
            {
                return greatCircleDistance(atcairport, coordinate) <= range;
            });
        }

        /*
         * Distances, bearing to own plane
         */
        void CAirportList::calculcateDistanceAndBearingToPlane(const Geo::CCoordinateGeodetic &position)
        {
            std::for_each(this->begin(), this->end(), [ & ](CAirport & airport)
            {
                airport.calculcateDistanceAndBearingToPlane(position);
            });
        }

        /*
         * Remove outside range
         */
        void CAirportList::removeIfOutsideRange(const Geo::CCoordinateGeodetic &position, const CLength &maxDistance, bool updateValues)
        {
            this->removeIf([ & ](CAirport &airport)
            {
                return airport.calculcateDistanceAndBearingToPlane(position, updateValues) > maxDistance;
            });
        }

        /*
         * Sort by range
         */
        void CAirportList::sortByRange(const BlackMisc::Geo::CCoordinateGeodetic &position, bool updateValues)
        {
            if (updateValues)
            {
                this->calculcateDistanceAndBearingToPlane(position);
            }
            this->sort([ & ](const CAirport &a, const CAirport &b) { return a.getDistanceToPlane() < b.getDistanceToPlane(); });
        }

    } // namespace
} // namespace
