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

#include <algorithm>

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include <QPaintEvent>
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPoint>
#include <QMouseEvent>
#include <QStatusBar>
#include <QProgressBar>
#include <QPointF>
#include <QPixmap>

#include <BioImageCore>
#include <BioImageFormats>

#include "tileviewer.h"
#include "controlsWidget.h"
#include "scalebar.h"

#include "wvmessageparser.h"
#include "notifyWidget.h"
#include "scrollImageView.h"

#include "bisquikAccess.h"
#include "bisquikWebAccess.h"

// Graphic annotation - begin
#ifdef WV_GR_LAYER_SUPPORT
#include <QDomDocument>
#include "AnnotatorControlsWidget.h"
#include "gview.h"
#include "treemodel.h"
#include "treeitem.h"
#endif
// Graphic annotation - end

#ifdef WV_EMBEDDED_CONTROLS
#undef WV_APPLICATION
#undef WV_CONFIG_FILE
#define WV_APPLICATION  "BioView4DN"
#define WV_CONFIG_FILE  "wv4dn.ini"
#endif

#if defined(Q_WS_X11)
#define WV_DISABLE_ONSCREEN_WIDGETS
#endif

//#define WV_DISABLE_COMMAND_INTERRUPTION

//#define WV_DISABLE_STANDALONE_MENUS
//#define WV_EMBEDDED_CONTROLS


//*****************************************************************************
// TileViewer
//*****************************************************************************

TileViewer* TileViewer::self_ref = NULL;

TileViewer::TileViewer() {
  //if (self_ref == NULL) 
    self_ref = this;
  
  messager = new WVMessageParser( &server ); 
  connect(&server, SIGNAL( connected(QTcpSocket *) ), this, SLOT( onClientConnected() ));
  connect(&server, SIGNAL( disconnected(QTcpSocket *) ), this, SLOT( onClientDisconnected() ));
  initMessager();


  conf = new DConfig( WV_ORGANIZATION, WV_APPLICATION, WV_CONFIG_FILE );
  conf->loadConfig();

  setObjectName( "MainWidget" );
  setWindowTitle(tr("bioView"));
  progress_circle = NULL;
  
  in_operation = false;
  operation_done = true;
  prev_image_size = QSize(0,0);
  cur_file_name = "";
  cur_page = 0;
  thumb_size = QSize(200,200);
  client_thumb_size = QSize(200,200);
  full_virtual_screen = false;
  rotation_angle = 0;
 
  im.setCallbacks( tv_progress_proc, NULL, tv_test_abort_proc );
  connect(&im, SIGNAL( repaintRequired() ), this, SLOT( onRepaintRequired() ));

  imageScroll = new DScrollImageView(this);

  #ifndef WV_DISABLE_COMMAND_INTERRUPTION
  imageScroll->setViewerParent(this);
  #endif
  imageScroll->setImageManager( &im );
  
  
  // sliders for Z and T
  centralwidget = new QWidget(this);
  centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
  gridLayout = new QGridLayout(centralwidget);
  gridLayout->setSpacing(2);
  gridLayout->setContentsMargins(2, 2, 2, 2);
  gridLayout->setObjectName(QString::fromUtf8("gridLayout"));

  gridLayout->addWidget(imageScroll, 0, 0, 1, 1);

  horizontalSlider = new QSlider(centralwidget);
  horizontalSlider->setObjectName(QString::fromUtf8("horizontalSlider"));
  horizontalSlider->setOrientation(Qt::Horizontal);
  horizontalSlider->setToolTip(QApplication::translate("MainWindow", "T (time)", 0, QApplication::UnicodeUTF8));

  gridLayout->addWidget(horizontalSlider, 1, 0, 1, 1);

  verticalSlider = new QSlider(centralwidget);
  verticalSlider->setObjectName(QString::fromUtf8("verticalSlider"));
  verticalSlider->setOrientation(Qt::Vertical);
  verticalSlider->setInvertedAppearance ( true );
  verticalSlider->setInvertedControls ( true );
  verticalSlider->setToolTip(QApplication::translate("MainWindow", "Z (depth)", 0, QApplication::UnicodeUTF8));

  gridLayout->addWidget(verticalSlider, 0, 1, 1, 1);

  horizontalSlider->setVisible(false);
  verticalSlider->setVisible(false);

  setCentralWidget(centralwidget);
  //setCentralWidget(imageScroll);

  connect(imageScroll, SIGNAL( zoomChangeRequest(int, bool, const QPointF &) ), 
          this,     SLOT( changeZoom( int, bool, const QPointF &) ));

  connect(imageScroll, SIGNAL( viewAreaMoved(const QRectF &) ), 
          this,     SLOT( updateVisArea(const QRectF &) ));

  connect(imageScroll, SIGNAL( cursorMoved(const QPoint &) ), 
          this,     SLOT( onVisCursorMoved(const QPoint &) ));


  connect(horizontalSlider, SIGNAL( valueChanged(int) ), this, SLOT( onSliderT(int) ));
  connect(verticalSlider, SIGNAL( valueChanged(int) ), this, SLOT( onSliderZ(int) ));
  
  controls = new WVControlsWidget( this );
  controls->initChannels( im.channelNames(), 
                          im.channelMapping( WVImageManager::dcRed ), 
                          im.channelMapping( WVImageManager::dcGreen ), 
                          im.channelMapping( WVImageManager::dcBlue ) );
  controls->initEnhancement( im.enhancementTypes(), 0 );

  // add lut modifiers
  for (int m=im.lutModifiers().size()-1; m>=0; --m)
    controls->insertLutModifier( im.lutModifiers()[m] );

  connect(controls, SIGNAL( channelsChanged(const int &,const int &,const int &,const int &,const int &,const int &) ), 
          this,     SLOT( onChannelsChanged(const int &,const int &,const int &,const int &,const int &,const int &) ));
  connect(controls, SIGNAL( enhancementChanged(int) ), this, SLOT( onEnhancementChanged(int) ));
  connect(controls, SIGNAL( zoomChanged(int) ), 
          this,     SLOT( onZoomChanged(int) ));
  connect(controls, SIGNAL( visCenterChanged(const QPointF &) ), 
          this,     SLOT( onVisCenterChanged(const QPointF &) ));
  connect(controls, SIGNAL( visCenterSelected(const QPointF &) ), 
          this,     SLOT( onVisCenterSelected(const QPointF &) ));
  
  connect(controls, SIGNAL( pageChanged(int) ), 
          this,     SLOT( onPageChanged(int) ));

  // if the controls window should be enabled then create dock widget
  #ifdef WV_EMBEDDED_CONTROLS
  QDockWidget *controlsDockWidget = new QDockWidget(this, Qt::FramelessWindowHint);
  controlsDockWidget->setObjectName ( "controlsDockWidget" );
  controlsDockWidget->setAllowedAreas(Qt::RightDockWidgetArea );
  controlsDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
  controlsDockWidget->setWidget(controls);
  this->addDockWidget(Qt::RightDockWidgetArea, controlsDockWidget);
  controls->show();
  #endif // WV_EMBEDDED_CONTROLS

  //////////////////////////////////////////////////
  // Graphic annotator - begin
  //////////////////////////////////////////////////
  #ifdef WV_GR_LAYER_SUPPORT
  #ifdef WV_EMBEDDED_CONTROLS
  annotatorControls = new AnnotatorControlsWidget( this );
  
  QDockWidget *annotatorControlsDockWidget = new QDockWidget(this, Qt::FramelessWindowHint);
  annotatorControlsDockWidget->setObjectName ( "annotatorControlsDockWidget" );
  annotatorControlsDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea );
  annotatorControlsDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
  annotatorControlsDockWidget->setWidget(annotatorControls);

  this->addDockWidget(Qt::LeftDockWidgetArea, annotatorControlsDockWidget);

  connect(annotatorControls, SIGNAL( SelectionChanged(QModelIndexList) ), imageScroll, SLOT( onUpdateScreen() ));
  connect(annotatorControls, SIGNAL( toolChangeRequested(int) ), this, SLOT( toolIndexChanged(int) ));
  
  //bottom bar including apply, ok and cancel button
  bottomWidget = new QWidget( this );

  QPushButton* nextButton = new QPushButton( tr("&Next"), bottomWidget );
  connect(nextButton, SIGNAL(clicked()), this, SLOT(onNextBtnClicked()));

  QPushButton* prevButton = new QPushButton( tr("&Prev"), bottomWidget );
  connect(prevButton, SIGNAL(clicked()), this, SLOT(onPrevBtnClicked()));

  QPushButton* saveButton = new QPushButton( tr("&Save"), bottomWidget );
  connect(saveButton, SIGNAL(clicked()), this, SLOT(onSaveBtnClicked()));

  QPushButton* applyButton = new QPushButton( tr("&Apply"), bottomWidget );
  connect(applyButton, SIGNAL(clicked()), this, SLOT(onApplyBtnClicked()));
  QPushButton* okButton = new QPushButton( tr("&OK"), bottomWidget );
  connect(okButton, SIGNAL(clicked()), this, SLOT(onOkBtnClicked()));
  QPushButton* cancelButton = new QPushButton( tr("&Cancel"), bottomWidget );
  connect(cancelButton, SIGNAL(clicked()), this, SLOT(onCancelBtnClicked()));



  //void onSaveBtnClicked();
  //void onReloadBtnClicked();


  QGridLayout *bottomLayout = new QGridLayout;
  bottomLayout->setMargin( 0 );
  bottomLayout->setSpacing( 2 );
  //bottomLayout->addStretch( 500 );

  bottomLayout->addWidget( prevButton, 0, 0, 1, 2 );
  bottomLayout->addWidget( nextButton, 0, 2, 1, 2 );
  bottomLayout->addWidget( saveButton, 1, 0, 1, 1 );
  bottomLayout->addWidget( applyButton, 1, 1, 1, 1  );
  bottomLayout->addWidget( okButton, 1, 2, 1, 1  );
  bottomLayout->addWidget( cancelButton, 1, 3, 1, 1  );
  bottomWidget->setLayout(bottomLayout);

  QSizePolicy sizePolicy1(QSizePolicy::Maximum, QSizePolicy::Maximum);
  bottomWidget->setSizePolicy(sizePolicy1);

  QDockWidget *bottomDockWidget = new QDockWidget(this, Qt::FramelessWindowHint);
  bottomDockWidget->setObjectName ( "bottomDockWidget" );
  bottomDockWidget->setAllowedAreas(Qt::RightDockWidgetArea );
  bottomDockWidget->setFeatures(QDockWidget::NoDockWidgetFeatures);
  bottomDockWidget->setWidget(bottomWidget);

  this->addDockWidget(Qt::RightDockWidgetArea, bottomDockWidget);
  #endif // WV_EMBEDDED_CONTROLS

  gview = new gView(this);
  gview->setImageManager( &im );
  imageScroll->setGraphicsLayer( gview );
  controls->setGraphicsLayer( gview );
  
  connect(controls, SIGNAL( pageChanged(int) ), gview, SLOT( onPageChanged(int) ));
  connect(gview, SIGNAL( movePage(int) ), controls, SLOT( onSliderValueChanged(int) ));
  //connect(annotatorControls, SIGNAL( AddTemplate(QString) ), gview, SLOT( onAddTemplate(QString) ));
  connect(annotatorControls, SIGNAL( RemoveTemplate(QModelIndexList) ), gview, SLOT( onRemoveTemplate(QModelIndexList) ));
  connect(annotatorControls, SIGNAL( SelectionChanged(QModelIndexList) ), gview, SLOT( onSelectionChanged(QModelIndexList) ));
  connect(annotatorControls, SIGNAL( configChangeRequested(QModelIndexList) ), gview, SLOT( OnConfigChangeRequested(QModelIndexList) ));
  connect(annotatorControls, SIGNAL( FocusInTree() ), gview, SLOT( OnFocusInTree() ));
  connect(annotatorControls, SIGNAL( EnvSelectionChanged(QString, QString) ), gview, SLOT( OnEnvSelectionChanged(QString, QString) ));
  connect(annotatorControls, SIGNAL( projectGraphicsToAllPages(bool) ), this, SLOT( onProjectGraphicsToAllPages(bool) ));

  //connect(gview, SIGNAL( setTreeModel_data(TreeModel*) ), annotatorControls, SLOT( onSetTreeModel_data(TreeModel*) ));
  connect(gview, SIGNAL( envUpdated(QStringList, QStringList,QStringList, QStringList,QStringList, QStringList) ), 
          annotatorControls, SLOT( onEnvUpdated(QStringList, QStringList,QStringList, QStringList,QStringList, QStringList) ));
  connect(gview, SIGNAL( selectionUpdated(QModelIndexList) ), annotatorControls, SLOT( onSelectionUpdated(QModelIndexList) ));
  connect(gview, SIGNAL( removeSelected() ), annotatorControls, SLOT( onRemoveBtnReleased() ));
  connect(gview, SIGNAL( setViewCursor( WVCursorShape ) ), imageScroll, SLOT( onSetViewCursor( WVCursorShape ) ));
  
  connect(gview, SIGNAL(  mouseReleaseOnTool( const QString &) ), annotatorControls, SLOT( onToolMouseRelease( const QString &) ));

  #ifdef WV_EMBEDDED_CONTROLS
  annotatorControls->setTreeModel_data(gview->getTreeModel_data());
  #endif // WV_EMBEDDED_CONTROLS
  connect(controls, SIGNAL( pageChanged(int) ), gview->getTreeModel_data(), SLOT( onPageChanged(int) ));

  //QSettings qdanSettings( QSettings::IniFormat, QSettings::UserScope, DN_ORGANIZATION, DN_APPLICATION );
  //QString cixName = qdanSettings.value( "Configs/Last Config File", dn_templatePath()+"/xdma.cix").toString();

  //remove close button on title bar
  #ifdef WV_EMBEDDED_CONTROLS
  this->setWindowFlags(Qt::WindowTitleHint);
  #endif // WV_EMBEDDED_CONTROLS
  
  #endif //WV_GR_LAYER_SUPPORT
  //////////////////////////////////////////////////
  // Graphic annotator - end
  //////////////////////////////////////////////////


  createActions();
  createMenus();
  createStatusBar();

  // read config and start server if needed
  conf->loadWidgetConfig( this );
  conf->loadWidgetConfig( controls, true );
  //controls->show();

  scaleBar = NULL;
  scaleBar = new DScaleBar(this);
  scaleBar->hide();

  metadataBar = NULL;
  metadataBar = new DMetaDataBar(this);
  metadataBar->hide();

  loadConfig();

  #ifndef WV_EMBEDDED_CONTROLS
  setAcceptDrops(true);
  #endif
}

