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

#ifndef WV_MESSAGEPARSER_H
#define WV_MESSAGEPARSER_H

//#include <QObject>
//#include <QVariant>
//#include <QPointF>
#include <QtCore>
#include <QHash>

#include "wvmessenger.h"

#define WV_MSG_GENERIC                "WV_MSG_GENERIC"           // 
 
#define WV_MSG_WARNING                "WARNING"                  // sends QString
#define WV_MSG_ERROR                  "ERROR"                    // sends QString

#define WV_MSG_OPERATION_START        "OPERATION_START"          // sends QString
#define WV_MSG_OPERATION_END          "OPERATION_END"            //
#define WV_MSG_OPERATION_PROGRESS     "OPERATION_PROGRESS"       // sends qint32

#define WV_MSG_POSITION_CHANGED       "POSITION_CHANGED"         // sends QPointF
#define WV_MSG_VISIBLE_AREA_CHANGED   "VISIBLE_AREA_CHANGED"     // sends QRectF
#define WV_MSG_SCALE_CHANGED          "SCALE_CHANGED"            // sends double

#define WV_MSG_RED_CHANGED            "RED_CHANGED"              // sends qint32
#define WV_MSG_GREEN_CHANGED          "GREEN_CHANGED"            // sends qint32
#define WV_MSG_BLUE_CHANGED           "BLUE_CHANGED"             // sends qint32
#define WV_MSG_YELLOW_CHANGED         "YELLOW_CHANGED"           // sends qint32
#define WV_MSG_MAGENTA_CHANGED        "MAGENTA_CHANGED"          // sends qint32
#define WV_MSG_CYAN_CHANGED           "CYAN_CHANGED"             // sends qint32
#define WV_MSG_VIEW_CHAN_CHANGED      "VIEW_CHAN_CHANGED"        // sends QColor
#define WV_MSG_VIEW_CHANNELS_CHANGED  "VIEW_CHANNELS_CHANGED"    // sends QList<QVariant>

#define WV_MSG_ENHANCEMENT_CHANGED    "ENHANCEMENT_CHANGED"      // sends qint32

#define WV_MSG_PAGE_CHANGED           "PAGE_CHANGED"             // sends qint32
#define WV_MSG_NUM_PAGES_CHANGED      "NUM_PAGES_CHANGED"        // sends qint32

#define WV_MSG_THUMBNAIL_CHANGED      "THUMBNAIL_CHANGED"        // sends QPixmap
#define WV_MSG_THUMB_SIZE_CHANGED     "THUMB_SIZE_CHANGED"       // sends QSize
#define WV_MSG_METADATA_CHANGED       "METADATA_CHANGED"         // sends QString
#define WV_MSG_IMAGE_SIZE             "IMAGE_SIZE"               // sends QSize
#define WV_MSG_IMAGE_INFO_TEXT        "IMAGE_INFO_TEXT"          // sends QString

#define WV_MSG_PHYS_PIXEL_SIZE_X      "PHYS_PIXEL_SIZE_X"        // sends double
#define WV_MSG_PHYS_PIXEL_SIZE_UNIT   "PHYS_PIXEL_SIZE_UNIT"     // sends QString

#define WV_MSG_CHAN_LIST_CHANGED      "CHANNELS_LIST"            // sends QStringList
#define WV_MSG_ENHS_LIST_CHANGED      "ENHANCEMENT_LIST"         // sends QStringList

#define WV_MSG_LOCAL_FILE_NAME        "LOCAL_FILE_NAME"          // sends QString
#define WV_MSG_BLOB                   "BLOB"                     // sends QByteArray
#define WV_MSG_IMAGE_FILE             "IMAGE_FILE"               // sends QByteArray
#define WV_MSG_IMAGE_STREAM           "IMAGE_STREAM"             // sends QFile
#define WV_MSG_FULL_VIRTUAL_SCR       "FULL_VIRTUAL_SCR"         //

#define WV_MSG_CURSOR_POSITION        "CURSOR_POSITION"          // sends QPointF

