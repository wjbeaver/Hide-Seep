/*******************************************************************************

  WVMessenger provides basic interface for sending messages

  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
    12/30/2006 18:55 - Queues for messages
      
  ver: 2
        
*******************************************************************************/

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include "wvmessenger.h"

//------------------------------------------------------------------------------
// WVMessenger
//------------------------------------------------------------------------------

WVMessenger::WVMessenger( ): QObject() {
  tcpSocket = NULL;
  blockSize = 0;
  timer_interval = 25;
  enqueue_reception = false;
  enqueue_transmition = false;
  
  timer.setInterval( timer_interval );
  connect(&timer, SIGNAL(timeout()), this, SLOT(processEnqueuedMessages()) );
}

WVMessenger::~WVMessenger() { 
  disconnect(); 
}

void WVMessenger::initSocket() {
  if (tcpSocket == NULL) return;

  connect(tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()) );
  connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(onDisconnected()) );
  connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)),
          this,      SLOT(onError(QAbstractSocket::SocketError)) );
  connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(onReadyRead()) );
}

void WVMessenger::onConnected() {
  emit connected( tcpSocket );
}

void WVMessenger::onDisconnected() {
  emit disconnected( tcpSocket );
}

void WVMessenger::onError ( QAbstractSocket::SocketError er ) {
  emit error( er, tcpSocket );
}

void WVMessenger::processMessages() {
  if (tcpSocket == NULL) return;
  bool v = tcpSocket->canReadLine();
  if (v && !reading_message) onReadyRead();
}

void WVMessenger::onReadyRead() {
  if (tcpSocket == NULL) return;
  if ( tcpSocket->state() != QAbstractSocket::ConnectedState ) return;

  QDataStream in(tcpSocket);
  //in.setVersion(QDataStream::Qt_4_2);
  in.setVersion( in.version() ); // set to the current Qt version instead

  if (blockSize == 0) {
    if (tcpSocket->bytesAvailable() < (int)sizeof(quint32)) return;
    in >> blockSize;
    reading_message = true;
  }
  if (tcpSocket->bytesAvailable() < blockSize) return;
  QString msgString;
  in >> msgString;

  QVariant msgData;
  in >> msgData;

  onMessage( msgString, msgData );
  reading_message = false;
  blockSize = 0;
  if (tcpSocket->bytesAvailable() > 0) onReadyRead();
}

void WVMessenger::onMessage( const QString &msg, const QVariant &data ) {
  emit message( msg, data );
}

void WVMessenger::disconnect() {
  if (tcpSocket == NULL) return;
  tcpSocket->abort();
}

void WVMessenger::sendMessage( const QString &msg, const QVariant &data, QTcpSocket *socket ) {
  if (socket == NULL) return;
  if ( socket->state() != QAbstractSocket::ConnectedState ) return;

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  //out.setVersion(QDataStream::Qt_4_2);
  out.setVersion( out.version() ); // set to the current Qt version
  out << (quint32) 0;
  out << msg;
  out << data;
  out.device()->seek(0);
  out << (quint32)(block.size() - sizeof(quint32));

  socket->write(block);
  socket->flush();
}

void WVMessenger::sendStreamMessage( const QString &msg, const QString &name, QIODevice *data, QTcpSocket *socket ) {
  if (socket == NULL) return;
  if ( socket->state() != QAbstractSocket::ConnectedState ) return;

  QString notifier("MESSAGE_TYPE_STREAM");

  int notifier_size = notifier.size();
  int message_size = msg.size() + name.size() + data->size();

  QDataStream out(socket);
  out.setVersion( out.version() ); // set to the current Qt version
  out << (quint32) notifier_size;
  out << notifier;
  out << (quint32) message_size;
  out << msg;
  out << name;
  out << data;
}

void WVMessenger::sendMessage( const QString &msg, const QVariant &data ) {
  if (!enqueue_transmition)
    sendMessage( msg, data, tcpSocket );
  else {
    message_out_list.append( WVMessageBase(msg, data) );
    timer.start();
  }
}

void WVMessenger::processEnqueuedMessages() {

  // filter messages for negating messages
  filterMessageList( &message_out_list );

  for (int i=0; i<message_out_list.size(); ++i )
    if (message_out_list[i].msg != "")
      sendMessage( message_out_list[i].msg, message_out_list[i].data, tcpSocket );
  message_out_list.clear();
}

void WVMessenger::filterMessageList( QList<WVMessageBase> *message_list ) {
  if (message_list == NULL) return;

  for (int i=message_list->size()-1; i>=0; --i ) {
    
    if (message_list->at(i).msg != "POSITION_CHANGED" ) continue;

    for (int j=i-1; j>=0; --j ) {
      if (message_list->at(j).msg == message_list->at(i).msg)
          (*message_list)[j].msg = "";
    } // for j
  } // for i
}

//------------------------------------------------------------------------------
// WVMessageBase
//------------------------------------------------------------------------------

WVMessageBase::WVMessageBase( const QString &_msg, const QVariant &_data ) {
  msg  = _msg;
  data = _data;
}

WVMessageBase::WVMessageBase( const WVMessageBase &v ) {
  msg  = v.msg;
  data = v.data;
}