TileViewer::~TileViewer() {
  conf->saveConfig();
  if ( conf->getValue( "AllowSavePosition", true ).toBool() )
    conf->saveWidgetConfig( this );
  conf->saveWidgetConfig( controls );
  if (scaleBar != NULL) delete scaleBar;
  if (metadataBar != NULL) delete metadataBar;
  delete controls;
  if (self_ref) self_ref = NULL;
}

void TileViewer::loadConfig() {

  useLargeCursorsAct->setChecked( conf->getValue( "UseLargeCursors", false ).toBool() );
  imageScroll->setUseLargeCursors( useLargeCursorsAct->isChecked() );

  useSmoothZoomAct->setChecked( conf->getValue( "UseSmoothZoom", false ).toBool() );
  if ( useSmoothZoomAct->isChecked() )
    im.setZoomMethod( WVImageManager::dzmBL );
  else
    im.setZoomMethod( WVImageManager::dzmNN );

  useDynamicEnhanceAct->setChecked( conf->getValue( "UseDynamicEnhancement", false ).toBool() );  
  if ( useDynamicEnhanceAct->isChecked() )
    im.setEnhancemetArea( WVImageManager::deaOnlyVisible );
  else
    im.setEnhancemetArea( WVImageManager::deaFullImage );

  useWheelToPanAct->setChecked( conf->getValue( "UseWheelToPan", false ).toBool() );  
  if ( useWheelToPanAct->isChecked() )
    imageScroll->setMouseScrollTool( DScrollImageView::mstPan );
  else
    imageScroll->setMouseScrollTool( DScrollImageView::mstZoom );

  centerSmallImagesAct->setChecked( conf->getValue( "CenterSmallImages", true ).toBool() );  
  imageScroll->setCenterSmallImages( centerSmallImagesAct->isChecked() );

  if ( conf->getValue( "FullVirtualScreen", false ).toBool() ) fullVirtualScreen();
}

//--------------------------------------------------------------------------------------------
//callbacks
//--------------------------------------------------------------------------------------------

void TileViewer::operationStart( bool determinate ) {

  QApplication::processEvents();
  time_prgs.start();
  time_oper.start();
  in_operation = true;
  operation_done = false;
}

void TileViewer::operationStartNow() {
  operationStart( );
  time_prgs.addSecs( 60 );
  operationProgress( "", 100, 0 );
}

void TileViewer::operationStop() {

  if (statusProgress != NULL) {
    status_bar->removeWidget( statusProgress );    
    delete statusProgress; 
    statusProgress = NULL;
  }

  if (progress_circle != NULL) {
    delete progress_circle;
    progress_circle = NULL;
  }

  in_operation = false;
  operation_done = true;
  statusMessages->setText( tr("Done in %1ms").arg(time_oper.elapsed()) );
  messager->sendOperationEnd();
  QApplication::processEvents();
}