#define WV_MSG_CLOSE_WALL             "CLOSE_WALL"               //
#define WV_MSG_SHOW_SCALEBAR          "SHOW_SCALEBAR"            // sends bool
#define WV_MSG_SHOW_METADATA          "SHOW_METADATA"            // sends bool

#define WV_MSG_TRIG_SMOOTH_ZOOM       "TRIG_SMOOTH_ZOOM"         //
#define WV_MSG_SHOW_SMOOTH_ZOOM       "SHOW_SMOOTH_ZOOM"         // sends bool
#define WV_MSG_TRIG_DYN_ENHANCE       "TRIG_DYN_ENHANCE"         //
#define WV_MSG_SHOW_DYN_ENHANCE       "SHOW_DYN_ENHANCE"         // sends bool
#define WV_MSG_TRIG_ROT_RIGHT         "TRIG_ROT_RIGHT"           //
#define WV_MSG_TRIG_ROT_LEFT          "TRIG_ROT_LEFT"            //
#define WV_MSG_TRIG_ROT_180           "TRIG_ROT_180"             //

#define WV_MSG_ZOOM_IN                "ZOOM_IN_AT"               // sends QPointF
#define WV_MSG_ZOOM_OUT               "ZOOM_OUT_AT"              // sends QPointF

#define WV_MSG_TRIG_PROJECT_MAX       "TRIG_PROJECT_MAX"         //
#define WV_MSG_TRIG_PROJECT_MIN       "TRIG_PROJECT_MIN"         //

#define WV_MSG_TRIG_NEGATIVE          "TRIG_NEGATIVE"            //
#define WV_MSG_BRIGHTNESS_CONTRAST    "BRIGHTNESS_CONTRAST"      // sends QList<QVariant>
#define WV_MSG_LEVELS                 "LEVELS"                   // sends QList<QVariant>

class WVMessageParser;

class WVMessage  {

public:
  WVMessage( WVMessageParser *_parent );

  QString msg;
  QVariant data;
  bool blocked_send;
  bool blocked_receive;

  virtual QVariant getValue( const QVariant &dat ) const;
  virtual void sendValue( const QVariant &dat ) const;

protected:
  WVMessageParser *parent;
};

class WVMessage;
class WVMessage_Warning;



class WVMessageParser : public QObject {
  Q_OBJECT

public:
  // we must connect to a valid messenger that should exist through all life of WVMessageParser
  WVMessageParser( const WVMessenger *messenger );
  WVMessageParser( ) {}
  void init( const WVMessenger *messenger ); 

  void blockSending( bool block );
  //void blockSending( const QString &msg, bool block );
  void blockReceiving( bool block );
  //void blockReceiving( const QString &msg, bool block );

signals:
  void genericMessage             ( const QString &msg, const QVariant &data );
  
  void warningMessage             ( const QString & );
  void errorMessage               ( const QString & );

  void positionChanged            ( const QPointF & );
  void visibleAreaChanged         ( const QRectF & );
  void scaleChanged               ( const double & );
  void zoomIn                     ( const QPointF & );
  void zoomOut                    ( const QPointF & );

  void redChanged                 ( const qint32 & );
  void greenChanged               ( const qint32 & );
  void blueChanged                ( const qint32 & );
  void yellowChanged              ( const qint32 & );
  void magentaChanged             ( const qint32 & );
  void cyanChanged                ( const qint32 & );
  void viewChanChanged            ( const QColor & );
  void viewChannelsChanged        ( const QList<QVariant> & );
  
  void enhancementChanged         ( const qint32 & );

  void pageChanged                ( const qint32 & );
  void numPagesChanged            ( const qint32 & );
  void imageSizeChanged           ( const QSize & );
  void metadataChanged            ( const QString & );
  void thumbnailChanged           ( const QPixmap & );
  void thumbSizeChanged           ( const QSize & );

  void channelsListChanged        ( const QStringList & );
  void enhancementListChanged     ( const QStringList & );

  void localFileName              ( const QString & );

  void operationStart             ( const QString & );
  void operationEnd               ( );
  void operationProgress          ( const qint32 & );

  void physPixelSizeX             ( const double & );
  void physPixelSizeUnit          ( const QString & );

