/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "interpolationlogdisplaydialog.h"
#include "ui_interpolationlogdisplaydialog.h"
#include "blackcore/simulatorcommon.h"

using namespace BlackCore;

namespace BlackGui
{
    namespace Components
    {
        CInterpolationLogDisplayDialog::CInterpolationLogDisplayDialog(CSimulatorCommon *simulatorCommon, CAirspaceMonitor *airspaceMonitor, QWidget *parent) :
            QDialog(parent),
            ui(new Ui::CInterpolationLogDisplayDialog)
        {
            ui->setupUi(this);
            this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);
            this->setSimulator(simulatorCommon);

            if (airspaceMonitor)
            {
                this->setAirspaceMonitor(airspaceMonitor);
            }
            else
            {
                ui->comp_InterpolationLogDisplay->linkWithAirspaceMonitor();
            }
        }

        CInterpolationLogDisplayDialog::~CInterpolationLogDisplayDialog()
        {
            //! \todo KB 2018-05 this is a hack, I have no idea why I need to invalidate the parent. But without doing it, the dtor of comp_InterpolationLogDisplay is called tywice
            ui->comp_InterpolationLogDisplay->setParent(nullptr);
        }

        void CInterpolationLogDisplayDialog::setSimulator(CSimulatorCommon *simulatorCommon)
        {
            ui->comp_InterpolationLogDisplay->setSimulator(simulatorCommon);
        }

        void CInterpolationLogDisplayDialog::setAirspaceMonitor(CAirspaceMonitor *airspaceMonitor)
        {
            ui->comp_InterpolationLogDisplay->setAirspaceMonitor(airspaceMonitor);
        }
    } // ns
} // ns