void TileViewer::operationProgress( const QString &s, int total, int pos ) {
  if ( !in_operation ) return;
  if ( operation_done ) return;
  if (time_prgs.elapsed() < 500 ) return;

  if (statusProgress == NULL) {
    statusProgress = new QProgressBar(this);
    statusProgress->setMaximum ( 100 );
    statusProgress->setMinimum ( 0 );
    status_bar->addWidget ( statusProgress );
  }

  #ifndef WV_DISABLE_ONSCREEN_WIDGETS
  if (progress_circle == NULL) {
    progress_circle = new DProgressCircle(this);
    progress_circle->showBasedOn( this );
    messager->sendOperationStart( "Processing..." );
  }
  #endif

  int perc = pos*100.0/total;

  if (statusProgress != NULL) {
    statusProgress->setTextVisible( true );
    statusProgress->setValue( perc );
  }
  statusMessages->setText(s);

  if (progress_circle != NULL) {
    progress_circle->setProgress( perc );
  }
  
  messager->sendOperationProgress( perc );
  QApplication::processEvents();
  time_prgs.start();
}

void TileViewer::operationMessage( MsgTextType id, const QString &s ) {
  //QString app = tr("BioView");

  if (id == mttInfo)
    //QMessageBox::information(this, app, s);
    DNotificationWidget::information(s, 5000, this);
  else
  if (id == mttWarning)
    //QMessageBox::warning(this, app, s);
    DNotificationWidget::warning(s, 10000, this);
  else
  if (id == mttError)
    //QMessageBox::critical(this, app, s);
    DNotificationWidget::error(s, 50000, this);

}

void TileViewer::operationInterruptPrevious() {
  in_operation = false;  
  QApplication::processEvents();
}

bool TileViewer::operationTestAbort() { 
  QApplication::processEvents();
  return !in_operation; 
}

//--------------------------------------------------------------------------------------------
//files
//--------------------------------------------------------------------------------------------

void TileViewer::clearImage() {
  operationInterruptPrevious();
  
  im.clear();
  setWindowTitle(tr("bioView"));
  statusImageInf->setText ( "" );
  cur_file_name = "";
  prev_image_size = QSize(0,0); 
  controls->onPageChanged( 0 );
  controls->setThumbnail( im.getThumbnail(thumb_size), 0, 0 );
  imageScroll->repaintView();
  scaleBar->hide();
}

int TileViewer::exec( const QString &fileName ) {

  //if (genEventLoop) {
  //  qWarning("QDialog::exec: Recursive call detected");
  //  return -1;
  //}

  setAttribute(Qt::WA_DeleteOnClose, false);
  setAttribute(Qt::WA_ShowModal, true);

#ifdef WV_GR_LAYER_SUPPORT
  if(gview != NULL) gview->clear();
#endif
  clearImage();
  show();
  loadImage(fileName);
  resizeEvent(NULL);

  //QEventLoop eventLoop;
  //genEventLoop = &eventLoop;
  //(void) eventLoop.exec();
  //genEventLoop = 0;

  return 1;
}

#ifdef WV_GR_LAYER_SUPPORT
void TileViewer::drawAnnotations ( QPixmap *pm ) {
  if (!pm || pm->isNull()) return;
  if (!gview) return;

  QMatrix graphicsMatrix; 
  double sx = (double) pm->width() / (double) im.imageWidth();
  graphicsMatrix.scale(sx, sx);
  QPainter p(pm);
  p.setMatrix(graphicsMatrix);
  drawAnnotations(&p);
}

void TileViewer::drawAnnotations ( QPainter *p ) {
  if (!p) return;
  if (!gview) return;
  QRegion rgn(imageScroll->viewArea().toRect());
  gview->paint( p, rgn );
}
#endif //WV_GR_LAYER_SUPPORT

void TileViewer::loadImage( const QString &fn, int page_to_load ) {
  operationInterruptPrevious();
  QString fileName = fn;
  
  if (fileName.startsWith("bioview://")) {
    BQUrl bqu(fileName);
    QString path = conf->loadPath( "DownloadPath" );
    BQAccessWrapper bqdialog;
    bqdialog.setPath( path );
    bqdialog.setUserName( bqu.user() );
    bqdialog.setPassword( bqu.password() );
    connect(&bqdialog, SIGNAL(inProcess( const QString &, unsigned int, unsigned int)), this, SLOT(operationProgressDT(const QString &, unsigned int, unsigned int)));

    operationStart();
    if ( bqdialog.doDownload( bqu.url() ) )
      fileName = bqdialog.imageFileName();
    operationStop();
  } // if "bioview://" url


  if (fileName.size() < 1) return;
  operationStart();
  
  if (!im.loadImage(fileName, page_to_load) ) {
    operationStop();
    operationMessage( mttInfo, tr("Cannot load %1.").arg(fileName) );
  }
  else {
    conf->savePath( "ImagePath", fileName );
    this->setWindowTitle( fileName );
    statusImageInf->setText ( im.getShortImageInfo() );
    cur_file_name = fileName;
    cur_page = (page_to_load < 0) ? 0 : page_to_load;

    if (page_to_load < 0)
      initNewImage();
    
    // here we will have to modify slightly later to make thumb always with maximum num of channels
    // even if it's a page
    controls->onPageChanged( cur_page );
    controls->setThumbnail( im.getThumbnail(thumb_size), im.imageWidth(), im.imageHeight() );

    messager->sendPageChanged( cur_page );
    messager->sendThumbnailChanged( im.getThumbnail(client_thumb_size) );

    updateSlidersByPage(cur_page);
  
    operationStop(); 

    if ( prev_image_size != im.imageSize() ) {
      //fit image to the screen and force repaint
      im.zoomSet( im.zoomClosestSmaller( imageScroll->viewScreen().size().toSize() ) );
      imageScroll->setVirtualViewSize( im.viewSize(), imageScroll->viewCenter() );
    }
    else imageScroll->repaintView();

    prev_image_size = im.imageSize();
  }
  
}

void TileViewer::initScaleBar() {
  if (!scaleBar) return;
  scaleBar->setScale( im.viewPixelSizeX(), "um" );

  if ( !scaleBar->isEmpty() && conf->getValue( "ShowScaleBar", true ).toBool() )  
    scaleBar->showBasedOn( imageScroll->viewport(), imageScroll->pos() );     
  else
    scaleBar->hide();
}

void TileViewer::initMetadataBar() {
  if (!metadataBar) return;
  metadataBar->setMetadata( im.getMetaText() );
  //#if !defined WV_EMBEDDED_CONTROLS
  if ( !metadataBar->isEmpty() && conf->getValue( "ShowMetadataBar", false ).toBool() ) {
    metadataBar->showBasedOn( imageScroll->viewport(), imageScroll->pos() );
    metadataBar->show();
  } else
    metadataBar->hide();
  //#else
  //metadataBar->hide();
  //#endif
}

void TileViewer::initSlidersZT() {
  horizontalSlider->setMinimum( 0 );
  horizontalSlider->setMaximum( im.imageNumT()-1 );
  horizontalSlider->setSingleStep( 1 );
  horizontalSlider->setVisible( im.imageNumT()>1 );
  horizontalSlider->setValue( 0 );


  verticalSlider->setMinimum( 0 );
  verticalSlider->setMaximum( im.imageNumZ()-1 );
  verticalSlider->setSingleStep( 1 );
  verticalSlider->setVisible( im.imageNumZ()>1 );
  verticalSlider->setValue( 0 );
}

void TileViewer::updateSlidersByPage(int p) {
  int nz = std::max<int>(im.imageNumZ(), 1);
  int nt = std::max<int>(im.imageNumT(), 1);
  int t = p/nz;
  int z = p - t*nz;
  verticalSlider->setValue( z );
  horizontalSlider->setValue( t );
}

void TileViewer::initNewImageControls() {
  //---------------------------------------------------
  // send data to controls window
  //---------------------------------------------------
  controls->initChannels( im.channelNames(), 
                          im.channelMapping( WVImageManager::dcRed ), 
                          im.channelMapping( WVImageManager::dcGreen ), 
                          im.channelMapping( WVImageManager::dcBlue ) );
  controls->initEnhancement( im.enhancementTypes(), im.currentEnhancementIndex() );
  //controls->setMetadata( im.getMetaText() );
  controls->setMetadata( im.getMetaData() );
  controls->setPages( im.imageNumPages() );

  initSlidersZT();
  initScaleBar();
  initMetadataBar();   
}

void TileViewer::initNewImageClient() {
  //---------------------------------------------------
  // send data to the client if connected
  //---------------------------------------------------
  if ( !server.isConnected() ) return;
  messager->sendChannelsListChanged( im.channelNames() );

  messager->sendRedChanged( im.channelMapping( WVImageManager::dcRed ) );
  messager->sendGreenChanged( im.channelMapping( WVImageManager::dcGreen ) );
  messager->sendBlueChanged( im.channelMapping( WVImageManager::dcBlue ) );
  
  messager->sendEnhancementListChanged( im.enhancementTypes() );
  messager->sendEnhancementChanged( im.currentEnhancementIndex() );

  messager->sendMetadataChanged( im.getMetaText() );
  messager->sendNumPagesChanged( im.imageNumPages() );

  // send the thumbnail of the image and also the original size of the image
  messager->sendImageSizeChanged( QSize( im.imageWidth(), im.imageHeight() ) );
  messager->sendImageInfoText( im.getShortImageInfo() );
  //messager->sendVisibleAreaChanged( im.viewToImage( imageScroll->viewArea() ) );
  //messager->sendScaleChanged( im.currentViewScale() );

  if (!im.pixelSizeEmpty()) {
    messager->sendPhysPixelSizeX( im.viewPixelSizeX(1.0) );
    messager->sendPhysPixelSizeUnit( "um" );
  }
}

