/*******************************************************************************
  
  TileViewer is a main widget of the viewer app, it controls all user inputs
  and owns all other necessary widgets
    
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
    2007-10-09 16:46 - centering of small images
      
  ver: 6
       
*******************************************************************************/

#ifndef TILEVIEWER_H
#define TILEVIEWER_H

#include <QMainWindow>
#include <QPixmap>
#include <QImage>
#include <QTime>
#include <QtNetwork>

//#define CDECL __cdecl
#define CDECL

#define WV_ORGANIZATION "UCSB"
#define WV_APPLICATION  "BioView"
#define WV_CONFIG_FILE  "wv.ini"
#define WV_VERSION      "1.1.18"


#include "wvimagemanager.h"
//#include "controlsWidget.h"

#include "wvserver.h"
#include "appconfig.h"
#include "progresscircle.h"


class WVMessageParser;
class QAction;
class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
class QPaintEvent;
class WVControlsWidget;
class QMouseEvent;
class QProgressBar;
class TileViewer;

class DScaleBar;
class DMetaDataBar;

//class DScrollImageView;
#include "scrollImageView.h"

// Graphic annotation - begin
#ifdef WV_GR_LAYER_SUPPORT
#include "../qdancommondefinitions.h"
class AnnotatorControlsWidget;
class gView;
class TreeModel;
class TreeItem;
class TreeView;
class gConfigDialog;
#endif
// Graphic annotation - end

class TileViewer : public QMainWindow
{
    Q_OBJECT

public:
  enum MsgTextType { 
    mttInfo=0, 
    mttWarning, 
    mttError
  };

  TileViewer();
  ~TileViewer();

  void loadImage( const QString &fileName, int page_to_load=-1 );
  void clearImage();
  // this will open image viewer as a dialog and will notify in return if something changed
  int exec( const QString &fileName );

  void operationInterruptPrevious();
  bool operationTestAbort();

public slots:
  //callbacks
  void operationStart( bool determinate = true);
  void operationStartNow();
  void operationStop();
  void operationProgress( const QString &s, int total, int pos );
  void operationProgressDT( const QString &s, unsigned int done, unsigned int total ) { operationProgress( s, total, done ); }
  void operationMessage( MsgTextType id, const QString &s );
  // this function waits until prev operation is finihed and then exits

  void serverStart();
  void serverStop();
  void serverDisconnect();

  void fullScreen();
  void fullVirtualScreen();
  void showControls();
  void trigShowScale();
  void trigShowMeta();


  void onOtherInstanceMessage( const QString & message );

// Graphic annotation - begin
signals:
  void dataUpdated(const QString &, bool save = false );
  void reloadRequest();
  void loadNextRequest();
  void loadPrevRequest();

private slots:
  void onApplyBtnClicked();
  void onOkBtnClicked();
  void onCancelBtnClicked();
  void onProjectGraphicsToAllPages(bool);
  void onSaveBtnClicked();
  void onReloadBtnClicked();
  void onNextBtnClicked();
  void onPrevBtnClicked();

#ifdef WV_GR_LAYER_SUPPORT
public:
  void setGObjectData( const QString & );
  void setEnvironment( const QString & );
  void setPixelSize( const QVector<double> &v ) { im.setPixelSize(v); initScaleBar(); }

private:
  AnnotatorControlsWidget *annotatorControls;	
  QWidget *bottomWidget;
  gView	*gview;

  bool openTemplate( const QString &fileName );
  
  void drawAnnotations ( QPixmap * );
  void drawAnnotations ( QPainter * );
#endif
// Graphic annotation - end

private slots:

  void open();
  void openBisquik();
  void configDownload();
  void saveDisplay();
  void saveDisplayClipboard();
  void printDisplay();
  void zoomIn( const QPointF &center = QPointF(-1,-1) ) { changeZoom(1, false, center); }
  void zoomOut( const QPointF &center = QPointF(-1,-1) ) { changeZoom(-1, false, center); }
  void normalSize() { changeZoom( 0 ); }
  void fitToWindow();
  void about();
  void formats();
  void help();
  void useLargeCursors();
  void useWheelToPan();
  void useSmoothZoom();
  void trigSmoothZoom();
  void useDynamicEnhancement();
  void trigDynamicEnhancement();
  void centerSmallImages();


  void onChannelsChanged(const int &r, const int &g, const int &b, const int &y, const int &m, const int &c);
  void onEnhancementChanged(int e);
  void onZoomChanged(int z);
  void onVisCenterChanged(const QPointF &center);
  void onVisCenterSelected(const QPointF &center);
  void onPageChanged(int p);

  void onVisCursorMoved( const QPoint & );

  void onClientConnected();
  void onClientDisconnected();

  void updateVisArea( const QRectF & );
  void changeZoom( int z, bool z_is_absolute=false, const QPointF &center = QPointF(-1,-1) );

  void toolIndexChanged ( int index );

  void onRepaintRequired();

  void onRotate( double deg );
  void onRotate90() { onRotate(90); }
  void onRotate180() { onRotate(180); }
  void onRotate270() { onRotate(-90); }

  void onProjectMax( );
  void onProjectMin( );

