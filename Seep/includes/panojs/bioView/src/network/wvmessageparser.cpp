/*******************************************************************************

  WVMessageParser parses messages received by WVServer or WVClient and emits 
  meaningful signals, e.g. if position received emits positionChanged, etc.

  WVMessageParser connects to WVServer or WVClient by the slot onMessage
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
    2007-03-22 18:56 - new messages
    2007-10-08 13:15 - new messages: smooth zoom, dyn enhance, rotations
    2007-10-10 14:03 - new messages: zoom in/out at
      
  ver: 9
        
*******************************************************************************/

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include "wvmessageparser.h"


//------------------------------------------------------------------------------
// Message classes
//------------------------------------------------------------------------------

class WVMessage_Warning : public WVMessage {
public:
  WVMessage_Warning( WVMessageParser *_parent ): WVMessage(_parent) {
    msg = WV_MSG_WARNING;
  }
  
  void sendValue( const QVariant &data ) const {
    if (!blocked_send)
      emit parent->warningMessage( data.toString() );
  }
};


//------------------------------------------------------------------------------
// WVMessageParser
//------------------------------------------------------------------------------

WVMessageParser::WVMessageParser( const WVMessenger *messenger ) {
  init(messenger );
}

void WVMessageParser::init( const WVMessenger *messenger ) {
  if ( messenger == NULL ) return;
  block_sending_all = false;
  block_receiving_all = false;

  connect( messenger, SIGNAL( message(const QString &, const QVariant &) ),
           this,      SLOT( onRawMessage(const QString &, const QVariant &) ) );

  connect( this,      SIGNAL( rawMessage(const QString &, const QVariant &) ),
           messenger, SLOT( sendMessage(const QString &, const QVariant &) ) );


  addMessage( WVMessage_Warning( this ) );

}

void WVMessageParser::blockSending( bool block ) {
  block_sending_all = block;
}

void WVMessageParser::blockReceiving( bool block ) {
  block_receiving_all = block;
}

void WVMessageParser::addMessage( const WVMessage &msg ) {
  messages.insert( msg.msg, msg );
}

//------------------------------------------------------------------------------
// reception
//------------------------------------------------------------------------------

