/*******************************************************************************

  WVRemoteWidget is a main widget of a Remote Controller for WallViewer
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation

      
  ver: 2
        
*******************************************************************************/

#ifndef REMOTEWIDGET_H
#define REMOTEWIDGET_H

#include <QWidget>
#include <QPixmap>
#include <QMainWindow>
#include <QTime>
#include <QString>

#include <ui_remote.h>


#define WV_ORGANIZATION    "UCSB"
#define WV_APPLICATION     "BioViewRemote"
#define WV_CONFIG_FILE     "wvremote.ini"
#define WV_REMOTE_VERSION  "0.1.1"

#include "wvclient.h"
#include "appconfig.h"

class WVMessageParser;
class DProgressCircle;
class DScaleBar;

#if (QT_VERSION >= 0x040100)
//#define signals Q_SIGNALS 
//#define slots Q_SLOTS
#endif

class QMouseEvent;

class WVRemoteWidget : public QMainWindow {
  Q_OBJECT

public:
  enum MsgTextType { 
    mttInfo=0, 
    mttWarning, 
    mttError
  };

  WVRemoteWidget(QWidget *parent = 0);
  ~WVRemoteWidget();

public slots:
  //callbacks
  void operationStart( bool determinate = true);
  void operationStop();
  void operationProgress( const QString &s, int total, int pos );
  void operationMessage( MsgTextType id, const QString &s );

public:

  void initChannels( const QStringList &l, int r, int g, int b );
  void setChannelRed( int v );
  void setChannelGreen( int v );
  void setChannelBlue( int v );
  void setChannelYellow( int v );
  void setChannelMagenta( int v );
  void setChannelCyan( int v );
  void initEnhancement( const QStringList &l, int def=0 );
  void setEnhancement( int v );

  void setThumbnail( const QPixmap &pm, unsigned int orig_w, unsigned int orig_h );
  void setOriginalImageSize( unsigned int orig_w, unsigned int orig_h );
  void setMetadata( const QString &txt );
  void setPages( unsigned int _num_pages );

  void updateView(const QRectF &);
  void updateScale(double scale);

public slots:

  void onViewChanged(double scale, const QRectF &r);
  void onViewChanged(double scale, int x, int y, int w, int h) {
    onViewChanged( scale, QRectF(x,y,w,h) ); }
  void onThumbAreaPosChanged(const QPointF &center);
  void onPageChanged(int);
  void onToolButtonPressed( const QPointF &, Qt::MouseButton  );

public slots:
  void onChannelsChanged(int);
  void onChannelsVisChanged(int);
  void onEnhancementChanged(int);
  void onZoomOutReleased();
  void onZoomInReleased();
  void onZoom11Released();
  void onSliderValueChanged(int);
  void onNextPageReleased();
  void onPrevPageReleased();
  void onInitPageReleased();

  void onCursorPositionChanged ( const QPointF & ); 

  void clientConnect();
  void clientDisconnect();
  void muHostClose();
  void muHostShowMetadata();
  void muHostShowScaleBar();

  void onClientConnected();
  void onClientDisconnected();
  void onClientError();

  void open();
  void fullVirtualScreen();
  void about();
  void help();

  void onConfigureWallHost();
  void onExtChannels();

// network messages
public slots:
  void nmChannelsListChanged    ( const QStringList & );  

  void nmRedChanged             ( const qint32 & );
  void nmGreenChanged           ( const qint32 & );
  void nmBlueChanged            ( const qint32 & );
  void nmYellowChanged          ( const qint32 & );
  void nmMagentaChanged         ( const qint32 & );
  void nmCyanChanged            ( const qint32 & );

  void nmEnhancementChanged     ( const qint32 & );
  void nmEnhancementListChanged ( const QStringList & );

  void nmScaleChanged           ( const double & );
  void nmPageChanged            ( const qint32 & );
  void nmNumPagesChanged        ( const qint32 & );
  void nmImageSizeChanged       ( const QSize & );
  void nmMetadataChanged        ( const QString & );

  void nmThumbnailChanged       ( const QPixmap & );

  void nmVisibleAreaChanged     ( const QRectF & ); 

  void nmOperationStart         ( const QString &v );
  void nmOperationEnd           ( );
  void nmOperationProgress      ( const qint32 &v );

  void nmPhysPixelSizeX         ( const double & );
  void nmPhysPixelSizeUnit      ( const QString & );

  void nmImageInfoText          ( const QString & );

signals:
  void channelsChanged(int r, int g, int b);
  void enhancementChanged(int e);
  void zoomChanged(int z);
  void visCenterChanged( const QPointF &center );
  void pageChanged( const qint32 );

protected:
  void mouseMoveEvent ( QMouseEvent * event );
  void mousePressEvent ( QMouseEvent * event );
  void mouseReleaseEvent ( QMouseEvent * event );

  void resizeEvent(QResizeEvent *);

  void closeEvent ( QCloseEvent * );

  //void keyPressEvent ( QKeyEvent * );
  //void keyReleaseEvent ( QKeyEvent * );

private:
  Ui::ControlsForm ui;
  
  QStatusBar *status_bar;
  QLabel *statusImageInf;
  DProgressCircle *progress_circle;
  DScaleBar *scaleBar;

  QString operation_message;
  bool show_scale_bar;
  double pixel_scale;
  double thumb_ratio;
  int display_channels_visibility[6];

  QPoint mousePoint;
  bool in_mouse_move;
  QTime time_prgs;

  unsigned int cur_page;
  unsigned int num_pages;

  void initMessager();
  void setChildrenEnabled(bool);

  void createStatusBar();

//--------------------------------------
// misc
//--------------------------------------
private:
  DConfig *conf;

//--------------------------------------
// Network
//--------------------------------------
private:
  WVClient client;
  WVMessageParser *messager; 

};

#endif // REMOTEWIDGET_H
