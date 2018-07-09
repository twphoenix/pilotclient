/* Copyright (C) 2018
 * swift project Community / Contributors
 *
 * This file is part of swift Project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "copysettingsandcachescomponent.h"
#include "ui_copysettingsandcachescomponent.h"
#include "blackgui/guiapplication.h"
#include "blackgui/guiutility.h"
#include "blackmisc/cachesettingsutils.h"
#include "blackmisc/json.h"
#include "blackmisc/fileutils.h"
#include "blackmisc/icons.h"

#include <QRegularExpression>
#include <QFileInfo>
#include <QStringBuilder>

using namespace BlackMisc;
using namespace BlackMisc::Audio;
using namespace BlackMisc::Input;
using namespace BlackMisc::Network;
using namespace BlackMisc::Network::Data;
using namespace BlackMisc::Network::Settings;
using namespace BlackMisc::Simulation::Settings;
using namespace BlackCore::Audio;
using namespace BlackCore::Application;
using namespace BlackCore::Data;
using namespace BlackGui::Settings;

namespace BlackGui
{
    namespace Components
    {
        CCopySettingsAndCachesComponent::CCopySettingsAndCachesComponent(QWidget *parent) :
            QFrame(parent),
            ui(new Ui::CCopySettingsAndCachesComponent)
        {
            ui->setupUi(this);
            this->initAll();
            this->allCheckBoxesReadOnly();
            connect(ui->pb_SelectAll, &QPushButton::clicked, this, &CCopySettingsAndCachesComponent::selectAll);
            connect(ui->pb_DeselectAll, &QPushButton::clicked, this, &CCopySettingsAndCachesComponent::deselectAll);
            connect(ui->pb_Copy, &QPushButton::clicked, this, &CCopySettingsAndCachesComponent::copy);
            connect(ui->comp_OtherSwiftVersions, &COtherSwiftVersionsComponent::versionChanged, this, &CCopySettingsAndCachesComponent::onOtherVersionChanged);
        }

        CCopySettingsAndCachesComponent::~CCopySettingsAndCachesComponent()
        { }

        void CCopySettingsAndCachesComponent::onOtherVersionChanged(const CApplicationInfo &info)
        {
            readOnlyCheckbox(ui->cb_SettingsAudio, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsAudio.getFilename()));
            readOnlyCheckbox(ui->cb_SettingsAudioInputDevice, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsAudio.getFilename()));
            readOnlyCheckbox(ui->cb_SettingsAudioOutputDevice, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsAudioOutputDevice.getFilename()));

            readOnlyCheckbox(ui->cb_SettingsNetworkTrafficServers, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsTrafficNetworkServers.getFilename()));
            readOnlyCheckbox(ui->cb_CacheLastNetworkServer, !CCacheSettingsUtils::hasOtherVersionCacheFile(info, m_cacheLastNetworkServer.getFilename()));
            readOnlyCheckbox(ui->cb_CacheLastVatsimServer, !CCacheSettingsUtils::hasOtherVersionCacheFile(info, m_cacheLastVatsimServer.getFilename()));

            readOnlyCheckbox(ui->cb_SettingsGuiGeneral, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsGuiGeneral.getFilename()));
            readOnlyCheckbox(ui->cb_SettingsDockWidget, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsDockWidget.getFilename()));
            readOnlyCheckbox(ui->cb_SettingsViewUpdate, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsViewUpdate.getFilename()));

            readOnlyCheckbox(ui->cb_SettingsEnabledSimulators, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsEnabledSimulators.getFilename()));
            readOnlyCheckbox(ui->cb_SettingsSimulatorFSX, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsSimulatorFsx.getFilename()));
            readOnlyCheckbox(ui->cb_SettingsSimulatorP3D, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsSimulatorP3D.getFilename()));
            readOnlyCheckbox(ui->cb_SettingsSimulatorXPlane, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsSimulatorXPlane.getFilename()));

            readOnlyCheckbox(ui->cb_SettingsActionHotkeys, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsActionHotkeys.getFilename()));
            readOnlyCheckbox(ui->cb_SettingsTextMessages, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsTextMessage.getFilename()));
            readOnlyCheckbox(ui->cb_SettingsAtcStations, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsAtcStations.getFilename()));

            readOnlyCheckbox(ui->cb_SettingsConsolidation, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsConsolidation.getFilename()));
            readOnlyCheckbox(ui->cb_SettingsModel, !CCacheSettingsUtils::hasOtherVersionSettingsFile(info, m_settingsModel.getFilename()));
        }

        void CCopySettingsAndCachesComponent::initAll()
        {
            this->initAudio();
            this->initNetwork();
            this->initUi();
            this->initSimulator();
            this->initMisc();
            this->initModel();
        }

        void CCopySettingsAndCachesComponent::initAudio()
        {
            ui->cb_SettingsAudio->setText(checkBoxText(TSettings::humanReadable(), true));
            ui->cb_SettingsAudioInputDevice->setText(checkBoxText(TInputDevice::humanReadable(), true));
            ui->cb_SettingsAudioOutputDevice->setText(checkBoxText(TOutputDevice::humanReadable(), true));
        }

        void CCopySettingsAndCachesComponent::initNetwork()
        {
            ui->cb_SettingsNetworkTrafficServers->setText(checkBoxText(TTrafficServers::humanReadable(), true));
            ui->cb_CacheLastNetworkServer->setText(checkBoxText(TLastServer::humanReadable(), false));
            ui->cb_CacheLastVatsimServer->setText(checkBoxText(TVatsimLastServer::humanReadable(), false));
        }

        void CCopySettingsAndCachesComponent::initUi()
        {
            ui->cb_SettingsGuiGeneral->setText(checkBoxText(TGeneralGui::humanReadable(), true));
            ui->cb_SettingsDockWidget->setText(checkBoxText(TDockWidget::humanReadable(), true));
            ui->cb_SettingsViewUpdate->setText(checkBoxText(TViewUpdateSettings::humanReadable(), true));
        }

        void CCopySettingsAndCachesComponent::initSimulator()
        {
            ui->cb_SettingsEnabledSimulators->setText(checkBoxText(TEnabledSimulators::humanReadable(), true));
            ui->cb_SettingsSimulatorFSX->setText(checkBoxText(TSimulatorFsx::humanReadable(), true));
            ui->cb_SettingsSimulatorP3D->setText(checkBoxText(TSimulatorP3D::humanReadable(), true));
            ui->cb_SettingsSimulatorXPlane->setText(checkBoxText(TSimulatorXP::humanReadable(), true));
        }

        void CCopySettingsAndCachesComponent::initMisc()
        {
            ui->cb_SettingsActionHotkeys->setText(checkBoxText(TActionHotkeys::humanReadable(), true));
            ui->cb_SettingsAtcStations->setText(checkBoxText(TAtcStationsSettings::humanReadable(), true));
            ui->cb_SettingsTextMessages->setText(checkBoxText(TextMessageSettings::humanReadable(), true));
        }

        void CCopySettingsAndCachesComponent::initModel()
        {
            ui->cb_SettingsModel->setText(checkBoxText(TModel::humanReadable(), true));
            ui->cb_SettingsConsolidation->setText(checkBoxText(TBackgroundConsolidation::humanReadable(), true));
        }

        void CCopySettingsAndCachesComponent::copy()
        {
            const CApplicationInfo otherVersionInfo = ui->comp_OtherSwiftVersions->selectedOtherVersion();
            if (otherVersionInfo.isNull()) { return; }

            // ------- audio -------
            if (ui->cb_SettingsAudio->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsAudio.getFilename());
                if (!joStr.isEmpty())
                {
                    const Audio::CSettings audioSettings = Audio::CSettings::fromJson(joStr, true);
                    this->displayStatusMessage(m_settingsAudio.setAndSave(audioSettings), audioSettings.toQString(true));
                }
            }

            if (ui->cb_SettingsAudioInputDevice->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsAudioInputDevice.getFilename());
                const QString audioInputSettings = Json::firstJsonValueAsString(joStr);
                if (!audioInputSettings.isEmpty())
                {
                    this->displayStatusMessage(m_settingsAudioInputDevice.setAndSave(audioInputSettings), audioInputSettings);
                }
            }

            if (ui->cb_SettingsAudioOutputDevice->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsAudioOutputDevice.getFilename());
                const QString audioOutputSettings = Json::firstJsonValueAsString(joStr);
                if (!audioOutputSettings.isEmpty())
                {
                    this->displayStatusMessage(m_settingsAudioOutputDevice.setAndSave(audioOutputSettings), audioOutputSettings);
                }
            }

            // ------- network -------
            if (ui->cb_CacheLastNetworkServer->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionCacheFileContent(otherVersionInfo, m_cacheLastNetworkServer.getFilename());
                if (!joStr.isEmpty())
                {
                    const CServer server = CServer::fromJson(joStr, true);
                    this->displayStatusMessage(m_cacheLastNetworkServer.set(server), server.toQString(true));
                }
            }

            if (ui->cb_CacheLastVatsimServer->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionCacheFileContent(otherVersionInfo, m_cacheLastVatsimServer.getFilename());
                if (!joStr.isEmpty())
                {
                    const CServer server = CServer::fromJson(joStr, true);
                    this->displayStatusMessage(m_cacheLastVatsimServer.set(server), server.toQString(true));
                }
            }

            // ------- GUI -------
            if (ui->cb_SettingsGuiGeneral->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsGuiGeneral.getFilename());
                if (!joStr.isEmpty())
                {
                    const CGeneralGuiSettings guiGeneral = CGeneralGuiSettings::fromJson(joStr, true);
                    this->displayStatusMessage(m_settingsGuiGeneral.setAndSave(guiGeneral), guiGeneral.toQString(true));
                }
            }

            if (ui->cb_SettingsDockWidget->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsDockWidget.getFilename());
                if (!joStr.isEmpty())
                {
                    const CDockWidgetSettings dwSettings = CDockWidgetSettings::fromJson(joStr, true);
                    this->displayStatusMessage(m_settingsDockWidget.setAndSave(dwSettings), dwSettings.toQString(true));
                }
            }

            if (ui->cb_SettingsViewUpdate->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsViewUpdate.getFilename());
                if (!joStr.isEmpty())
                {
                    const CViewUpdateSettings viewUpdate = CViewUpdateSettings::fromJson(joStr, true);
                    this->displayStatusMessage(m_settingsViewUpdate.setAndSave(viewUpdate), viewUpdate.toQString(true));
                }
            }

            // ------- sims -------
            if (ui->cb_SettingsEnabledSimulators->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsEnabledSimulators.getFilename());
                if (!joStr.isEmpty())
                {
                    const QStringList enabledSims = Json::firstJsonValueAsStringList(joStr);
                    if (!enabledSims.isEmpty())
                    {
                        this->displayStatusMessage(m_settingsEnabledSimulators.setAndSave(enabledSims), enabledSims.join(", "));
                    }
                }
            }

            if (ui->cb_SettingsSimulatorFSX->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsSimulatorFsx.getFilename());
                if (!joStr.isEmpty())
                {
                    const CSimulatorSettings settings = CSimulatorSettings::fromJson(joStr, true);
                    this->displayStatusMessage(m_settingsSimulatorFsx.setAndSave(settings), settings.toQString(true));
                }
            }

            if (ui->cb_SettingsSimulatorP3D->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsSimulatorP3D.getFilename());
                if (!joStr.isEmpty())
                {
                    const CSimulatorSettings settings = CSimulatorSettings::fromJson(joStr, true);
                    this->displayStatusMessage(m_settingsSimulatorP3D.setAndSave(settings), settings.toQString(true));
                }
            }

            if (ui->cb_SettingsSimulatorXPlane->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsSimulatorXPlane.getFilename());
                if (!joStr.isEmpty())
                {
                    const CSimulatorSettings settings = CSimulatorSettings::fromJson(joStr, true);
                    this->displayStatusMessage(m_settingsSimulatorXPlane.setAndSave(settings), settings.toQString(true));
                }
            }

            // ------ model ------
            if (ui->cb_SettingsModel->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsModel.getFilename());
                if (!joStr.isEmpty())
                {
                    const CModelSettings settings = CModelSettings::fromJson(joStr, true);
                    this->displayStatusMessage(m_settingsModel.setAndSave(settings), settings.toQString(true));
                }
            }

            if (ui->cb_SettingsConsolidation->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsConsolidation.getFilename());
                if (!joStr.isEmpty())
                {
                    bool ok = false;
                    const int consolidation = Json::firstJsonValueAsInt(joStr, -1, &ok);
                    if (ok)
                    {
                        this->displayStatusMessage(m_settingsConsolidation.setAndSave(consolidation), QString::number(consolidation));
                    }
                }
            }

            // ------ misc -------
            if (ui->cb_SettingsActionHotkeys->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsActionHotkeys.getFilename());
                if (!joStr.isEmpty())
                {
                    const CActionHotkeyList hotkeys = CActionHotkeyList::fromJson(joStr, true);
                    this->displayStatusMessage(m_settingsActionHotkeys.setAndSave(hotkeys), hotkeys.toQString(true));
                }
            }

            if (ui->cb_SettingsAtcStations->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsAtcStations.getFilename());
                if (!joStr.isEmpty())
                {
                    const CAtcStationsSettings settings = CAtcStationsSettings::fromJson(joStr, true);
                    this->displayStatusMessage(m_settingsAtcStations.setAndSave(settings), settings.toQString(true));
                }
            }

            if (ui->cb_SettingsTextMessages->isChecked())
            {
                const QString joStr = CCacheSettingsUtils::otherVersionSettingsFileContent(otherVersionInfo, m_settingsTextMessage.getFilename());
                if (!joStr.isEmpty())
                {
                    const CTextMessageSettings settings = CTextMessageSettings::fromJson(joStr, true);
                    this->displayStatusMessage(m_settingsTextMessage.setAndSave(settings), settings.toQString(true));
                }
            }
        }

        void CCopySettingsAndCachesComponent::selectAll()
        {
            for (QCheckBox *cb : this->checkBoxes())
            {
                cb->setChecked(cb->isEnabled());
            }
        }

        void CCopySettingsAndCachesComponent::deselectAll()
        {
            for (QCheckBox *cb : this->checkBoxes())
            {
                cb->setChecked(false);
            }
        }

        QList<QCheckBox *> CCopySettingsAndCachesComponent::checkBoxes() const
        {
            return this->findChildren<QCheckBox *>();
        }

        void CCopySettingsAndCachesComponent::displayStatusMessage(const CStatusMessage &msg, const QString &value)
        {
            if (msg.isEmpty()) { return; }
            if (value.isEmpty())
            {
                ui->le_Status->setText(msg.getMessage());
            }
            else
            {
                ui->le_Status->setText(msg.getMessage() % QStringLiteral(" ") % value);
            }
            if (sGui) { sGui->processEventsToRefreshGui(); }
        }

        void CCopySettingsAndCachesComponent::allCheckBoxesReadOnly()
        {
            for (QCheckBox *cb : checkBoxes())
            {
                readOnlyCheckbox(cb, true);
            }
        }

        void CCopySettingsAndCachesComponent::readOnlyCheckbox(QCheckBox *cb, bool readOnly)
        {
            Q_ASSERT_X(cb, Q_FUNC_INFO, "need checkbox");
            CGuiUtility::checkBoxReadOnly(cb, readOnly);
            if (readOnly) { cb->setChecked(false); }
            cb->setEnabled(!readOnly);
            cb->setIcon(readOnly ? CIcons::cross16() : CIcons::tick16());
        }

        QString CCopySettingsAndCachesComponent::checkBoxText(const QString &text, bool setting)
        {
            static const QString s("%1 [setting]");
            static const QString c("%1 [cache]");
            return setting ? s.arg(text) : c.arg(text);
        }

        bool CCopySettingsAndCachesWizardPage::validatePage()
        {
            return true;
        }

    } // ns
} // ns