void WVMessageParser::onRawMessage( const QString &msg, const QVariant &data ) {

  if (block_receiving_all) return;

  if (msg == WV_MSG_WARNING) {
    emit this->warningMessage( data.toString() );
    return;
  } else
  if (msg == WV_MSG_ERROR) {
    emit errorMessage( data.toString() );
    return;
  } else
  if (msg == WV_MSG_POSITION_CHANGED) {
    QPointF p = data.value<QPointF>();
    emit positionChanged( p );
    return;
  } else
  if (msg == WV_MSG_VISIBLE_AREA_CHANGED) {
    QRectF v = data.value<QRectF>();
    emit visibleAreaChanged(v);
    return;
  } else
  if (msg == WV_MSG_SCALE_CHANGED) {
    emit scaleChanged( data.toDouble() );
    return;
  } else
  if (msg == WV_MSG_RED_CHANGED) {
    emit redChanged( data.value<qint32>() );
    return;
  } else
  if (msg == WV_MSG_GREEN_CHANGED) {
    emit greenChanged( data.value<qint32>() );
    return;
  } else
  if (msg == WV_MSG_BLUE_CHANGED) {
    emit blueChanged( data.value<qint32>() );
    return;
  } else
  if (msg == WV_MSG_YELLOW_CHANGED) {
    emit yellowChanged( data.value<qint32>() );
    return;
  } else
  if (msg == WV_MSG_MAGENTA_CHANGED) {
    emit magentaChanged( data.value<qint32>() );
    return;
  } else
  if (msg == WV_MSG_CYAN_CHANGED) {
    emit cyanChanged( data.value<qint32>() );
    return;
  } else
  if (msg == WV_MSG_VIEW_CHAN_CHANGED) {
    emit viewChanChanged( data.value<QColor>() );
    return;
  } else
  if (msg == WV_MSG_VIEW_CHANNELS_CHANGED) {
    emit viewChannelsChanged( data.value< QList<QVariant> >() );
    return;
  } else
  if (msg == WV_MSG_ENHANCEMENT_CHANGED) {
    emit enhancementChanged( data.value<qint32>() );
    return;
  } else
  if (msg == WV_MSG_PAGE_CHANGED) {
    emit pageChanged( data.value<qint32>() );
    return;
  } else
  if (msg == WV_MSG_NUM_PAGES_CHANGED) {
    emit numPagesChanged( data.value<qint32>() );
    return;
  } else
  if (msg == WV_MSG_IMAGE_SIZE) {
    emit imageSizeChanged( data.value<QSize>() );
    return;
  } else
  if (msg == WV_MSG_METADATA_CHANGED) {
    emit metadataChanged( data.toString() );
    return;
  } else
  if (msg == WV_MSG_THUMBNAIL_CHANGED) {
    emit thumbnailChanged( data.value<QPixmap>() );
    return;
  } else
  if (msg == WV_MSG_THUMB_SIZE_CHANGED) {
    emit thumbSizeChanged( data.value<QSize>() );
    return;
  } else
  if (msg == WV_MSG_CHAN_LIST_CHANGED) {
    emit channelsListChanged( data.value<QStringList>() );
    return;
  } else
  if (msg == WV_MSG_ENHS_LIST_CHANGED) {
    emit enhancementListChanged( data.value<QStringList>() );
    return;
  } else
  if (msg == WV_MSG_LOCAL_FILE_NAME) {
    emit localFileName( data.toString() );
    return;
  } else
  if (msg == WV_MSG_OPERATION_START) {
    emit operationStart( data.toString() );
    return;
  } else
  if (msg == WV_MSG_OPERATION_END) {
    emit operationEnd( );
    return;
  } else
  if (msg == WV_MSG_OPERATION_PROGRESS) {
    emit operationProgress( data.value<qint32>() );
    return;
  } else
  if (msg == WV_MSG_PHYS_PIXEL_SIZE_X) {
    emit physPixelSizeX( data.toDouble() );
    return;
  } else
  if (msg == WV_MSG_PHYS_PIXEL_SIZE_UNIT) {
    emit physPixelSizeUnit( data.toString() );
    return;
  } else
  if (msg == WV_MSG_IMAGE_INFO_TEXT) {
    emit imageInfoText( data.toString() );
    return;
  } else
  if (msg == WV_MSG_FULL_VIRTUAL_SCR) {
    emit fullVirtualScreen( );
    return;
  } else
  if (msg == WV_MSG_CURSOR_POSITION) {
    emit cursorPositionChanged( data.value<QPointF>() );
    return;
  } else
  if (msg == WV_MSG_CLOSE_WALL) {
    emit closeWall( );
    return;
  } else
  if (msg == WV_MSG_SHOW_SCALEBAR) {
    emit showScaleBarChanged( data.toBool() );
    return;
  } else
  if (msg == WV_MSG_SHOW_METADATA) {
    emit showMetadataChanged( data.toBool() );
    return;
  } else
  if (msg == WV_MSG_TRIG_SMOOTH_ZOOM) {
    emit trigSmoothZoom( );
    return;
  } else
  if (msg == WV_MSG_SHOW_SMOOTH_ZOOM) {
    emit showSmoothZoom( data.toBool() );
    return;
  } else
  if (msg == WV_MSG_TRIG_DYN_ENHANCE) {
    emit trigDynEnhance( );
    return;
  } else
  if (msg == WV_MSG_SHOW_DYN_ENHANCE) {
    emit showDynEnhance( data.toBool() );
    return;
  } else
  if (msg == WV_MSG_TRIG_ROT_RIGHT) {
    emit rotateRight( );
    return;
  } else
  if (msg == WV_MSG_TRIG_ROT_LEFT) {
    emit rotateLeft( );
    return;
  } else
  if (msg == WV_MSG_TRIG_ROT_180) {
    emit rotate180( );
    return;
  } else
  if (msg == WV_MSG_ZOOM_IN) {
    emit zoomIn( data.value<QPointF>() );
    return;
  } else
  if (msg == WV_MSG_ZOOM_OUT) {
    emit zoomOut( data.value<QPointF>() );
    return;
  } else
  if (msg == WV_MSG_TRIG_PROJECT_MAX) {
    emit projectMax( );
    return;
  } else
  if (msg == WV_MSG_TRIG_PROJECT_MIN) {
    emit projectMin( );
    return;
  } else
  if (msg == WV_MSG_TRIG_NEGATIVE) {
    emit negative( );
    return;
  } else
  if (msg == WV_MSG_BRIGHTNESS_CONTRAST) {
    emit brightnessContrast( data.value< QList<QVariant> >() );
    return;
  } else
  if (msg == WV_MSG_LEVELS) {
    emit levels( data.value< QList<QVariant> >() );
    return;
  } else
  if (msg == WV_MSG_IMAGE_FILE) {
    QString name;
    QByteArray blob;
    QDataStream in( data.toByteArray() );
    in >> name;
    in >> blob;
    emit fileBlob( name, blob );
    return;
  }

  emit genericMessage( msg, data );
}