void TileViewer::initNewImage() {
  
  initNewImageControls();
  initNewImageClient();
}

void TileViewer::open() {
  QString path = conf->loadPath( "ImagePath" );

  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), path, WVImageManager::dialogFilters() );
  if (!fileName.isEmpty()) {
    loadImage( fileName );
    conf->savePath( "ImagePath", fileName );
  }
}

void TileViewer::saveDisplay() {
  QString path = conf->loadPath( "ImagePath" );
  path += ".display.png";
  QString selectedFilter;
  //QList<QByteArray> fmts = QImageWriter::supportedImageFormats(); 

  /*
  TDimFormatManager fm; 
  std::string allext = fm.getAllExtensions();
  QString qall( "All images (*." );
  qall += allext.c_str();
  qall.replace(QString("|"), QString(" *."));
  qall += ");;";
  
  std::string str = fm.getQtFilters();
  QString qstr( str.c_str() );
  qstr.replace(QString("|"), QString(" *."));
  qstr += "All files (*.*)";
  return qall+qstr;
  */

  //QString fileName = QFileDialog::getSaveFileName(0, tr("Save Display Image"), path, WVImageManager::dialogFilters(), &selectedFilter );

  QString fileName = QFileDialog::getSaveFileName(0, tr("Save Display Image"), path, "Portable Network Graphics (*.png)", &selectedFilter );
  if (!fileName.isEmpty()) {
    if ( !fileName.toLower().endsWith(".png") ) fileName += ".png";
    QImage img = renderDisplay();
    img.save(fileName, "PNG");
  }

}

void TileViewer::saveDisplayClipboard() {
  QImage img = renderDisplay();
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setImage( img, QClipboard::Clipboard );
}

void TileViewer::printDisplay() {

  QPrintDialog printDialog(&printer, this);
  if (printDialog.exec() != QDialog::Accepted) return;

  QPainter painter;
  painter.begin(&printer);
  painter.save();

  QImage img = renderDisplay();

  double xscale = printer.pageRect().width()/double(img.width());
  double yscale = printer.pageRect().height()/double(img.height());
  double scale = qMin(xscale, yscale);

  painter.translate(printer.paperRect().x() + printer.pageRect().width()/2,
                   printer.paperRect().y() + printer.pageRect().height()/2);
  painter.scale(scale, scale);
  painter.translate(-img.width()/2, -img.height()/2);

  painter.drawImage( QPoint(0,0), img );

  painter.restore();
  painter.end();
}

void TileViewer::centerViewAtImagePoint( const QPointF &cent, bool force_full_repaint ) {
  operationInterruptPrevious();
  imageScroll->centerViewAt( im.imageToView( cent ), force_full_repaint );
}

void TileViewer::changeZoom( int z, bool z_is_absolute, const QPointF &center ) {
  bool res = false;

  // store view center point in image coordinats
  QPointF cent = im.viewToImage( imageScroll->viewCenter() );
  if ( center.x()!=-1 && center.y()!=-1 )
    cent = im.viewToImage( center );
  
  if (!z_is_absolute) {
    if (z < 0) res = im.zoomOut();
    else
    if (z > 0) res = im.zoomIn();
    else
      res = im.zoom11(); 
  }
  else
    res = im.zoomSet( z );

  if (res) {
    imageScroll->setVirtualViewSize( im.viewSize(), im.imageToView(cent) );

    if (scaleBar && !scaleBar->isEmpty() && conf->getValue( "ShowScaleBar", true ).toBool() )
      scaleBar->setScale( im.viewPixelSizeX(), "um" );
  }
}

void TileViewer::fitToWindow() {
  int z = im.zoomClosestSmaller( imageScroll->viewScreen().size().toSize() );
  changeZoom( z, true );
}

void TileViewer::fullScreen() {
  if ( windowState() != Qt::WindowFullScreen ) {
    setWindowState(Qt::WindowFullScreen);
    statusBar()->hide();
    //imageScroll->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    //imageScroll->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
  } else {
    setWindowState(Qt::WindowNoState);
    statusBar()->show();
    imageScroll->setHorizontalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
    imageScroll->setVerticalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
  }
}

void TileViewer::fullVirtualScreen() {
  
  if ( !full_virtual_screen ) {
    conf->saveWidgetConfig( this );

    //setWindowState(Qt::WindowFullScreen);
    //statusBar()->hide();

    //imageScroll->setHorizontalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    //imageScroll->setVerticalScrollBarPolicy ( Qt::ScrollBarAlwaysOff );
    QRect virtual_geom = DSysConfig::virtualScreenMaxGeometry();
    move( virtual_geom.topLeft() );
    QSize s = virtual_geom.size();
    s.setWidth( s.width() - (frameGeometry().width()-s.width()) );
    s.setHeight( s.height() - (frameGeometry().height()-s.height()) );
    resize( s );
    
    full_virtual_screen = true;
  } else {
    setWindowState(Qt::WindowNoState);
    conf->loadWidgetConfig( this );
    statusBar()->show();
    imageScroll->setHorizontalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
    imageScroll->setVerticalScrollBarPolicy ( Qt::ScrollBarAsNeeded );
    full_virtual_screen = false;
  }

}

void TileViewer::about() {
  QString msg;
  msg.sprintf( "<b>bioView ver: %s</b>, 2005-2010<br />", WV_VERSION );
  msg += "Center for BioImage Informatics, UCSB<br />";
  msg += "www.bioimage.ucsb.edu<br />";
  msg += "Author: Dima Fedorov<br />";
  msg += "Graphical annotations: Jae Hyeok Choi";
  QMessageBox::about(this, tr("About bioView"), msg);
}

void TileViewer::formats() {
  TDimFormatManager fm;
  std::string str = fm.getAllFormatsHTML();

  QString msg;
  msg += "<h3>bioView supported formats:</h3>";
  msg += str.c_str();
  QMessageBox::about(0, tr("Formats supported in bioView"), msg);
}

void TileViewer::help() {
#ifndef WV_EMBEDDED_CONTROLS
  QString docs = "file:///" + DSysConfig::applicationPath() + "/doc/bioview.html"; 
#else
  QString docs = "file:///" + DSysConfig::applicationPath() + "/doc/graphical_annotations.html"; 
#endif
  QDesktopServices::openUrl(docs);
}

void TileViewer::showControls() {
  if (!controls) return;
  
  if (controls->isVisible()) {
    #ifndef WV_EMBEDDED_CONTROLS
    controls->hide();
    #endif
  } else {
    controls->show();
  }
  
  controlsAct->setChecked( controls->isVisible() );
}

void TileViewer::trigShowScale() {
  if (!scaleBar) return;
  bool s = !conf->getValue( "ShowScaleBar", true ).toBool();
  conf->setValue( "ShowScaleBar", s );
  
  showScaleBarAct->setChecked( s );
  if (scaleBar->isEmpty()) s = false;  

  if (s)
    scaleBar->showBasedOn( imageScroll->viewport(), imageScroll->pos() );
  else 
    scaleBar->hide();
}

void TileViewer::trigShowMeta() {
  if (!metadataBar) return;
  bool s = !conf->getValue( "ShowMetadataBar", false ).toBool(); 
  conf->setValue( "ShowMetadataBar", s );

  showMetadataAct->setChecked( s );
  if (metadataBar->isEmpty()) s = false;

  if (s)
    metadataBar->showBasedOn( imageScroll->viewport(), imageScroll->pos() );
  else 
    metadataBar->hide();
}

void TileViewer::serverStart() {
  int port = conf->getValue( "ServerPort", 0 ).toInt();
  if (!server.listen( QHostAddress::Any, port ))
    operationMessage( mttWarning, tr("Unable to start the server: %1.").arg(server.server()->errorString()) );
  else {
    serverStartAct->setEnabled(false);
    serverStopAct->setEnabled(true);
    operationMessage( mttInfo, tr("Server started on port: %1").arg(port) );
  }
}

void TileViewer::serverStop() {
  server.stop();
  serverStartAct->setEnabled(true);
  serverStopAct->setEnabled(false);
  operationMessage( mttInfo, "Server stoped..." );
}

void TileViewer::serverDisconnect() {
  server.disconnect();
}

void TileViewer::onClientConnected() {
  serverDisconnectAct->setEnabled(true);

  QString s;
  s += "Connected client from: ";
  s += server.socket()->peerName();
  s += " ";
  s += server.socket()->peerAddress().toString();
  s += tr(":%1").arg( server.socket()->peerPort() );
  operationMessage( mttInfo, s );

  initNewImageClient();
}

