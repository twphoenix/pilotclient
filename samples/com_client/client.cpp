#include <blackmisc/debug.h>
#include "client.h"
#include <blackmisc/message.h>

using namespace BlackMisc;

Client::Client(QObject *parent) : QObject(parent), comclient(IContext::getInstance())
{
    connect(&comclient, &BlackMisc::CComClient::doError,
            this, &Client::onError);
    connect(&comclient, &BlackMisc::CComClient::doConnected,
            this, &Client::onClientConnected);

    QString address = "127.0.0.1";
    comclient.connectTo(address, 6809);
}

Client::~Client()
{
}

void Client::onError(QAbstractSocket::SocketError /* error */, QString message)
{
    bAppWarning << "Socket error!";
}

void Client::onClientConnected()
{
    QByteArray message_data;
    QDataStream out(&message_data, QIODevice::WriteOnly);

    IMessage* testmsg = new TestMessage();

    *testmsg >> out;

    comclient.sendMessage(testmsg->getID(), message_data);
    delete testmsg;
}