  void imageInfoText              ( const QString & );

  void fullVirtualScreen          ( );

  void cursorPositionChanged      ( const QPointF & );

  void closeWall                  ( );
  void showScaleBarChanged        ( bool );
  void showMetadataChanged        ( bool );

  void trigSmoothZoom             ( );
  void showSmoothZoom             ( bool );
  void trigDynEnhance             ( );
  void showDynEnhance             ( bool );
  void rotateRight                ( );
  void rotateLeft                 ( );
  void rotate180                  ( );

  void projectMax                 ( );
  void projectMin                 ( );
  void negative                   ( );
  void brightnessContrast         ( const QList<QVariant> & );
  void levels                     ( const QList<QVariant> & );

  void fileBlob                   ( const QString &, const QByteArray & );

public slots:
  void sendGenericMessage         ( const QString &msg, const QVariant &data );

  void sendWarningMessage         ( const QString & );
  void sendErrorMessage           ( const QString & );

  void sendPositionChanged        ( const QPointF & );
  void sendVisibleAreaChanged     ( const QRectF & );
  void sendScaleChanged           ( const double & );

  void sendRedChanged             ( const qint32 & );
  void sendGreenChanged           ( const qint32 & );
  void sendBlueChanged            ( const qint32 & );
  void sendYellowChanged          ( const qint32 & );
  void sendMagentaChanged         ( const qint32 & );
  void sendCyanChanged            ( const qint32 & );
  void sendViewChanChanged        ( const QColor & );
  void sendViewChannelsChanged    ( const QList<QVariant> & );


  void sendEnhancementChanged     ( const qint32 & );

  void sendPageChanged            ( const qint32 & );
  void sendNumPagesChanged        ( const qint32 & );
  void sendImageSizeChanged       ( const QSize & );
  void sendMetadataChanged        ( const QString & );
  void sendThumbnailChanged       ( const QPixmap & );
  void sendThumbSizeChanged       ( const QSize & );

  void sendChannelsListChanged    ( const QStringList & );
  void sendEnhancementListChanged ( const QStringList & );

  void sendLocalFileName          ( const QString & );
  // this function is only intended for small blobs!!!!!!!!!!!!!!!!!!!!!
  void sendBlob                   ( const QByteArray & );
  // this function is only intended for small files!!!!!!!!!!!!!!!!!!!!!
  void sendFile                   ( const QString & );

  void sendOperationStart         ( const QString & );
  void sendOperationEnd           ( );
  void sendOperationProgress      ( const qint32 & );

  void sendPhysPixelSizeX         ( const double & );
  void sendPhysPixelSizeUnit      ( const QString & );
  
  void sendImageInfoText          ( const QString & );

  void sendFullVirtualScreen      ( );

  void sendCursorPosition         ( const QPointF & );

  void sendCloseWall              ( );
  void sendShowScaleBar           ( bool );
  void sendShowMetadata           ( bool );

  void sendTrigSmoothZoom         ( );
  void sendShowSmoothZoom         ( bool );
  void sendTrigDynEnhance         ( );
  void sendShowDynEnhance         ( bool );
  void sendRotateRight            ( );
  void sendRotateLeft             ( );
  void sendRotate180              ( );

  void sendZoomIn                 ( const QPointF & );
  void sendZoomOut                ( const QPointF & );

  void sendProjectMax             ( );
  void sendProjectMin             ( );
  void sendNegative               ( );
  void sendBrightnessContrast     ( const QList<QVariant> & );
  void sendLevels                 ( const QList<QVariant> & );

//------------------------------------------------------------------------------
// internals
//------------------------------------------------------------------------------

signals:
  void rawMessage( const QString &msg, const QVariant &data );
  
private slots:
  virtual void onRawMessage( const QString &msg, const QVariant &data );

private:
  bool block_sending_all;
  bool block_receiving_all;

  friend class WVMessage;
  friend class WVMessage_Warning;

  QHash<QString, WVMessage> messages;
  void addMessage( const WVMessage &msg );
};

#endif // WV_MESSAGEPARSER_H