void TileViewer::onClientDisconnected() {
  serverDisconnectAct->setEnabled(false);
  operationMessage( mttInfo, "Client disconnected..." );
}

void TileViewer::useLargeCursors() {
  useLargeCursorsAct->setChecked( useLargeCursorsAct->isChecked() );
  imageScroll->setUseLargeCursors( useLargeCursorsAct->isChecked() );
  conf->setValue( "UseLargeCursors", useLargeCursorsAct->isChecked() );
}

void TileViewer::useWheelToPan() {
  useWheelToPanAct->setChecked( useWheelToPanAct->isChecked() );
  if ( useWheelToPanAct->isChecked() )
    imageScroll->setMouseScrollTool( DScrollImageView::mstPan );
  else
    imageScroll->setMouseScrollTool( DScrollImageView::mstZoom );
  conf->setValue( "UseWheelToPan", useWheelToPanAct->isChecked() );
}

void TileViewer::useSmoothZoom() {
  useSmoothZoomAct->setChecked( useSmoothZoomAct->isChecked() );
  if ( useSmoothZoomAct->isChecked() )
    im.setZoomMethod( WVImageManager::dzmBL );
  else
    im.setZoomMethod( WVImageManager::dzmNN );
  imageScroll->repaintView();
  conf->setValue( "UseSmoothZoom", useSmoothZoomAct->isChecked() );
  messager->sendShowSmoothZoom( useSmoothZoomAct->isChecked() );
}

void TileViewer::trigSmoothZoom() {
  useSmoothZoomAct->setChecked( !useSmoothZoomAct->isChecked() );
  useSmoothZoom();
}

void TileViewer::useDynamicEnhancement() {
  useDynamicEnhanceAct->setChecked( useDynamicEnhanceAct->isChecked() );
  if ( useDynamicEnhanceAct->isChecked() )
    im.setEnhancemetArea( WVImageManager::deaOnlyVisible );
  else
    im.setEnhancemetArea( WVImageManager::deaFullImage );
  imageScroll->repaintView();
  conf->setValue( "UseDynamicEnhancement", useDynamicEnhanceAct->isChecked() );
  messager->sendShowDynEnhance( useDynamicEnhanceAct->isChecked() );
}

void TileViewer::centerSmallImages() {
  centerSmallImagesAct->setChecked( centerSmallImagesAct->isChecked() );
  imageScroll->setCenterSmallImages( centerSmallImagesAct->isChecked() );
  conf->setValue( "CenterSmallImages", centerSmallImagesAct->isChecked() );
}

void TileViewer::trigDynamicEnhancement() {
  useDynamicEnhanceAct->setChecked( !useDynamicEnhanceAct->isChecked() );
  useDynamicEnhancement();
}

