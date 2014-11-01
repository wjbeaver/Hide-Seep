/*******************************************************************************

  WVClient provides basic client interface for sending/receiving messages,
  it will be exchanging messages following protocol defined by WVMessenger  
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
        
*******************************************************************************/

#include <QtNetwork>

#include "wvclient.h"

WVClient::WVClient( ): WVMessenger() {
  tcpSocket = new QTcpSocket(this);
  initSocket();
}

void WVClient::connectToServer( const QString &hostName, int hostPort ) {
  tcpSocket->abort();
  blockSize = 0;
  tcpSocket->connectToHost( hostName, hostPort );
}



