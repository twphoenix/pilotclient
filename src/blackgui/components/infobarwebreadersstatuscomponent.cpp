/* Copyright (C) 2015
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

#include "blackcore/webdataservices.h"
#include "blackgui/components/infobarwebreadersstatuscomponent.h"
#include "blackgui/guiapplication.h"
#include "blackmisc/verify.h"
#include "ui_infobarwebreadersstatuscomponent.h"

#include <QtGlobal>

using namespace BlackGui;
using namespace BlackMisc;
using namespace BlackCore;
using namespace BlackMisc::Network;

namespace BlackGui
{
    namespace Components
    {
        CInfoBarWebReadersStatusBase::CInfoBarWebReadersStatusBase(QWidget *parent) : QFrame(parent)
        { }

        void CInfoBarWebReadersStatusBase::init()
        {
            Q_ASSERT_X(sGui, Q_FUNC_INFO, "No sGui");
            this->initLeds();

            // connect timer
            m_timer.setInterval(30 * 1000);
            m_timer.start();
            m_timer.setObjectName("CInfoBarWebReadersStatusBase::CheckSwiftDbTimer");
            bool c = connect(&m_timer, &QTimer::timeout, this,  &CInfoBarWebReadersStatusBase::checkServerAndData);
            Q_ASSERT_X(c, Q_FUNC_INFO, "Failed connect");
            c = connect(sGui, &CGuiApplication::changedInternetAccessibility, this, &CInfoBarWebReadersStatusBase::accessibilityChanged);
            Q_ASSERT_X(c, Q_FUNC_INFO, "Failed connect");
            c = connect(sGui, &CGuiApplication::changedSwiftDbAccessibility, this, &CInfoBarWebReadersStatusBase::accessibilityChanged);
            Q_ASSERT_X(c, Q_FUNC_INFO, "Failed connect");

            if (sGui->hasWebDataServices())
            {
                c = connect(sGui->getWebDataServices(), &CWebDataServices::dataRead, this, &CInfoBarWebReadersStatusBase::dataRead);
                Q_ASSERT_X(c, Q_FUNC_INFO, "Failed connect");
            }

            Q_UNUSED(c);
        }

        CInfoBarWebReadersStatusBase::~CInfoBarWebReadersStatusBase()
        {
            m_timer.stop();
        }

        void CInfoBarWebReadersStatusBase::initLeds()
        {
            CLedWidget::LedShape shape = CLedWidget::Rounded;
            this->led_SwiftDb->setValues(CLedWidget::Yellow, CLedWidget::Black, shape, "DB online", "DB offline", 14);
            this->led_DataReady->setValues(CLedWidget::Yellow, CLedWidget::Black, shape, "all data ready", "data missing", 14);

            this->led_IcaoAircraft->setValues(CLedWidget::Yellow, CLedWidget::Black, CLedWidget::Red, shape, "reading", "idle", "failed", 14);
            this->led_IcaoAirline->setValues(CLedWidget::Yellow, CLedWidget::Black, CLedWidget::Red, shape, "reading", "idle", "failed", 14);
            this->led_Countries->setValues(CLedWidget::Yellow, CLedWidget::Black, CLedWidget::Red, shape, "reading", "idle", "failed", 14);
            this->led_Models->setValues(CLedWidget::Yellow, CLedWidget::Black, CLedWidget::Red, shape, "reading", "idle", "failed", 14);
            this->led_Liveries->setValues(CLedWidget::Yellow, CLedWidget::Black, CLedWidget::Red, shape, "reading", "idle", "failed", 14);
            this->led_Distributors->setValues(CLedWidget::Yellow, CLedWidget::Black, CLedWidget::Red, shape, "reading", "idle", "failed", 14);
        }

        void CInfoBarWebReadersStatusBase::dataRead(CEntityFlags::Entity entity, CEntityFlags::ReadState readState, int count)
        {
            Q_UNUSED(count);
            QList<CLedWidget *> leds = this->entityToLeds(entity);
            if (!leds.isEmpty()) { this->setLedReadStates(leds, readState); }
        }

        void CInfoBarWebReadersStatusBase::accessibilityChanged(bool accessible)
        {
            Q_UNUSED(accessible);
            this->checkServerAndData();
        }

        void CInfoBarWebReadersStatusBase::checkServerAndData()
        {
            const bool swift = sGui && sGui->isSwiftDbAccessible();
            this->led_SwiftDb->setOn(swift);

            const bool allData = hasAllData();
            this->led_DataReady->setOn(allData);
        }

        void CInfoBarWebReadersStatusBase::setLedReadStates(const QList<CLedWidget *> &leds, CEntityFlags::ReadState readState)
        {
            for (CLedWidget *led : leds)
            {
                setLedReadState(led, readState);
            }
        }

        void CInfoBarWebReadersStatusBase::setLedReadState(CLedWidget *led, CEntityFlags::ReadState readState)
        {
            Q_ASSERT_X(led, Q_FUNC_INFO, "no LED");
            const int blinkTime = 2.5 * 1000;
            switch (readState)
            {
            case CEntityFlags::ReadFinished:
            case CEntityFlags::ReadFinishedRestricted:
                led->setOn(true, blinkTime);
                break;
            case CEntityFlags::StartRead:
                led->setOn(true);
                break;
            case CEntityFlags::ReadFailed:
                led->setTriState(2 * blinkTime);
                break;
            default:
                BLACK_VERIFY_X(false, Q_FUNC_INFO, "read state not handled");
                break;
            }
        }

        QList<CLedWidget *> CInfoBarWebReadersStatusBase::entityToLeds(CEntityFlags::Entity entity) const
        {
            QList<CLedWidget *> leds;
            if (entity.testFlag(CEntityFlags::CountryEntity)) { leds << this->led_Countries; }
            if (entity.testFlag(CEntityFlags::DistributorEntity)) { leds << this->led_Distributors; }
            if (entity.testFlag(CEntityFlags::AircraftIcaoEntity)) { leds << this->led_IcaoAircraft; }
            if (entity.testFlag(CEntityFlags::AirlineIcaoEntity)) { leds << this->led_IcaoAirline; }
            if (entity.testFlag(CEntityFlags::LiveryEntity)) { leds << this->led_Liveries; }
            if (entity.testFlag(CEntityFlags::ModelEntity)) { leds << this->led_Models; }
            return leds;
        }

        bool CInfoBarWebReadersStatusBase::hasAllData() const
        {
            if (!sGui) { return false; }
            return sGui->getWebDataServices()->getAirlineIcaoCodesCount() > 0 &&
                   sGui->getWebDataServices()->getAircraftIcaoCodesCount() > 0 &&
                   sGui->getWebDataServices()->getDistributorsCount() > 0 &&
                   sGui->getWebDataServices()->getModelsCount() > 0 &&
                   sGui->getWebDataServices()->getLiveriesCount() > 0 &&
                   sGui->getWebDataServices()->getCountriesCount() > 0;
        }

        void CInfoBarWebReadersStatusBase::setLeds(CLedWidget *ledDb, CLedWidget *dataReady, CLedWidget *ledIcaoAircraft, CLedWidget *ledIcaoAirline, CLedWidget *ledCountries, CLedWidget *ledDistributors, CLedWidget *ledLiveries, CLedWidget *ledModels)
        {
            this->led_SwiftDb = ledDb;
            this->led_DataReady = dataReady;
            this->led_IcaoAircraft = ledIcaoAircraft;
            this->led_IcaoAirline = ledIcaoAirline;
            this->led_Countries = ledCountries;
            this->led_Distributors = ledDistributors;
            this->led_Liveries = ledLiveries;
            this->led_Models = ledModels;
        }

        CInfoBarWebReadersStatusComponent::CInfoBarWebReadersStatusComponent(QWidget *parent) :
            CInfoBarWebReadersStatusBase(parent), ui(new Ui::CInfoBarWebReadersStatusComponent)
        {
            ui->setupUi(this);
            this->setLeds(ui->led_SwiftDb, ui->led_DataReady, ui->led_IcaoAircraft, ui->led_IcaoAirline, ui->led_Countries, ui->led_Distributors, ui->led_Liveries, ui->led_Models);
            this->init();
        }

        CInfoBarWebReadersStatusComponent::~CInfoBarWebReadersStatusComponent()
        { }
    } // namespace
} // namespace