void TileViewer::createActions() {

  openAct = new QAction(tr("&Open..."), this);
  openAct->setShortcut(tr("Ctrl+O"));
  connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

  openFromBisqikAct = new QAction(tr("&Open from Bisquik..."), this);
  connect(openFromBisqikAct, SIGNAL(triggered()), this, SLOT(openBisquik()));

  configDownloadPathAct = new QAction(tr("&Configure download path..."), this);
  connect(configDownloadPathAct, SIGNAL(triggered()), this, SLOT(configDownload()));

  saveDisplayAct = new QAction(tr("&Save Display..."), this);
  saveDisplayAct->setShortcut(tr("Ctrl+D"));
  connect(saveDisplayAct, SIGNAL(triggered()), this, SLOT(saveDisplay()));

  saveDisplayClipboardAct = new QAction(tr("&Save Display to clipboard"), this);
  //saveDisplayClipboardAct->setShortcut(tr("Ctrl+D"));
  connect(saveDisplayClipboardAct, SIGNAL(triggered()), this, SLOT(saveDisplayClipboard()));

  printDisplayAct = new QAction(tr("&Print Display..."), this);
  printDisplayAct->setShortcut(tr("Ctrl+P"));
  connect(printDisplayAct, SIGNAL(triggered()), this, SLOT(printDisplay()));

  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setShortcut(tr("Ctrl+Q"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

  #ifdef WV_DISABLE_STANDALONE_MENUS
  saveAct = new QAction(tr("&Save"), this);
  saveAct->setShortcut(tr("Ctrl+S"));
  connect(saveAct, SIGNAL(triggered()), this, SLOT(onSaveBtnClicked()));

  reloadAct = new QAction(tr("&Reload"), this);
  reloadAct->setShortcut(tr("Ctrl+R"));
  connect(reloadAct, SIGNAL(triggered()), this, SLOT(onReloadBtnClicked()));
  #endif

  zoomInAct = new QAction(tr("Zoom &In (50%)"), this);
  zoomInAct->setShortcut(tr("Ctrl++"));
  zoomInAct->setEnabled(true);
  connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

  zoomOutAct = new QAction(tr("Zoom &Out (50%)"), this);
  zoomOutAct->setShortcut(tr("Ctrl+-"));
  zoomOutAct->setEnabled(true);
  connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

  normalSizeAct = new QAction(tr("&Normal Size"), this);
  normalSizeAct->setShortcut(tr("Ctrl+N"));
  normalSizeAct->setEnabled(true);
  connect(normalSizeAct, SIGNAL(triggered()), this, SLOT(normalSize()));

  fitToWindowAct = new QAction(tr("Fit to &Window"), this);
  fitToWindowAct->setEnabled(true);
  fitToWindowAct->setCheckable(false);
  fitToWindowAct->setShortcut(tr("Ctrl+W"));
  connect(fitToWindowAct, SIGNAL(triggered()), this, SLOT(fitToWindow()));

  fullScreenAct = new QAction(tr("&Full screen"), this);
  fullScreenAct->setEnabled(true);
  fullScreenAct->setCheckable(false);
  fullScreenAct->setShortcut(tr("Ctrl+F"));
  connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(fullScreen()));

  fullVirtualScreenAct = new QAction(tr("Full &virtual screen"), this);
  fullVirtualScreenAct->setEnabled(true);
  fullVirtualScreenAct->setCheckable(false);
  fullVirtualScreenAct->setShortcut(tr("Ctrl+V"));
  connect(fullVirtualScreenAct, SIGNAL(triggered()), this, SLOT(fullVirtualScreen()));

  useLargeCursorsAct = new QAction(tr("Use large cursors"), this);
  useLargeCursorsAct->setEnabled(true);
  useLargeCursorsAct->setCheckable(true);
  useLargeCursorsAct->setChecked( false );
  connect(useLargeCursorsAct, SIGNAL(triggered()), this, SLOT(useLargeCursors()));

  useWheelToPanAct = new QAction(tr("Use Scroll Wheel to pan"), this);
  useWheelToPanAct->setEnabled(true);
  useWheelToPanAct->setCheckable(true);
  useWheelToPanAct->setChecked( false );
  connect(useWheelToPanAct, SIGNAL(triggered()), this, SLOT(useWheelToPan()));

  useSmoothZoomAct = new QAction(tr("Use smooth zoom"), this);
  useSmoothZoomAct->setEnabled(true);
  useSmoothZoomAct->setCheckable(true);
  useSmoothZoomAct->setChecked( false );
  connect(useSmoothZoomAct, SIGNAL(triggered()), this, SLOT(useSmoothZoom()));

  useDynamicEnhanceAct = new QAction(tr("Use dynamic enhancement (Only visible area)"), this);
  useDynamicEnhanceAct->setEnabled(true);
  useDynamicEnhanceAct->setCheckable(true);
  useDynamicEnhanceAct->setChecked( false );
  connect(useDynamicEnhanceAct, SIGNAL(triggered()), this, SLOT(useDynamicEnhancement()));

  centerSmallImagesAct = new QAction(tr("Center small images"), this);
  centerSmallImagesAct->setEnabled(true);
  centerSmallImagesAct->setCheckable(true);
  centerSmallImagesAct->setChecked( true );
  connect( centerSmallImagesAct, SIGNAL(triggered()), this, SLOT(centerSmallImages()) );

  rotate90Act = new QAction(tr("Rotate &Right"), this);
  //rotate90Act->setShortcut(tr("Ctrl+R"));
  rotate90Act->setEnabled(true);
  connect(rotate90Act, SIGNAL(triggered()), this, SLOT(onRotate90()));

  rotate270Act = new QAction(tr("Rotate &Left"), this);
  //rotate270Act->setShortcut(tr("Ctrl+L"));
  rotate270Act->setEnabled(true);
  connect(rotate270Act, SIGNAL(triggered()), this, SLOT(onRotate270()));

  rotate180Act = new QAction(tr("Rotate 180 deg"), this);
  //rotate180Act->setShortcut(tr("Ctrl+I"));
  rotate180Act->setEnabled(true);
  connect(rotate180Act, SIGNAL(triggered()), this, SLOT(onRotate180()));

  projectMaxAct = new QAction(tr("Project all planes by maximum intesity"), this);
  projectMaxAct->setEnabled(true);
  connect(projectMaxAct, SIGNAL(triggered()), this, SLOT(onProjectMax()));

  projectMinAct = new QAction(tr("Project all planes by minimum intesity"), this);
  projectMinAct->setEnabled(true);
  connect(projectMinAct, SIGNAL(triggered()), this, SLOT(onProjectMin()));


  serverStartAct = new QAction(tr("&Start server"), this);
  serverStartAct->setEnabled(true);
  serverStartAct->setCheckable(false);
  //serverStartAct->setShortcut(tr("Ctrl+F"));
  connect(serverStartAct, SIGNAL(triggered()), this, SLOT(serverStart()));

  serverStopAct = new QAction(tr("&Stop server"), this);
  serverStopAct->setEnabled(false);
  serverStopAct->setCheckable(false);
  //serverStopAct->setShortcut(tr("Ctrl+F"));
  connect(serverStopAct, SIGNAL(triggered()), this, SLOT(serverStop()));

  serverDisconnectAct = new QAction(tr("&Disconnect client"), this);
  serverDisconnectAct->setEnabled(false);
  serverDisconnectAct->setCheckable(false);
  //serverDisconnectAct->setShortcut(tr("Ctrl+F"));
  connect(serverDisconnectAct, SIGNAL(triggered()), this, SLOT(serverDisconnect()));

  controlsAct = new QAction(tr("Controls"), this);
  controlsAct->setEnabled(true);
  controlsAct->setCheckable(true);
  //controlsAct->setShortcut(tr("Ctrl+"));
  connect(controlsAct, SIGNAL(triggered()), this, SLOT(showControls()));

  showScaleBarAct = new QAction(tr("Scale bar"), this);
  showScaleBarAct->setEnabled(true);
  showScaleBarAct->setCheckable(true);
  showScaleBarAct->setChecked( conf->getValue( "ShowScaleBar", true ).toBool() );
  connect(showScaleBarAct, SIGNAL(triggered()), this, SLOT(trigShowScale()));

  showMetadataAct = new QAction(tr("Metadata bar"), this);
  showMetadataAct->setEnabled(true);
  showMetadataAct->setCheckable(true);
  showMetadataAct->setChecked( conf->getValue( "ShowMetadataBar", false ).toBool() );
  connect(showMetadataAct, SIGNAL(triggered()), this, SLOT(trigShowMeta()));

  helpAct = new QAction(tr("&Help"), this);
  connect(helpAct, SIGNAL(triggered()), this, SLOT(help()));


  aboutAct = new QAction(tr("&About"), this);
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  formatsAct = new QAction(tr("&Formats"), this);
  connect(formatsAct, SIGNAL(triggered()), this, SLOT(formats()));

  //aboutQtAct = new QAction(tr("About &Qt"), this);
  //connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
}

void TileViewer::createMenus()
{
    fileMenu = new QMenu(tr("&File"), this);
    #ifndef WV_DISABLE_STANDALONE_MENUS
    fileMenu->addAction(openAct);
    fileMenu->addAction(openFromBisqikAct);
    fileMenu->addAction(configDownloadPathAct);
    fileMenu->addSeparator();
    #endif
    fileMenu->addAction(saveDisplayAct);
    fileMenu->addAction(saveDisplayClipboardAct);
    fileMenu->addAction(printDisplayAct);

    #ifdef WV_DISABLE_STANDALONE_MENUS
    fileMenu->addSeparator();
    fileMenu->addAction(saveAct);
    fileMenu->addAction(reloadAct);
    #endif

    #ifndef WV_DISABLE_STANDALONE_MENUS
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
    #endif

    viewMenu = new QMenu(tr("&View"), this);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(normalSizeAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fitToWindowAct);
    viewMenu->addSeparator();
    viewMenu->addAction(useLargeCursorsAct);
    viewMenu->addAction(useWheelToPanAct);
    viewMenu->addAction(useSmoothZoomAct);
    viewMenu->addAction(useDynamicEnhanceAct);
    viewMenu->addAction(centerSmallImagesAct);
    viewMenu->addSeparator();
    viewMenu->addAction(fullScreenAct);
    viewMenu->addAction(fullVirtualScreenAct);
    
    modifyMenu = new QMenu(tr("&Modify"), this);
    modifyMenu->addAction(rotate90Act);
    modifyMenu->addAction(rotate270Act);
    modifyMenu->addAction(rotate180Act);
    modifyMenu->addSeparator();
    modifyMenu->addAction(projectMaxAct);
    modifyMenu->addAction(projectMinAct);

    //#ifndef WV_DISABLE_STANDALONE_MENUS
    serverMenu = new QMenu(tr("&Server"), this);
    serverMenu->addAction(serverStartAct);
    serverMenu->addAction(serverStopAct);
    serverMenu->addSeparator();
    serverMenu->addAction(serverDisconnectAct);
    //#endif

    windowMenu = new QMenu(tr("&Window"), this);
    #ifndef WV_DISABLE_STANDALONE_MENUS
    windowMenu->addAction(controlsAct);
    windowMenu->addSeparator();
    #endif
    windowMenu->addAction(showMetadataAct);
    windowMenu->addAction(showScaleBarAct);

    helpMenu = new QMenu(tr("&Help"), this);
    helpMenu->addAction(helpAct);
    helpMenu->addAction(formatsAct);
    helpMenu->addSeparator();
    helpMenu->addAction(aboutAct);

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(viewMenu);
    menuBar()->addMenu(modifyMenu);
    #ifndef WV_DISABLE_STANDALONE_MENUS
    menuBar()->addMenu(serverMenu);
    #endif
    menuBar()->addMenu(windowMenu);
    menuBar()->addMenu(helpMenu);
}

void TileViewer::createStatusBar() {
  status_bar = new QStatusBar(this);
  statusProgress = NULL;

  statusImageInf = new QLabel(status_bar);
  statusPosition = new QLabel(status_bar);
  statusMessages = new QLabel(status_bar);

  statusImageInf->setText ( im.getShortImageInfo() );
  statusPosition->setText ("[0x0]");

  status_bar->addWidget ( statusImageInf );
  status_bar->addWidget ( statusPosition );
  status_bar->addWidget ( statusMessages );

  statusImageInf->setMinimumWidth( 180 );
  statusPosition->setMinimumWidth( 200 );
  statusMessages->setMinimumWidth( 200 );

  setStatusBar(status_bar);
}

void TileViewer::updateActions()
{
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void TileViewer::updateVisArea( const QRectF &area ) {

  controls->onViewChanged( im.currentViewScale(), im.viewToImage(area) );
  messager->sendVisibleAreaChanged( im.viewToImage(area) );
  messager->sendScaleChanged( im.currentViewScale() );
  prev_view_center = imageScroll->viewCenter();
  #ifdef WV_GR_LAYER_SUPPORT
  if (gview) gview->updateViewCenter(imageScroll->viewCenter());
  #endif //WV_GR_LAYER_SUPPORT
}

void TileViewer::updateView() {
  imageScroll->repaintView();
  controls->setThumbnail( im.getThumbnail(thumb_size), im.imageWidth(), im.imageHeight() );
  updateVisArea( imageScroll->viewArea() );
}

void TileViewer::onChannelsChanged(const int &r, const int &g, const int &b, const int &y, const int &m, const int &c) {
  im.setMapping( WVImageManager::dcRed, r );
  im.setMapping( WVImageManager::dcGreen, g );
  im.setMapping( WVImageManager::dcBlue, b );
  im.setMapping( WVImageManager::dcYellow, y );
  im.setMapping( WVImageManager::dcPurple, m );
  im.setMapping( WVImageManager::dcCyan, c );
  updateView();
}

void TileViewer::onRepaintRequired() {
  imageScroll->repaintView();
}

void TileViewer::onEnhancementChanged(int e) {
  im.setCurrentEnhancement((WVImageManager::DisplayLiveEnhancemet) e);
  controls->setEnhancementControlsWidget( im.currentEnhancement()->widget() );
  updateView();
}

void TileViewer::onZoomChanged(int z) {
  changeZoom( z );
}

void TileViewer::onPageChanged(int p) {
  loadImage( cur_file_name, p );
}

void TileViewer::onSliderT( int v ) {
  if (in_operation) return;
  int page = ( im.imageNumZ()*horizontalSlider->value() ) + verticalSlider->value();
  onPageChanged(page);
}

void TileViewer::onSliderZ( int v ) {
  if (in_operation) return;
  int page = ( im.imageNumZ()*horizontalSlider->value() ) + verticalSlider->value();
  onPageChanged(page);
}

void TileViewer::onVisCenterChanged(const QPointF &center) {
  centerViewAtImagePoint( center );
}

void TileViewer::onVisCenterSelected(const QPointF &center) {
  imageScroll->repaintView();
}

void TileViewer::onVisCursorMoved( const QPoint &vp ) {
 
  QPoint p = im.viewToImage( vp ).toPoint();

  if (im.viewPixelSizeX() == 0)
    statusPosition->setText( tr("[%1x%2]").arg(p.x()).arg(p.y())  );
  else {
    QPointF pf = p;
    pf.setX( pf.x() * im.viewPixelSizeX(1.0) );
    pf.setY( pf.y() * im.viewPixelSizeY(1.0) );
    QString str;
    str.sprintf( "[%dx%d]px [%.4fx%.4f]um", p.x(), p.y(), pf.x(), pf.y() );
    //str = tr("[%1x%2]px [%3x%4]um").arg(p.x()).arg(p.y()).arg(pf.x()).arg(pf.y()); 
    statusPosition->setText( str );
  }

  #ifdef WV_GR_LAYER_SUPPORT
  if (imageScroll->mouseTool() == DScrollImageView::mtZoom) return;
  if (!gview) return;
  QString tool_str = gview->getCurrentToolState( im.viewPixelSizeX(1.0), im.viewPixelSizeY(1.0) );
  if (!tool_str.isEmpty()) statusPosition->setText( tool_str );
  #endif //WV_GR_LAYER_SUPPORT
}

void TileViewer::closeEvent ( QCloseEvent * ) {
  #ifndef WV_EMBEDDED_CONTROLS
  qApp->quit();
  #endif
}

void TileViewer::resizeEvent(QResizeEvent *) {
  if ( scaleBar && scaleBar->isVisible() )
    scaleBar->showBasedOn( imageScroll->viewport(), imageScroll->pos() );

  if ( metadataBar && metadataBar->isVisible() )
    metadataBar->showBasedOn( imageScroll->viewport(), imageScroll->pos() );
}

//-----------------------------------------
// network messages
//-----------------------------------------

//-------------------------------------------------------------------------------
// internals
//-------------------------------------------------------------------------------

void TileViewer::initMessager() {
  
  connect( messager,  SIGNAL( localFileName(const QString &) ), 
           this,        SLOT( nmLocalFileName(const QString &) ));

  connect( messager,  SIGNAL( scaleChanged(const double &) ), 
           this,        SLOT( nmScaleChanged(const double &) ));

  connect( messager, SIGNAL( redChanged( const qint32 &) ), 
           this,       SLOT( nmRedChanged(const qint32 &) ));

  connect( messager, SIGNAL( greenChanged( const qint32 &) ), 
           this,       SLOT( nmGreenChanged(const qint32 &) ));

  connect( messager, SIGNAL( blueChanged( const qint32 &) ), 
           this,       SLOT( nmBlueChanged(const qint32 &) ));

  connect( messager, SIGNAL( yellowChanged( const qint32 &) ), 
           this,       SLOT( nmYellowChanged(const qint32 &) ));

  connect( messager, SIGNAL( magentaChanged( const qint32 &) ), 
           this,       SLOT( nmMagentaChanged(const qint32 &) ));

  connect( messager, SIGNAL( cyanChanged( const qint32 &) ), 
           this,       SLOT( nmCyanChanged(const qint32 &) ));

  connect( messager, SIGNAL( viewChanChanged( const QColor &) ), 
           this,       SLOT( nmViewChanChanged(const QColor &) ));

  connect( messager, SIGNAL( viewChannelsChanged( const QList<QVariant> &) ), 
           this,       SLOT( nmViewChannelsChanged(const QList<QVariant> &) ));

  connect( messager, SIGNAL( enhancementChanged( const qint32 &) ), 
           this,       SLOT( nmEnhancementChanged(const qint32 &) ));

  connect( messager, SIGNAL( pageChanged( const qint32 &) ), 
           this,       SLOT( nmPageChanged(const qint32 &) ));

  connect( messager, SIGNAL( thumbSizeChanged(const QSize &) ), 
           this,       SLOT( nmThumbSizeChanged(const QSize &) ));

  connect( messager, SIGNAL( positionChanged(const QPointF &) ), 
           this,       SLOT( nmPositionChanged(const QPointF &) ));

  connect( messager, SIGNAL( fullVirtualScreen() ), 
           this,       SLOT( nmFullVirtualScreen() ));

  connect( messager, SIGNAL( closeWall() ), 
           this,       SLOT( nmCloseWall() ));

  connect( messager, SIGNAL( cursorPositionChanged(const QPointF &) ), 
           this,       SLOT( nmCursorPosition(const QPointF &) ));

  connect( messager, SIGNAL( showScaleBarChanged(bool) ), 
          this,       SLOT( nmShowScaleBar(bool) ));

  connect( messager, SIGNAL( showMetadataChanged(bool) ), 
          this,       SLOT( nmShowMetadata(bool) ));

  connect( messager, SIGNAL( rotateRight() ), 
           this,       SLOT( onRotate90() ));

  connect( messager, SIGNAL( rotateLeft() ), 
           this,       SLOT( onRotate270() ));

  connect( messager, SIGNAL( rotate180() ), 
           this,       SLOT( onRotate180() ));

  connect( messager, SIGNAL( trigSmoothZoom() ), 
           this,       SLOT( trigSmoothZoom() ));

  connect( messager, SIGNAL( trigDynEnhance() ), 
           this,       SLOT( trigDynamicEnhancement() ));

  connect( messager, SIGNAL( zoomIn( const QPointF &) ), 
           this,       SLOT( nmZoomIn( const QPointF &) ));

  connect( messager, SIGNAL( zoomOut( const QPointF &) ), 
           this,       SLOT( nmZoomOut( const QPointF &) ));

  connect( messager, SIGNAL( projectMax() ), 
           this,       SLOT( onProjectMax() ));

  connect( messager, SIGNAL( projectMin() ), 
           this,       SLOT( onProjectMin() ));

  connect( messager, SIGNAL( fileBlob( const QString &, const QByteArray &) ), 
           this,       SLOT( nmFileBlob(const QString &, const QByteArray &) ));

}

void TileViewer::nmLocalFileName( const QString &fileName ) {
  if (!fileName.isEmpty()) {
    loadImage( fileName );
    //conf->savePath( "ImagePath", fileName );
  }
}

void TileViewer::nmScaleChanged( const double &v ) {
  changeZoom( v, false );
}

void TileViewer::nmPositionChanged( const QPointF &v ) {
  centerViewAtImagePoint( v );
}

void TileViewer::nmRedChanged( const qint32 &v ) {
  im.setMapping( WVImageManager::dcRed, v );
  updateView();
}

void TileViewer::nmGreenChanged( const qint32 &v ) {
  im.setMapping( WVImageManager::dcGreen, v );
  updateView();
}

void TileViewer::nmBlueChanged( const qint32 &v ) {
  im.setMapping( WVImageManager::dcBlue, v );
  updateView();
}

void TileViewer::nmYellowChanged( const qint32 &v ) {
  im.setMapping( WVImageManager::dcYellow, v );
  updateView();
}

void TileViewer::nmMagentaChanged( const qint32 &v ) {
  im.setMapping( WVImageManager::dcPurple, v );
  updateView();
}

void TileViewer::nmCyanChanged( const qint32 &v ) {
  im.setMapping( WVImageManager::dcCyan, v );
  updateView();
}

void TileViewer::nmViewChanChanged( const QColor &v ) {
  im.setMapping( WVImageManager::dcRed, v.red() );  
  im.setMapping( WVImageManager::dcGreen, v.green() );
  im.setMapping( WVImageManager::dcBlue, v.blue() );
  updateView();
}

void TileViewer::nmViewChannelsChanged( const QList<QVariant> &v ) {
  if (v.size()>=6) {
    im.setMapping( WVImageManager::dcRed,     v[0].toInt() );  
    im.setMapping( WVImageManager::dcGreen,   v[1].toInt() );
    im.setMapping( WVImageManager::dcBlue,    v[2].toInt() );
    im.setMapping( WVImageManager::dcYellow,  v[3].toInt() );  
    im.setMapping( WVImageManager::dcPurple,  v[4].toInt() );
    im.setMapping( WVImageManager::dcCyan,    v[5].toInt() );
  }
  updateView();
}

void TileViewer::nmEnhancementChanged( const qint32 &v ) {
  onEnhancementChanged(v);
}

void TileViewer::nmPageChanged( const qint32 &v ) {
  onPageChanged(v);
}

void TileViewer::nmThumbSizeChanged( const QSize &v ) {
  client_thumb_size = v;

  if (im.imageEmpty()) return;
  messager->sendThumbnailChanged( im.getThumbnail(client_thumb_size) );
  messager->sendVisibleAreaChanged( im.viewToImage( imageScroll->viewArea() ) );
  messager->sendScaleChanged( im.currentViewScale() );
  messager->sendImageInfoText( im.getShortImageInfo() );
}

void TileViewer::nmFullVirtualScreen( ) {
  fullVirtualScreen();
}

void TileViewer::nmCloseWall( ) {
  if ( conf->getValue( "AllowRemoteClose", false ).toBool() )
    qApp->quit();  
}

void TileViewer::nmShowScaleBar( bool v ) {

  conf->setValue( "ShowScaleBar", !v );
  trigShowScale();
}

void TileViewer::nmShowMetadata( bool v ) {

  conf->setValue( "ShowMetadataBar", !v );
  trigShowMeta();
}

void TileViewer::nmCursorPosition( const QPointF &v ) {
  if ( !conf->getValue( "TrackClientCursor", false ).toBool() ) return;
  QPointF p = im.imageToView( v ) - imageScroll->viewOffset() + imageScroll->centeringOffset();
  QPoint gp = imageScroll->viewport()->mapToGlobal( p.toPoint() );
  QCursor::setPos( gp );
}

void TileViewer::onOtherInstanceMessage( const QString &message ) {
     
  if ( message.startsWith( "-file ", Qt::CaseInsensitive ) ) {
    QString fileName = message.right( message.size()-6 );
    if (!fileName.isEmpty()) loadImage( fileName );
  }
}

void TileViewer::dragEnterEvent(QDragEnterEvent *event) {
 if (event->mimeData()->hasFormat("text/uri-list"))
   event->acceptProposedAction();
}

void TileViewer::dropEvent(QDropEvent *event) {
  if ( event->mimeData()->hasUrls() ) {
    QList<QUrl> l = event->mimeData()->urls();
    event->acceptProposedAction();
    loadImage( l[0].toLocalFile() );
    //conf->savePath( "ImagePath", fileName );
  }
}

void TileViewer::toolIndexChanged ( int index ) {
  imageScroll->setMouseTool( DScrollImageView::MouseTools(index) );
}

QImage TileViewer::renderDisplay() {

  QImage img;
  if (!imageScroll) return img;

  QRectF visible_area = imageScroll->viewArea();
  img = QImage::QImage( visible_area.width(), visible_area.height(), QImage::Format_RGB32 );

  QPainter p(&img);

  // render image area
  QRectF draw_rect(0, 0, visible_area.width(), visible_area.height());
  QImage *roi_img = im.getDisplayRoi( visible_area.toRect() );
  p.drawImage( draw_rect.toRect(), *roi_img, draw_rect, Qt::ColorOnly );

  // render graphical annotations
  #ifdef WV_GR_LAYER_SUPPORT
  // adjust painter matrix
  QMatrix graphicsMatrix; 
  double sx = 1.0;
  sx = pow( 2.0, im.zoomCurrent() );
  QPointF offset = imageScroll->viewOffset();
  graphicsMatrix.translate( -offset.x(), -offset.y() );
  graphicsMatrix.scale(sx, sx);
  p.setMatrix(graphicsMatrix);
  drawAnnotations( &p );
  #endif

  // render scale bar
  if (scaleBar && scaleBar->isVisible()) {
    // adjust painter matrix for scale bar position
    QMatrix m; 
    //QPointF bound( visible_area.width()-scaleBar->width()-2, visible_area.height()-scaleBar->height()-2 );
    QPointF offset = scaleBar->barOffset();
    //m.translate( dim::min<float>(offset.x(), bound.x()), dim::min<float>(offset.y(), bound.y()) );
    m.translate( offset.x(), offset.y() );
    
    // account for centering the window
    QPoint centering_offset = imageScroll->centeringOffset();
    m.translate( -centering_offset.x(), -centering_offset.y() );

    p.setMatrix(m);
    scaleBar->drawBar( &p );
  }

  return img;
}

void TileViewer::addRotation(double ang) { 
  rotation_angle += ang;  
  int n = abs(rotation_angle / 360);
  if (n>0) rotation_angle /= n;

  // set rotation to gview
#ifdef WV_GR_LAYER_SUPPORT
  //if (!gview) return;
  //gview->view_rotation_angle = rotation_angle;
  this->imageScroll->view_rotation_angle = rotation_angle;
#endif
}

void TileViewer::onRotate( double deg ) {
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  im.rotateImage(deg);
  if ( prev_image_size != im.imageSize() )
    imageScroll->setVirtualViewSize( im.viewSize(), imageScroll->viewCenter() );
  prev_image_size = im.imageSize();
  QApplication::restoreOverrideCursor();
  addRotation(deg);
  messager->sendThumbnailChanged( im.getThumbnail(client_thumb_size) );
  updateView();
}

void TileViewer::onProjectMax( ) {

  operationInterruptPrevious();
  operationStart();
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  im.projectMax();
  QApplication::restoreOverrideCursor();
  operationStop();
  messager->sendThumbnailChanged( im.getThumbnail(client_thumb_size) );
  updateView();
}

void TileViewer::onProjectMin( ) {
  operationInterruptPrevious();
  operationStart();
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  im.projectMin();
  QApplication::restoreOverrideCursor();
  operationStop();
  messager->sendThumbnailChanged( im.getThumbnail(client_thumb_size) );
  updateView();
}

void TileViewer::openBisquik() {

  //BQAccessDialog bqdialog;
  BQWebAccessDialog bqdialog;
  QString init_path = conf->loadPath( "DownloadPath" );
  bqdialog.setPath( init_path );
  if ( bqdialog.exec() ) {
    QString fileName = bqdialog.imageFileName();
    conf->savePath( "DownloadPath", bqdialog.downloadPath() );
    loadImage( fileName );
  }
}

void TileViewer::configDownload() {

  QString path = conf->loadPath( "DownloadPath" );
  path = QFileDialog::getExistingDirectory( this, tr("Open Directory"), path );
  if (!path.isEmpty())
    conf->savePath( "DownloadPath", path );
}

void TileViewer::nmFileBlob( const QString &name, const QByteArray &blob ) {
  QString path = conf->loadPath( "DownloadPath" );
  path = path + '/' + name;
  QFile f(path);
  f.open(QIODevice::WriteOnly);
  f.write( blob );
  f.close();
  loadImage( path );
}

void TileViewer::keyPressEvent ( QKeyEvent *e ) {
  switch ( e->key() ) {
     case Qt::Key_Space: onPageChanged(cur_page+1); e->accept(); break;
  } // switch
}

void TileViewer::keyReleaseEvent ( QKeyEvent *e) {
  switch ( e->key() ) {
     case Qt::Key_PageUp:   onPageChanged(cur_page-1); e->accept(); break;
     case Qt::Key_PageDown: onPageChanged(cur_page+1); e->accept(); break;
  } // switch
}

//******************************************************************************
// Graphical annotator
//******************************************************************************

void TileViewer::onApplyBtnClicked() {
#ifdef WV_GR_LAYER_SUPPORT
  QString str;
  TreeModel* treemodel_data;
  treemodel_data = gview->getTreeModel_data();
  
  treemodel_data->TreeModelToXML(&str, "gobject");
  emit dataUpdated(str, false);	
#endif
}

void TileViewer::onOkBtnClicked() {
#ifdef WV_GR_LAYER_SUPPORT  
  QString str;
  TreeModel* treemodel_data;
  treemodel_data = gview->getTreeModel_data();

  treemodel_data->TreeModelToXML(&str, "gobject");
  
  emit dataUpdated(str, false);
  QApplication::processEvents();
  this->close();  
#endif
}

void TileViewer::onSaveBtnClicked() {
#ifdef WV_GR_LAYER_SUPPORT
  QString str;
  TreeModel* treemodel_data;
  treemodel_data = gview->getTreeModel_data();
  
  treemodel_data->TreeModelToXML(&str, "gobject");
  emit dataUpdated(str, true);
#endif
}

void TileViewer::onNextBtnClicked() {
#ifdef WV_GR_LAYER_SUPPORT
  QString str;
  TreeModel* treemodel_data;
  treemodel_data = gview->getTreeModel_data();
  
  treemodel_data->TreeModelToXML(&str, "gobject");
  emit dataUpdated(str, true);
  emit loadNextRequest();
#endif
}

void TileViewer::onPrevBtnClicked() {
#ifdef WV_GR_LAYER_SUPPORT
  QString str;
  TreeModel* treemodel_data;
  treemodel_data = gview->getTreeModel_data();
  
  treemodel_data->TreeModelToXML(&str, "gobject");
  emit dataUpdated(str, true);
  emit loadPrevRequest();
#endif
}

void TileViewer::onReloadBtnClicked() {
#ifdef WV_GR_LAYER_SUPPORT
  emit reloadRequest();
#endif
}

void TileViewer::onProjectGraphicsToAllPages( bool project ) {
#ifdef WV_GR_LAYER_SUPPORT  
  gview->setProjectAllPages( project );
  this->imageScroll->repaintView();
#endif
}

void TileViewer::onCancelBtnClicked() {
  this->close();  
}

#ifdef WV_GR_LAYER_SUPPORT

void TileViewer::setGObjectData( const QString &str ) 
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  
  gview->setGObjectData(str);
	annotatorControls->initParam();

  QApplication::restoreOverrideCursor();
}

void TileViewer::setEnvironment(const QString &env)
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	gview->setEnvironment(env);
	this->imageScroll->view_rotation_angle = 0;
  QApplication::restoreOverrideCursor();
}

#endif //WV_GR_LAYER_SUPPORT

  
