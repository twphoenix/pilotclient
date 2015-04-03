/* Copyright (C) 2013
 * swift Project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "fsxsimulatorsetup.h"

#include "blackmisc/project.h"
#include "blackmisc/simulation/fscommon/fscommonutil.h"
#include "blackmisc/simulation/fsx/simconnectutilities.h"

#include <QSettings>

using namespace BlackMisc;
using namespace BlackMisc::Simulation::FsCommon;

namespace BlackMisc
{
    namespace Simulation
    {
        namespace Fsx
        {
            void CFsxSimulatorSetup::init()
            {
                CSimulatorSetup::init();
                this->m_setup.addValue(SetupSimConnectCfgFile, CSimConnectUtilities::getLocalSimConnectCfgFilename());

                if (CProject::isCompiledWithFsxSupport())
                {
                    // set FSX path
                    QString fsxPath = CFsCommonUtil::fsxDirFromRegistry();
                    if (!fsxPath.isEmpty()) this->m_setup.value(CSimulatorSetup::SetupSimPath, CVariant(fsxPath));
                }
            }
        } // namespace
    } // namespace
} // namespace
