/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "simconnectobject.h"

namespace BlackSimPlugin
{
    namespace Fsx
    {
        CSimConnectObject::CSimConnectObject() { }

        CSimConnectObject::CSimConnectObject(const BlackMisc::Aviation::CCallsign &callsign, int requestId, int objectId, bool vtol) :
            m_callsign(callsign), m_requestId(requestId), m_objectId(objectId), m_vtol(vtol)
        { }

    } // namespace
} // namespace
