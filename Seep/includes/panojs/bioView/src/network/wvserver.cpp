/*******************************************************************************

  WVServer provides basic server interface for sending/receiving messages,
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
        
*******************************************************************************/

#include <QtNetwork>

#include "wvserver.h"

WVServer::WVServer( ): WVMessenger() {
  tcpSocket = NULL;
  tcpServer = new QTcpServer(this);
  connect(tcpServer, SIGNAL(newConnection()), this, SLOT(connectClient()));
}

bool WVServer::listen( const QHostAddress & address, quint16 port ) {
  return tcpServer->listen(address, port);
}

void WVServer::stop() {
  tcpServer->close();
}

bool WVServer::isConnected() {
  return (tcpSocket != NULL);
}

void WVServer::connectClient() {
  if (tcpSocket != NULL) {
    QTcpSocket *newClient = tcpServer->nextPendingConnection();
    sendMessage( tr("Not allowed to connect more than one client!!!"), 0, newClient );
    newClient->disconnectFromHost();
    return;
  }

  tcpSocket = tcpServer->nextPendingConnection();
  initSocket();
  emit connected( tcpSocket );
}

void WVServer::onDisconnected() {
  WVMessenger::onDisconnected();
  
  if (tcpSocket != NULL) {
    tcpSocket->deleteLater();
    tcpSocket = NULL;
  }
}



