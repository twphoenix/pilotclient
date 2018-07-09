/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKGUI_COMPONENTS_COPYSETTINGSANDCACHESCOMPONENT_H
#define BLACKGUI_COMPONENTS_COPYSETTINGSANDCACHESCOMPONENT_H

#include "blackgui/blackguiexport.h"
#include "blackgui/settings/guisettings.h"
#include "blackgui/settings/dockwidgetsettings.h"
#include "blackgui/settings/viewupdatesettings.h"
#include "blackgui/settings/textmessagesettings.h"
#include "blackgui/settings/atcstationssettings.h"
#include "blackcore/data/launchersetup.h"
#include "blackcore/data/vatsimsetup.h"
#include "blackcore/audio/audiosettings.h"
#include "blackcore/application/applicationsettings.h"
#include "blackcore/application/updatesettings.h"
#include "blackmisc/simulation/data/modelcaches.h"
#include "blackmisc/simulation/data/lastmodel.h"
#include "blackmisc/simulation/settings/simulatorsettings.h"
#include "blackmisc/simulation/settings/modelsettings.h"
#include "blackmisc/network/data/lastserver.h"
#include "blackmisc/network/settings/servers.h"
#include "blackmisc/applicationinfo.h"

#include <QWizardPage>
#include <QFrame>
#include <QScopedPointer>
#include <QCheckBox>
#include <QList>

namespace Ui { class CCopySettingsAndCachesComponent; }
namespace BlackGui
{
    namespace Components
    {
        /**
         * Copy settings and caches
         */
        class CCopySettingsAndCachesComponent : public QFrame
        {
            Q_OBJECT

        public:
            //! Ctor
            explicit CCopySettingsAndCachesComponent(QWidget *parent = nullptr);

            //! Dtor
            virtual ~CCopySettingsAndCachesComponent();

        private:
            //! Other version has been changed
            void onOtherVersionChanged(const BlackMisc::CApplicationInfo &info);

            //! Init UI
            void initAll();

            //! Audio init
            void initAudio();

            //! Network init
            void initNetwork();

            //! UI init
            void initUi();

            //! Simulator init
            void initSimulator();

            //! Misc. init
            void initMisc();

            //! Model init
            void initModel();

            //! Copy
            void copy();

            //! Select all
            void selectAll();

            //! Deselect all
            void deselectAll();

            //! All checkboxes
            QList<QCheckBox *> checkBoxes() const;

            //! Display status message
            void displayStatusMessage(const BlackMisc::CStatusMessage &msg, const QString &value);

            //! All check boxes read only
            void allCheckBoxesReadOnly();

            //! Enable checkbox
            static void readOnlyCheckbox(QCheckBox *cb, bool readOnly);

            //! Checkbox text
            static QString checkBoxText(const QString &text, bool setting);

            QScopedPointer<Ui::CCopySettingsAndCachesComponent> ui;

            BlackMisc::CSetting<BlackCore::Audio::TSettings> m_settingsAudio { this };
            BlackMisc::CSetting<BlackCore::Audio::TInputDevice>  m_settingsAudioInputDevice { this };
            BlackMisc::CSetting<BlackCore::Audio::TOutputDevice> m_settingsAudioOutputDevice { this };
            BlackMisc::CSetting<BlackMisc::Network::Settings::TTrafficServers> m_settingsTrafficNetworkServers { this };
            BlackMisc::CSetting<Settings::TGeneralGui> m_settingsGuiGeneral { this };
            BlackMisc::CSetting<Settings::TDockWidget> m_settingsDockWidget { this };
            BlackMisc::CSetting<Settings::TViewUpdateSettings> m_settingsViewUpdate { this };
            BlackMisc::CSetting<BlackCore::Application::TEnabledSimulators> m_settingsEnabledSimulators { this };
            BlackMisc::CSetting<BlackCore::Application::TActionHotkeys> m_settingsActionHotkeys { this };
            BlackMisc::CSetting<BlackMisc::Simulation::Settings::TSimulatorFsx> m_settingsSimulatorFsx { this }; //!< FSX settings
            BlackMisc::CSetting<BlackMisc::Simulation::Settings::TSimulatorFs9> m_settingsSimulatorFs9 { this }; //!< FS9 settings
            BlackMisc::CSetting<BlackMisc::Simulation::Settings::TSimulatorP3D> m_settingsSimulatorP3D { this }; //!< P3D settings
            BlackMisc::CSetting<BlackMisc::Simulation::Settings::TSimulatorXP>  m_settingsSimulatorXPlane  { this }; //!< XP settings
            BlackMisc::CSetting<Settings::TBackgroundConsolidation> m_settingsConsolidation { this }; //!< consolidation time
            BlackMisc::CSetting<BlackMisc::Simulation::Settings::TModel> m_settingsModel { this }; //!< model setting
            BlackMisc::CSetting<BlackGui::Settings::TAtcStationsSettings> m_settingsAtcStations { this };
            BlackMisc::CSetting<BlackGui::Settings::TextMessageSettings> m_settingsTextMessage { this };

            BlackMisc::CData<BlackMisc::Network::Data::TLastServer> m_cacheLastNetworkServer { this }; //!< recently used server (VATSIM, other)
            BlackMisc::CData<BlackCore::Data::TVatsimLastServer> m_cacheLastVatsimServer { this }; //!< recently used VATSIM server
            BlackMisc::CData<BlackMisc::Simulation::Data::TSimulatorLastSelection> m_cacheModelSetCurrentSimulator { this };
            BlackMisc::CData<BlackMisc::Simulation::Data::TModelCacheLastSelection> m_cacheModelsCurrentSimulator { this };
            BlackMisc::CData<BlackMisc::Simulation::Data::TLastModel> m_cacheLastAircraftModel { this }; //!< recently used aircraft model
            BlackMisc::CData<BlackCore::Data::TLauncherSetup> m_cacheLauncherSetup { this };
            BlackMisc::CData<BlackCore::Data::TVatsimSetup> m_cacheVatsimSetup { this };
        };

        /**
         * Wizard page for CCopySettingsAndCaches
         */
        class CCopySettingsAndCachesWizardPage : public QWizardPage
        {
        public:
            //! Constructors
            using QWizardPage::QWizardPage;

            //! Set config
            void setConfigComponent(CCopySettingsAndCachesComponent *config) { m_copyCachesAndSettings = config; }

            //! \copydoc QWizardPage::validatePage
            virtual bool validatePage() override;

        private:
            CCopySettingsAndCachesComponent *m_copyCachesAndSettings = nullptr;
        };
    } // ns
} // ns

#endif // guard