/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKSIMPLUGIN_FSXCOMMON_FSXSETTINGSCOMPONENT_H
#define BLACKSIMPLUGIN_FSXCOMMON_FSXSETTINGSCOMPONENT_H

#include "blackmisc/simulation/simulatorinfo.h"
#include <QFrame>
#include <QScopedPointer>

namespace Ui { class CFsxSettingsComponent; }
namespace BlackSimPlugin
{
    namespace FsxCommon
    {
        class CSimulatorFsxCommon;

        //! FSX/P3D settings
        class CFsxSettingsComponent : public QFrame
        {
            Q_OBJECT

        public:
            //! Constructor
            explicit CFsxSettingsComponent(QWidget *parent = nullptr);

            //! Destructor
            virtual ~CFsxSettingsComponent();

            //! Simulator, P3D/FSX
            void setSimulator(const BlackMisc::Simulation::CSimulatorInfo &simulator) { m_simulator = simulator; }

        private:
            //! Checkbox change
            void onSimConnectTraceChanged();

            //! Access the concrete implementation
            CSimulatorFsxCommon *getFsxSimulator () const;

            BlackMisc::Simulation::CSimulatorInfo m_simulator { "FSX" };
            QScopedPointer<Ui::CFsxSettingsComponent> ui;
        };
    } // ns
} // ns

#endif // guard