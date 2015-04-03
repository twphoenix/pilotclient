/* Copyright (C) 2013
 * swift project Community / Contributors
 *
 * This file is part of swift project. It is subject to the license terms in the LICENSE file found in the top-level
 * directory of this distribution and at http://www.swift-project.org/license.html. No part of swift project,
 * including this file, may be copied, modified, propagated, or distributed except according to the terms
 * contained in the LICENSE file.
 */

//! \file

#ifndef BLACKSAMPLE_CLI_CLIENT_H
#define BLACKSAMPLE_CLI_CLIENT_H

#include "blackcore/network.h"
#include "blackmisc/simulation/ownaircraftprovider.h"
#include <QObject>
#include <QTextStream>
#include <QMap>
#include <functional>

class Client : public QObject, BlackMisc::Simulation::COwnAircraftProviderSupport
{
    Q_OBJECT

public:
    Client(QObject *parent = nullptr);

signals:
    void quit();

public slots:
    void command(QString line);

private: //commands
    void help(QTextStream &args);
    void echo(QTextStream &args);
    void exit(QTextStream &args);
    void presetServerCmd(QTextStream &args);
    void presetCallsignCmd(QTextStream &args);
    void presetIcaoCodesCmd(QTextStream &args);
    void presetLoginModeCmd(QTextStream &args);
    void initiateConnectionCmd(QTextStream &args);
    void terminateConnectionCmd(QTextStream &args);
    void sendPrivateTextMessageCmd(QTextStream &args);
    void sendRadioTextMessageCmd(QTextStream &args);
    void sendIpQueryCmd(QTextStream &args);
    void sendFreqQueryCmd(QTextStream &args);
    void sendServerQueryCmd(QTextStream &args);
    void sendAtcQueryCmd(QTextStream &args);
    void sendAtisQueryCmd(QTextStream &args);
    void sendFlightPlanCmd(QTextStream &args);
    void sendFlightPlanQueryCmd(QTextStream &args);
    void sendRealNameQueryCmd(QTextStream &args);
    void sendCapabilitiesQueryCmd(QTextStream &args);
    void sendIcaoCodesQueryCmd(QTextStream &args);
    void setOwnAircraftCmd(QTextStream &args);
    void setOwnAircraftPositionCmd(QTextStream &args);
    void setOwnAircraftSituationCmd(QTextStream &args);
    void setOwnAircraftCockpitCmd(QTextStream &args);
    void sendPingCmd(QTextStream &args);
    void sendMetarQueryCmd(QTextStream &args);
    void sendWeatherDataQueryCmd(QTextStream &args);
    void sendCustomPacketCmd(QTextStream &args);

signals: //to send to INetwork
    void presetServer(const BlackMisc::Network::CServer &server);
    void presetCallsign(const BlackMisc::Aviation::CCallsign &callsign);
    void presetRealName(const QString &name);
    void presetIcaoCodes(const BlackMisc::Aviation::CAircraftIcao &icao);
    void presetLoginMode(BlackCore::INetwork::LoginMode mode);
    void initiateConnection();
    void terminateConnection();
    void sendTextMessages(const BlackMisc::Network::CTextMessageList &textMessages);
    void sendRadioTextMessage(const QVector<BlackMisc::PhysicalQuantities::CFrequency> &freqs, const QString &msg);
    void sendIpQuery();
    void sendFreqQuery(const BlackMisc::Aviation::CCallsign &callsign);
    void sendServerQuery(const BlackMisc::Aviation::CCallsign &callsign);
    void sendAtcQuery(const BlackMisc::Aviation::CCallsign &callsign);
    void sendAtisQuery(const BlackMisc::Aviation::CCallsign &callsign);
    void sendFlightPlan(const BlackMisc::Aviation::CFlightPlan &fp);
    void sendFlightPlanQuery(const BlackMisc::Aviation::CCallsign &callsign);
    void sendRealNameQuery(const BlackMisc::Aviation::CCallsign &callsign);
    void sendCapabilitiesQuery(const BlackMisc::Aviation::CCallsign &callsign);
    void sendIcaoCodesQuery(const BlackMisc::Aviation::CCallsign &callsign);
    void setOwnAircraftCockpit(const BlackMisc::Aviation::CComSystem &com1, const BlackMisc::Aviation::CComSystem &com2,
                               const BlackMisc::Aviation::CTransponder &xpdr, const QString &originator);
    void sendPing(const BlackMisc::Aviation::CCallsign &callsign);
    void sendMetarQuery(const QString &airportICAO);
    void sendWeatherDataQuery(const QString &airportICAO);
    void sendCustomPacket(const BlackMisc::Aviation::CCallsign &callsign, const QString &packetId, const QStringList &data);

public slots: //to receive from INetwork
    void atcPositionUpdate(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::PhysicalQuantities::CFrequency &freq,
                           const BlackMisc::Geo::CCoordinateGeodetic &pos, const BlackMisc::PhysicalQuantities::CLength &range);
    void atcDisconnected(const BlackMisc::Aviation::CCallsign &callsign);
    void connectionStatusChanged(BlackCore::INetwork::ConnectionStatus oldStatus, BlackCore::INetwork::ConnectionStatus newStatus);
    void ipReplyReceived(const QString &ip);
    void freqReplyReceived(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::PhysicalQuantities::CFrequency &freq);
    void serverReplyReceived(const BlackMisc::Aviation::CCallsign &callsign, const QString &server);
    void atcReplyReceived(const BlackMisc::Aviation::CCallsign &callsign, bool isATC);
    void atisReplyReceived(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::Aviation::CInformationMessage &atis);
    void realNameReplyReceived(const BlackMisc::Aviation::CCallsign &callsign, const QString &realname);
    void capabilitiesReplyReceived(const BlackMisc::Aviation::CCallsign &callsign, quint32 flags);
    void kicked(const QString &msg);
    void metarReplyReceived(const QString &data);
    void flightPlanReplyReceived(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::Aviation::CFlightPlan &flightPlan);
    void pilotDisconnected(const BlackMisc::Aviation::CCallsign &callsign);
    void icaoCodesReplyReceived(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::Aviation::CAircraftIcao &icaoData);
    void pongReceived(const BlackMisc::Aviation::CCallsign &callsign, const BlackMisc::PhysicalQuantities::CTime &elapsedTime);
    void textMessagesReceived(const BlackMisc::Network::CTextMessageList &messages);
    void customPacketReceived(const BlackMisc::Aviation::CCallsign &callsign, const QString &packetId, const QStringList &data);

private:
    QMap<QString, std::function<void(QTextStream &)>> m_commands;
    BlackCore::INetwork *m_net;
};

#endif // guard