//------------------------------------------------------------------------------
// submittion
//------------------------------------------------------------------------------

void WVMessageParser::sendGenericMessage ( const QString &msg, const QVariant &data ) {
  if (block_sending_all) return;
  emit rawMessage( msg, data );
}

void WVMessageParser::sendWarningMessage ( const QString &v ) {
  sendGenericMessage( WV_MSG_WARNING, v );
}

void WVMessageParser::sendErrorMessage ( const QString &v ) {
  sendGenericMessage( WV_MSG_ERROR, v );
}

void WVMessageParser::sendPositionChanged ( const QPointF &v ) {
  sendGenericMessage( WV_MSG_POSITION_CHANGED, v );
}

void WVMessageParser::sendScaleChanged ( const double &v ) {
  sendGenericMessage( WV_MSG_SCALE_CHANGED, v );
}

void WVMessageParser::sendRedChanged ( const qint32 &v ) {
  sendGenericMessage( WV_MSG_RED_CHANGED, v );
}

void WVMessageParser::sendGreenChanged ( const qint32 &v ) {
  sendGenericMessage( WV_MSG_GREEN_CHANGED, v );
}

void WVMessageParser::sendBlueChanged ( const qint32 &v ) {
  sendGenericMessage( WV_MSG_BLUE_CHANGED, v );
}

void WVMessageParser::sendYellowChanged ( const qint32 &v ) {
  sendGenericMessage( WV_MSG_YELLOW_CHANGED, v );
}

void WVMessageParser::sendMagentaChanged ( const qint32 &v ) {
  sendGenericMessage( WV_MSG_MAGENTA_CHANGED, v );
}

void WVMessageParser::sendCyanChanged ( const qint32 &v ) {
  sendGenericMessage( WV_MSG_CYAN_CHANGED, v );
}

void WVMessageParser::sendViewChanChanged( const QColor &v ) {
  sendGenericMessage( WV_MSG_VIEW_CHAN_CHANGED, v );
}

void WVMessageParser::sendViewChannelsChanged( const QList<QVariant> &v ) {
  sendGenericMessage( WV_MSG_VIEW_CHANNELS_CHANGED, v );
}

void WVMessageParser::sendEnhancementChanged ( const qint32 &v ) {
  sendGenericMessage( WV_MSG_ENHANCEMENT_CHANGED, v );
}

void WVMessageParser::sendPageChanged ( const qint32 &v ) {
  sendGenericMessage( WV_MSG_PAGE_CHANGED, v );
}

void WVMessageParser::sendNumPagesChanged ( const qint32 &v ) {
  sendGenericMessage( WV_MSG_NUM_PAGES_CHANGED, v );
}

void WVMessageParser::sendImageSizeChanged ( const QSize &v ) {
  sendGenericMessage( WV_MSG_IMAGE_SIZE, v );
}

void WVMessageParser::sendMetadataChanged ( const QString &v ) {
  sendGenericMessage( WV_MSG_METADATA_CHANGED, v );
}

void WVMessageParser::sendThumbnailChanged ( const QPixmap &v ) {
  sendGenericMessage( WV_MSG_THUMBNAIL_CHANGED, v );
}

void WVMessageParser::sendThumbSizeChanged ( const QSize &v ) {
  sendGenericMessage( WV_MSG_THUMB_SIZE_CHANGED, v );
}

void WVMessageParser::sendChannelsListChanged ( const QStringList &v ) {
  sendGenericMessage( WV_MSG_CHAN_LIST_CHANGED, v );
}

void WVMessageParser::sendEnhancementListChanged ( const QStringList &v ) {
  sendGenericMessage( WV_MSG_ENHS_LIST_CHANGED, v );
}

void WVMessageParser::sendLocalFileName( const QString &v ) {
  sendGenericMessage( WV_MSG_LOCAL_FILE_NAME, v );
}

void WVMessageParser::sendVisibleAreaChanged( const QRectF &v ) {
  sendGenericMessage( WV_MSG_VISIBLE_AREA_CHANGED, v );
}

void WVMessageParser::sendOperationStart( const QString &v ) {
  sendGenericMessage( WV_MSG_OPERATION_START, v );
}

void WVMessageParser::sendOperationEnd( ) {
  sendGenericMessage( WV_MSG_OPERATION_END, QVariant() );
}

void WVMessageParser::sendOperationProgress( const qint32 &v ) {
  sendGenericMessage( WV_MSG_OPERATION_PROGRESS, v );
}

