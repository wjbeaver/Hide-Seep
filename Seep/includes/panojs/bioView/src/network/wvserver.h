/*******************************************************************************

  WVServer provides basic server interface for sending/receiving messages,
  it will maintain connection with only one client with whom it will
  be exchanging messages following protocol defined by WVMessenger  
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
        
*******************************************************************************/

#ifndef WV_SERVER_H
#define WV_SERVER_H

#include <QtNetwork>

#include "wvmessenger.h"

class QTcpServer;

class WVServer : public WVMessenger {
  Q_OBJECT

public:
  WVServer( );
  bool listen( const QHostAddress & address = QHostAddress::Any, quint16 port = 0 );
  void stop();
  bool isConnected();
  QTcpServer *server() { return tcpServer; }

public slots:
  void onDisconnected( );

protected slots:
  virtual void connectClient();

protected:
  QTcpServer *tcpServer;

};

#endif // WV_SERVER_H
