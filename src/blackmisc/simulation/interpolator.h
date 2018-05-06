/* Copyright (C) 2014
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_SIMULATION_INTERPOLATOR_H
#define BLACKMISC_SIMULATION_INTERPOLATOR_H

#include "interpolationrenderingsetup.h"
#include "blackmisc/simulation/remoteaircraftprovider.h"
#include "blackmisc/simulation/interpolationsetupprovider.h"
#include "blackmisc/simulation/simulationenvironmentprovider.h"
#include "blackmisc/simulation/aircraftmodel.h"
#include "blackmisc/aviation/aircraftsituationchange.h"
#include "blackmisc/aviation/aircraftsituation.h"
#include "blackmisc/aviation/aircraftpartslist.h"
#include "blackmisc/aviation/callsign.h"
#include "blackmisc/logcategorylist.h"

#include <QObject>
#include <QString>
#include <QtGlobal>
#include <QTimer>

namespace BlackMisc
{
    namespace Simulation
    {
        class CInterpolationLogger;
        class CInterpolatorLinear;
        class CInterpolatorSpline;
        struct CInterpolationStatus;
        struct CPartsStatus;

        //! Interpolator, calculation inbetween positions
        template <typename Derived>
        class CInterpolator :
            protected CSimulationEnvironmentAware,
            protected CInterpolationSetupAware,
            protected CRemoteAircraftAware
        {
        public:
            //! Log categories
            const CLogCategoryList &getLogCategories();

            //! Current interpolated situation
            Aviation::CAircraftSituation getInterpolatedSituation(qint64 currentTimeSinceEpoc, const CInterpolationAndRenderingSetupPerCallsign &setup, CInterpolationStatus &status);

            //! Parts before given offset time
            Aviation::CAircraftParts getInterpolatedParts(qint64 currentTimeSinceEpoc, const CInterpolationAndRenderingSetupPerCallsign &setup, CPartsStatus &partsStatus, bool log = false) const;

            //! Interpolated parts, if not available guessed parts
            Aviation::CAircraftParts getInterpolatedOrGuessedParts(qint64 currentTimeSinceEpoc, const CInterpolationAndRenderingSetupPerCallsign &setup, CPartsStatus &partsStatus, bool log = false) const;

            //! Latest interpolation result
            const Aviation::CAircraftSituation &getLastInterpolatedSituation() const { return m_lastInterpolation; }

            //! Takes input between 0 and 1 and returns output between 0 and 1 smoothed with an S-shaped curve.
            //!
            //! Useful for making interpolation seem smoother, efficiently as it just uses simple arithmetic.
            //! \see https://en.wikipedia.org/wiki/Smoothstep
            //! \see http://sol.gfxile.net/interpolation/
            static double smootherStep(double x)
            {
                return x * x * x * (x * (x * 6.0 - 15.0) + 10.0);
            }

            //! Attach an observer to read the interpolator's state for debugging
            //! \remark parts logging has a \c bool \c log flag
            void attachLogger(CInterpolationLogger *logger) { m_logger = logger; }

            //! Is logger attached?
            bool hasAttachedLogger() const { return m_logger; }

            //! Get an interpolator info string (for debug info)
            QString getInterpolatorInfo() const;

            //! Reset last interpolation to null
            //! \remark mainly needed in UNIT tests
            void resetLastInterpolation();

            //! Clear all data
            //! \remark mainly needed in UNIT tests
            void clear();

            //! Init, or re-init the corressponding model
            //! \remark either by passing a model or using the provider
            void initCorrespondingModel(const CAircraftModel &model = {});

        protected:
            //! Constructor
            CInterpolator(const Aviation::CCallsign &callsign,
                          ISimulationEnvironmentProvider *simEnvProvider, IInterpolationSetupProvider *setupProvider, IRemoteAircraftProvider *p3,
                          CInterpolationLogger *logger);

            //! Center of gravity
            const PhysicalQuantities::CLength &getModelCG() const { return m_model.getCG(); }

            //! Center of gravity, fetched from provider in case needed
            PhysicalQuantities::CLength getAndFetchModelCG();

            //! Decides threshold when situation is considered on ground
            //! \sa BlackMisc::Aviation::CAircraftSituation::setOnGroundFromGroundFactorFromInterpolation
            static double groundInterpolationFactor();

            CAircraftModel m_model; //!< corresponding model
            const Aviation::CCallsign  m_callsign; //!< corresponding callsign
            Aviation::CAircraftSituation m_lastInterpolation { Aviation::CAircraftSituation::null() }; //!< latest interpolation
            Aviation::CAircraftSituationChange m_situationChange; //!< situations change
            PhysicalQuantities::CLength m_lastSceneryOffset = PhysicalQuantities::CLength::null();
            qint64 m_situationsLastModifiedUsed { -1 }; //!< based on situations last updated
            int m_interpolatedSituationsCounter = 0; //!< counter for each interpolated situations: statistics, every n-th interpolation ....

            //! Get situations and calculate change, also correct altitudes if applicable
            Aviation::CAircraftSituationList remoteAircraftSituationsAndChange(bool useSceneryOffset);

            //! Verify gnd flag, times, ... true means "OK"
            bool verifyInterpolationSituations(const Aviation::CAircraftSituation &oldest, const Aviation::CAircraftSituation &newer, const Aviation::CAircraftSituation &latest,
                                               const CInterpolationAndRenderingSetupPerCallsign &setup = CInterpolationAndRenderingSetupPerCallsign::null());

        private:
            CInterpolationLogger *m_logger = nullptr; //!< optional interpolation logger
            QTimer m_initTimer; //!< timer to init model, will be deleted when interpolator is deleted and cancel the call

            //! Log parts
            void logParts(qint64 timestamp, const Aviation::CAircraftParts &parts, int partsNo, bool empty, bool log) const;

            //! Deferred init
            void deferredInit();

            Derived *derived() { return static_cast<Derived *>(this); }
            const Derived *derived() const { return static_cast<const Derived *>(this); }
        };

        //! Status of interpolation
        struct BLACKMISC_EXPORT CInterpolationStatus
        {
        public:
            //! Did interpolation succeed?
            bool isInterpolated() const { return m_isInterpolated; }

            //! Set succeeded
            void setInterpolated(bool interpolated) { m_isInterpolated = interpolated; }

            //! Set situations count
            void setSituationsCount(int count) { m_situations = count; }

            //! Extra info
            void setExtraInfo(const QString &info);

            //! Set succeeded
            void setInterpolatedAndCheckSituation(bool succeeded, const Aviation::CAircraftSituation &situation);

            //! Is the corresponding position valid?
            bool hasValidSituation() const { return m_isValidSituation; }

            //! Valid interpolated situation
            bool hasValidInterpolatedSituation() const;

            //! Is that a valid position?
            void checkIfValidSituation(const Aviation::CAircraftSituation &situation);

            //! Reset to default values
            void reset();

            //! Info string
            QString toQString() const;

        private:
            bool m_isInterpolated = false;   //!< position is interpolated (means enough values, etc.)
            bool m_isValidSituation = false; //!< is valid situation
            int  m_situations = -1;          //!< number of situations used for interpolation
            QString m_extraInfo; //!< optional details
        };

        //! Status regarding parts
        struct BLACKMISC_EXPORT CPartsStatus
        {
        public:
            //! all OK
            bool allTrue() const;

            //! Supporting parts
            bool isSupportingParts() const { return m_supportsParts; }

            //! Set support flag
            void setSupportsParts(bool supports) { m_supportsParts = supports; }

            //! Reset to default values
            void reset();

            //! Ctor
            CPartsStatus() {}

            //! Ctor
            CPartsStatus(bool supportsParts) : m_supportsParts(supportsParts) {}

        private:
            bool m_supportsParts = false; //!< supports parts for given callsign
        };

        //! \cond PRIVATE
        extern template class BLACKMISC_EXPORT_DECLARE_TEMPLATE CInterpolator<CInterpolatorLinear>;
        extern template class BLACKMISC_EXPORT_DECLARE_TEMPLATE CInterpolator<CInterpolatorSpline>;
        //! \endcond
    } // namespace
} // namespace
#endif // guard
