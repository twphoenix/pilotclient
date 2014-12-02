/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "simconnect_object.h"
#include "blackcore/interpolator_linear.h"

using namespace BlackCore;

namespace BlackSimPlugin
{
    namespace Fsx
    {
        CSimConnectObject::CSimConnectObject() :
            m_interpolator(new CInterpolatorLinear()),
            m_requestId(-1),
            m_objectId(-1)
        {
        }
    }
}
