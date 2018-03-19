/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKMISC_SIMULATION_INTERPOLATIONSETUPLIST_H
#define BLACKMISC_SIMULATION_INTERPOLATIONSETUPLIST_H

#include "interpolationrenderingsetup.h"
#include "blackmisc/aviation/callsignobjectlist.h"
#include "blackmisc/blackmiscexport.h"
#include "blackmisc/collection.h"
#include "blackmisc/sequence.h"
#include "blackmisc/variant.h"

namespace BlackMisc
{
    namespace Simulation
    {
        //! Value object encapsulating a list of distributors.
        class BLACKMISC_EXPORT CInterpolationSetupList :
            public CSequence<CInterpolationAndRenderingSetupPerCallsign>,
            public Aviation::ICallsignObjectList<CInterpolationAndRenderingSetupPerCallsign, CInterpolationSetupList>,
            public Mixin::MetaType<CInterpolationSetupList>
        {
        public:
            BLACKMISC_DECLARE_USING_MIXIN_METATYPE(CInterpolationSetupList)

            //! Default constructor.
            CInterpolationSetupList();

            //! Construct from a base class object.
            CInterpolationSetupList(const CSequence<CInterpolationAndRenderingSetupPerCallsign> &other);
        };
    } //namespace
} // namespace

Q_DECLARE_METATYPE(BlackMisc::Simulation::CInterpolationSetupList)
Q_DECLARE_METATYPE(BlackMisc::CCollection<BlackMisc::Simulation::CInterpolationAndRenderingSetupPerCallsign>)
Q_DECLARE_METATYPE(BlackMisc::CSequence<BlackMisc::Simulation::CInterpolationAndRenderingSetupPerCallsign>)

#endif //guard
