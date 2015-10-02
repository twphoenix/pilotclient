/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "dbmodelcomponent.h"
#include "blackgui/stylesheetutility.h"
#include "ui_dbmodelcomponent.h"
#include <functional>

using namespace BlackMisc;
using namespace BlackMisc::Network;
using namespace BlackGui::Views;
using namespace BlackGui::Models;

namespace BlackGui
{
    namespace Components
    {
        CDbModelComponent::CDbModelComponent(QWidget *parent) :
            QFrame(parent),
            ui(new Ui::CDbModelComponent)
        {
            ui->setupUi(this);
            this->setViewWithIndicator(this->ui->tvp_AircraftModel);
            this->ui->tvp_AircraftModel->setAircraftModelMode(CAircraftModelListModel::Database);
            connect(this->ui->tvp_AircraftModel, &CAircraftModelView::requestNewBackendData, this, &CDbModelComponent::ps_reload);
            connect(&CStyleSheetUtility::instance(), &CStyleSheetUtility::styleSheetsChanged, this, &CDbModelComponent::ps_onStyleSheetChanged);

            // filter and drag and drop
            this->ui->tvp_AircraftModel->setFilterWidget(this->ui->filter_AircraftModelFilter);
            this->ui->tvp_AircraftModel->allowDragDropValueObjects(true, false);
        }

        CDbModelComponent::~CDbModelComponent()
        {
            gracefulShutdown();
        }

        void CDbModelComponent::setProvider(IWebDataServicesProvider *webDataReaderProvider)
        {
            CWebDataServicesAware::setProvider(webDataReaderProvider);
            webDataReaderProvider->connectDataReadSignal(
                this,
                std::bind(&CDbModelComponent::ps_modelsRead, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
            );
            int c = getModelsCount();
            if (c > 0)
            {
                ps_modelsRead(CEntityFlags::ModelEntity, CEntityFlags::ReadFinished, c);
            }
        }

        void CDbModelComponent::ps_modelsRead(CEntityFlags::Entity entity, CEntityFlags::ReadState readState, int count)
        {
            Q_UNUSED(count);
            if (entity.testFlag(CEntityFlags::ModelEntity) && readState == CEntityFlags::ReadFinished)
            {
                this->ui->tvp_AircraftModel->updateContainer(this->getModels());
            }
        }

        void CDbModelComponent::ps_reload()
        {
            if (!hasProvider()) { return; }
            triggerRead(CEntityFlags::ModelEntity);
        }

        void CDbModelComponent::ps_onStyleSheetChanged()
        {
            // code goes here
        }

    } // ns
} // ns
