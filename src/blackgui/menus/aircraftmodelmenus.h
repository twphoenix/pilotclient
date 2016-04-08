/* Copyright (C) 2016
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#ifndef BLACKGUI_MENUS_AIRCRAFTMODELMENUS_H
#define BLACKGUI_MENUS_AIRCRAFTMODELMENUS_H

#include "menudelegate.h"
#include "blackgui/views/aircraftmodelview.h"
#include "blackmisc/simulation/aircraftmodelinterfaces.h"
#include "blackmisc/simulation/fscommon/vpilotrulesreader.h"
#include <QMenu>
#include <QObject>

namespace BlackGui
{
    namespace Menus
    {
        //! Menu base for aircraft model view menus
        class IAircraftModelViewMenu : public IMenuDelegate
        {
        public:
            //! Constructor
            IAircraftModelViewMenu(BlackGui::Views::CAircraftModelView *modelView, bool separator = true) :
                IMenuDelegate(modelView, separator)
            {}

        protected:
            //! Model view
            BlackGui::Views::CAircraftModelView *modelView() const;

            //! Get aircraft models
            const BlackMisc::Simulation::CAircraftModelList &getAircraftModels() const;

            //! Selected aircraft models
            BlackMisc::Simulation::CAircraftModelList getSelectedAircraftModels() const;
        };

        //! Open the simulator file (e.g. aircraft.cfg) in the standard text editor
        class CShowSimulatorFileMenu : public IAircraftModelViewMenu
        {
            Q_OBJECT

        public:
            //! Constructor
            using IAircraftModelViewMenu::IAircraftModelViewMenu;

            //! \copydoc IMenuDelegate::customMenu
            virtual void customMenu(QMenu &menu) const override;

        private slots:
            //! Open simulator file
            void ps_showSimulatorFile() const;
        };

        //! Merge with DB data
        class CMergeWithDbDataMenu : public IAircraftModelViewMenu
        {
            Q_OBJECT

        public:
            //! Constructor
            using IAircraftModelViewMenu::IAircraftModelViewMenu;

            //! Constructor
            CMergeWithDbDataMenu(BlackGui::Views::CAircraftModelView *modelView, QObject *modelsTarget, bool separator = true);

            //! \copydoc IMenuDelegate::customMenu
            virtual void customMenu(QMenu &menu) const override;

        private slots:
            void ps_mergeData();
            void ps_mergeSelectedData();

        private:
            BlackMisc::Simulation::IModelsSetable *modelsTargetSetable() const;
            BlackMisc::Simulation::IModelsUpdatable *modelsTargetUpdatable() const;
            QObject *m_modelsTarget = nullptr; //!< optional target for setting/updating the models
        };
    } // ns
} // ns

#endif // guard
