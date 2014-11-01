/*******************************************************************************

  WVMessenger provides basic interface for sending messages
  clients and servers built on top of it inherit mesaging capabilities
  
  Messages are sent by:
    sendMessage( const QString &msg, const QVariant &data );
  And received through a signal:
    message( const QString &msg, const QVariant &data );

  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
    12/30/2006 18:55 - Queues for messages
      
  ver: 2
        
*******************************************************************************/

#ifndef WV_MESSENGER_H
#define WV_MESSENGER_H

#include <QObject>
#include <QTcpSocket>
#include <QList>
#include <QTimer>
#include <QVariant>
#include <QString>

//------------------------------------------------------------------------------
// WVMessageBase
//------------------------------------------------------------------------------

class WVMessageBase  {

public:
  WVMessageBase( const QString &_msg, const QVariant &_data );
  WVMessageBase( const WVMessageBase & );

  QString msg;
  QVariant data;
};

//------------------------------------------------------------------------------
// WVMessenger
//------------------------------------------------------------------------------

class WVMessenger : public QObject {
  Q_OBJECT

public:
  WVMessenger(); 
  ~WVMessenger();
  virtual void disconnect();
  QTcpSocket *socket() { return tcpSocket; }

  void setQueueReception( bool q ) { enqueue_reception = q; }
  void setQueueTransmition( bool q ) { enqueue_transmition = q; }

  void processMessages();

signals:
  void connected( QTcpSocket *socket );
  void disconnected( QTcpSocket *socket );
  void error( QAbstractSocket::SocketError error, QTcpSocket *socket );
  void message( const QString &msg, const QVariant &data );

public slots:
  void sendMessage( const QString &msg, const QVariant &data );

  virtual void onConnected( );
  virtual void onDisconnected( );
  virtual void onError ( QAbstractSocket::SocketError socketError );
  virtual void onReadyRead( );
  virtual void onMessage( const QString &msg, const QVariant &data );

public:
  static void filterMessageList( QList<WVMessageBase> *message_list );

protected:
  QTcpSocket *tcpSocket;
  quint32 blockSize;
  bool    reading_message;

  bool enqueue_reception;
  bool enqueue_transmition;
  QList<WVMessageBase> message_out_list;
  QList<WVMessageBase> message_in_list;
  QTimer timer;
  int timer_interval;
  
  void initSocket();
  void sendMessage( const QString &msg, const QVariant &data, QTcpSocket *socket );

  void sendStreamMessage( const QString &msg, const QString &name, QIODevice *data, QTcpSocket *socket );

protected slots:
  void processEnqueuedMessages();

};

#endif // WV_MESSENGER_H