void WVMessageParser::sendPhysPixelSizeX( const double &v ) {
  sendGenericMessage( WV_MSG_PHYS_PIXEL_SIZE_X, v );
}

void WVMessageParser::sendPhysPixelSizeUnit( const QString &v ) {
  sendGenericMessage( WV_MSG_PHYS_PIXEL_SIZE_UNIT, v );
}

void WVMessageParser::sendImageInfoText( const QString &v ) {
  sendGenericMessage( WV_MSG_IMAGE_INFO_TEXT, v );
}

void WVMessageParser::sendFullVirtualScreen( ) {
  sendGenericMessage( WV_MSG_FULL_VIRTUAL_SCR, QVariant() ); 
}

void WVMessageParser::sendCursorPosition( const QPointF &v ) {
  sendGenericMessage( WV_MSG_CURSOR_POSITION, v );
}

void WVMessageParser::sendCloseWall( ) {
  sendGenericMessage( WV_MSG_CLOSE_WALL, QVariant() ); 
}

void WVMessageParser::sendShowScaleBar( bool v ) {
  sendGenericMessage( WV_MSG_SHOW_SCALEBAR, v ); 
}

void WVMessageParser::sendShowMetadata( bool v ) {
  sendGenericMessage( WV_MSG_SHOW_METADATA, v ); 
}

void WVMessageParser::sendTrigSmoothZoom( ) {
  sendGenericMessage( WV_MSG_TRIG_SMOOTH_ZOOM, QVariant() ); 
}

void WVMessageParser::sendShowSmoothZoom( bool v ) {
  sendGenericMessage( WV_MSG_SHOW_SMOOTH_ZOOM, v ); 
}

void WVMessageParser::sendTrigDynEnhance( ) {
  sendGenericMessage( WV_MSG_TRIG_DYN_ENHANCE, QVariant() ); 
}

void WVMessageParser::sendShowDynEnhance( bool v ) {
  sendGenericMessage( WV_MSG_SHOW_DYN_ENHANCE, v ); 
}

void WVMessageParser::sendRotateRight( ) {
  sendGenericMessage( WV_MSG_TRIG_ROT_RIGHT, QVariant() ); 
}

void WVMessageParser::sendRotateLeft( ) {
  sendGenericMessage( WV_MSG_TRIG_ROT_LEFT, QVariant() ); 
}

void WVMessageParser::sendRotate180( ) {
  sendGenericMessage( WV_MSG_TRIG_ROT_180, QVariant() ); 
}

void WVMessageParser::sendZoomIn( const QPointF &v ) {
  sendGenericMessage( WV_MSG_ZOOM_IN, v ); 
}

void WVMessageParser::sendZoomOut( const QPointF &v ) {
  sendGenericMessage( WV_MSG_ZOOM_OUT, v ); 
}

void WVMessageParser::sendProjectMax( ) {
  sendGenericMessage( WV_MSG_TRIG_PROJECT_MAX, QVariant() ); 
}

void WVMessageParser::sendProjectMin( ) {
  sendGenericMessage( WV_MSG_TRIG_PROJECT_MIN, QVariant() ); 
}

void WVMessageParser::sendNegative( ) {
  sendGenericMessage( WV_MSG_TRIG_NEGATIVE, QVariant() ); 
}

void WVMessageParser::sendBrightnessContrast( const QList<QVariant> &v ) {
  sendGenericMessage( WV_MSG_BRIGHTNESS_CONTRAST, v ); 
}

void WVMessageParser::sendLevels( const QList<QVariant> &v ) {
  sendGenericMessage( WV_MSG_LEVELS, v ); 
}

void WVMessageParser::sendBlob( const QByteArray &blob ) {
  sendGenericMessage( WV_MSG_BLOB, blob ); 
}

// this function is only intended for small files!!!!!!!!!!!!!!!!!!!!!
void WVMessageParser::sendFile( const QString &name ) {
  QFileInfo fi(name);
  QFile f(name);
  f.open(QIODevice::ReadOnly);

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion( out.version() ); // set to the current Qt version
  out << fi.fileName();
  out << f.readAll();

  sendGenericMessage( WV_MSG_IMAGE_FILE, block ); 
}

//------------------------------------------------------------------------------
// WVMessage base
//------------------------------------------------------------------------------

WVMessage::WVMessage( WVMessageParser *_parent ) {
  parent = _parent;
  msg = WV_MSG_GENERIC;
}

QVariant WVMessage::getValue( const QVariant &dat ) const {
  //blocked_receive
  return dat;
}

void WVMessage::sendValue( const QVariant &dat ) const {
  if (!blocked_send)
    emit parent->genericMessage( msg, dat );
}