  void onSliderT( int );
  void onSliderZ( int );

private:
  void createActions();
  void createMenus();
  void createStatusBar();
  void updateActions();

  void updateView();

  void initNewImage();
  void initNewImageControls();
  void initNewImageClient();

  void loadConfig();

  DScrollImageView *imageScroll;
  QLabel *imageLabel;
  WVControlsWidget *controls;
  QStatusBar *status_bar;

  QWidget *centralwidget;
  QGridLayout *gridLayout;
  QSlider *horizontalSlider;
  QSlider *verticalSlider;


  QLabel *statusImageInf;
  QLabel *statusPosition;
  QLabel *statusMessages;
  QProgressBar *statusProgress;
  DProgressCircle *progress_circle;
  DScaleBar *scaleBar;
  DMetaDataBar *metadataBar;

  QAction *openAct;
  QAction *openFromBisqikAct;
  QAction *configDownloadPathAct;
  QAction *saveDisplayAct;
  QAction *saveDisplayClipboardAct;
  QAction *printDisplayAct;
  QAction *exitAct;
  QAction *saveAct;
  QAction *reloadAct;
  QAction *zoomInAct;
  QAction *zoomOutAct;
  QAction *normalSizeAct;
  QAction *fitToWindowAct;
  QAction *fullScreenAct;
  QAction *fullVirtualScreenAct;
  QAction *useLargeCursorsAct;
  QAction *useSmoothZoomAct;
  QAction *useDynamicEnhanceAct;
  QAction *useWheelToPanAct;
  QAction *centerSmallImagesAct;
  QAction *helpAct;
  QAction *aboutAct;
  QAction *formatsAct;
  QAction *serverStartAct;
  QAction *serverStopAct;
  QAction *serverDisconnectAct;
  QAction *controlsAct;

  QAction *rotate90Act;
  QAction *rotate180Act;
  QAction *rotate270Act;
  QAction *projectMaxAct;
  QAction *projectMinAct;

  QAction *showScaleBarAct;
  QAction *showMetadataAct;

  QMenu *fileMenu;
  QMenu *viewMenu;
  QMenu *modifyMenu;
  QMenu *serverMenu;
  QMenu *windowMenu;
  QMenu *helpMenu;

private slots:
  void nmLocalFileName      ( const QString & );
  void nmScaleChanged       ( const double & );

  void nmPositionChanged    ( const QPointF & );
  void nmRedChanged         ( const qint32 & );
  void nmGreenChanged       ( const qint32 & );
  void nmBlueChanged        ( const qint32 & );
  void nmYellowChanged      ( const qint32 & );
  void nmMagentaChanged     ( const qint32 & );
  void nmCyanChanged        ( const qint32 & );
  void nmEnhancementChanged ( const qint32 & );
  void nmPageChanged        ( const qint32 & );
  void nmThumbSizeChanged   ( const QSize & );
  void nmFullVirtualScreen  ( );

  void nmCursorPosition     ( const QPointF & );
  
  void nmCloseWall          ( );
  void nmShowScaleBar       ( bool );
  void nmShowMetadata       ( bool );
  void nmViewChanChanged    ( const QColor & );
  void nmViewChannelsChanged ( const QList<QVariant> & );

  void nmFileBlob           ( const QString &, const QByteArray & );

  void nmZoomIn( const QPointF &center )  { zoomIn(im.imageToView(center));  }
  void nmZoomOut( const QPointF &center ) { zoomOut(im.imageToView(center)); }

private:
  WVImageManager im;
  double scaleFactor;
  bool in_operation;
  bool operation_done;
  QSize prev_image_size;
  QSize thumb_size;
  QSize client_thumb_size;
  bool full_virtual_screen;

  QString cur_file_name;
  int     cur_page;
  QPointF prev_view_center;

  QTime time_prgs;
  QTime time_oper;
  int rotation_angle;

  QPrinter printer;

  void centerViewAtImagePoint( const QPointF &cent, bool force_full_repaint = false );
  void initMessager();
  QImage renderDisplay();
  void addRotation(double ang);

  void initScaleBar();
  void initMetadataBar();
  void initSlidersZT();
  void updateSlidersByPage(int p);

protected:
  void resizeEvent(QResizeEvent *);
  void closeEvent ( QCloseEvent * );
  void dragEnterEvent ( QDragEnterEvent * );
  void dropEvent ( QDropEvent * );

  void keyPressEvent ( QKeyEvent * );
  void keyReleaseEvent ( QKeyEvent *);

//--------------------------------------
// misc
//--------------------------------------
protected:
  DConfig *conf;
  friend class DScrollImageView;

//--------------------------------------
// Network
//--------------------------------------
private:
  WVServer server;
  WVMessageParser *messager; 

//--------------------------------------
// C Callbacks workaround
//--------------------------------------
private:
  static TileViewer *self_ref;

public:
  static void CDECL tv_progress_proc (long done, long total, char *descr) {
    self_ref->operationProgress( QString(descr), total, done );
  }

  static int CDECL tv_test_abort_proc() { 
    return !self_ref->in_operation; 
  }

};


#endif


