/*******************************************************************************

  WVRemoteWidget is a main widget of a Remote Controller for WallViewer
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation

      
  ver: 2
        
*******************************************************************************/

#include <QtGui>
#include <QtNetwork>

#include "wvmessageparser.h"
#include "notifyWidget.h"
#include "progresscircle.h"
#include "scalebar.h"

#include "remoteWidget.h"


WVRemoteWidget::WVRemoteWidget(QWidget *parent)
: QMainWindow(parent)
{
  ui.setupUi(this);
  createStatusBar();

  in_mouse_move = false;
  cur_page=0;
  num_pages=1;
  progress_circle = NULL;
  show_scale_bar = false;
  pixel_scale = 0.0;
  thumb_ratio = 1.0;
  
  ui.exChannelsWidget->setVisible(false);

  //client.setQueueTransmition( true );
  client.setQueueTransmition( false );
  messager = new WVMessageParser( &client ); 
  connect(&client, SIGNAL( connected(QTcpSocket *) ), this, SLOT( onClientConnected() ));
  connect(&client, SIGNAL( disconnected(QTcpSocket *) ), this, SLOT( onClientDisconnected() ));
  connect(&client, SIGNAL( error(QAbstractSocket::SocketError, QTcpSocket *) ), 
          this,      SLOT( onClientError() ));

  initMessager();



  connect(ui.redCombo,     SIGNAL( activated(int) ), this, SLOT( onChannelsChanged(int) ));
  connect(ui.greenCombo,   SIGNAL( activated(int) ), this, SLOT( onChannelsChanged(int) ));
  connect(ui.blueCombo,    SIGNAL( activated(int) ), this, SLOT( onChannelsChanged(int) ));
  connect(ui.yellowCombo,  SIGNAL( activated(int) ), this, SLOT( onChannelsChanged(int) ));
  connect(ui.magentaCombo, SIGNAL( activated(int) ), this, SLOT( onChannelsChanged(int) ));
  connect(ui.cyanCombo,    SIGNAL( activated(int) ), this, SLOT( onChannelsChanged(int) ));

  connect(ui.redCheck,     SIGNAL( stateChanged(int) ), this, SLOT( onChannelsVisChanged(int) ));
  connect(ui.greenCheck,   SIGNAL( stateChanged(int) ), this, SLOT( onChannelsVisChanged(int) ));
  connect(ui.blueCheck,    SIGNAL( stateChanged(int) ), this, SLOT( onChannelsVisChanged(int) ));
  connect(ui.yellowCheck,  SIGNAL( stateChanged(int) ), this, SLOT( onChannelsVisChanged(int) ));
  connect(ui.magentaCheck, SIGNAL( stateChanged(int) ), this, SLOT( onChannelsVisChanged(int) ));
  connect(ui.cyanCheck,    SIGNAL( stateChanged(int) ), this, SLOT( onChannelsVisChanged(int) ));

  connect(ui.moreChannelsLabel, SIGNAL( linkActivated(const QString & ) ), this, SLOT( onExtChannels() ));

  connect(ui.enhanceCombo, SIGNAL( activated(int) ), this, SLOT( onEnhancementChanged(int) ));

  connect(ui.zoomOutBtn, SIGNAL( released() ), this, SLOT( onZoomOutReleased() ));
  connect(ui.zoomInBtn, SIGNAL( released() ), this, SLOT( onZoomInReleased() ));
  connect(ui.zoom11Btn, SIGNAL( released() ), this, SLOT( onZoom11Released() ));

  //connect(ui.thumbLabel, SIGNAL( areaPositionChanged(const QPointF &) ), 
  //        this,          SLOT( onThumbAreaPosChanged(const QPointF &) ));
  connect(ui.thumbLabel, SIGNAL( areaPositionSelected(const QPointF &) ), 
            this,          SLOT( onThumbAreaPosChanged(const QPointF &) ));

  connect(ui.thumbLabel, SIGNAL( cursorPositionChanged(const QPointF &) ), 
          this,          SLOT( onCursorPositionChanged(const QPointF &) ));

  connect(ui.thumbLabel, SIGNAL( toolButtonPressed(const QPointF &, Qt::MouseButton) ), 
          this,          SLOT( onToolButtonPressed(const QPointF &, Qt::MouseButton) ));

  connect(ui.horizontalSlider, SIGNAL( valueChanged(int) ), this, SLOT( onSliderValueChanged(int) ));

  connect(ui.pageMinsBtn, SIGNAL( released() ), this, SLOT( onPrevPageReleased() ));
  connect(ui.pagePlusBtn, SIGNAL( released() ), this, SLOT( onNextPageReleased() ));
  connect(ui.pageZeroBtn, SIGNAL( released() ), this, SLOT( onInitPageReleased() ));

  connect(ui.browseBtn, SIGNAL( released() ), this, SLOT( open() ));


  //-------------------
  // Menus
  ui.actionConnect->setEnabled(true);
  connect(ui.actionConnect, SIGNAL(triggered()), this, SLOT(clientConnect()));

  ui.actionDisconnect->setEnabled(false);
  connect(ui.actionDisconnect, SIGNAL(triggered()), this, SLOT(clientDisconnect()));

  connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
  connect(ui.actionHelp, SIGNAL(triggered()), this, SLOT(help()));
  connect(ui.actionFullVirtualScreen, SIGNAL(triggered()), this, SLOT(fullVirtualScreen()));
  connect(ui.actionCloseWall, SIGNAL(triggered()), this, SLOT(muHostClose()));
  connect(ui.actionShowMetaData, SIGNAL(triggered()), this, SLOT(muHostShowMetadata()));
  connect(ui.actionShowScaleBar, SIGNAL(triggered()), this, SLOT(muHostShowScaleBar()));

  connect(ui.actionUse_Smooth_Zoom,         SIGNAL(triggered()), messager, SLOT(sendTrigSmoothZoom()));
  connect(ui.actionUse_Dynamic_Enhancement, SIGNAL(triggered()), messager, SLOT(sendTrigDynEnhance()));
  connect(ui.actionRotate_Right,            SIGNAL(triggered()), messager, SLOT(sendRotateRight()));
  connect(ui.actionRotate_Left,             SIGNAL(triggered()), messager, SLOT(sendRotateLeft()));
  connect(ui.actionRotate_180_deg,          SIGNAL(triggered()), messager, SLOT(sendRotate180()));

  connect(ui.actionConfigure_wall_host, SIGNAL(triggered()), this, SLOT(onConfigureWallHost()));

  connect(ui.actionProject_all_planes_by_maximum_intesity, SIGNAL(triggered()), 
          messager,                                        SLOT(sendProjectMax()));

  connect(ui.actionProject_all_planes_by_minimum_intesity, SIGNAL(triggered()), 
          messager,                                        SLOT(sendProjectMin()));

  setObjectName( "RemoteWidget" );

  conf = new DConfig( WV_ORGANIZATION, WV_APPLICATION, WV_CONFIG_FILE );
  conf->loadConfig();
  conf->loadWidgetConfig( this );

  setChildrenEnabled(false);

  //scaleBar = NULL;
  scaleBar = new DScaleBar(this);
  //scaleBar->hide();
  show_scale_bar = conf->getValue( "ShowScaleBar", false ).toBool();
}

WVRemoteWidget::~WVRemoteWidget() {
  conf->saveConfig();
  conf->saveWidgetConfig( this );
}

//-------------------------------------------------------------------------------
//callbacks
//-------------------------------------------------------------------------------

void WVRemoteWidget::operationStart( bool determinate) {
  time_prgs.start();  
  //progress_circle = new DProgressCircle(this);
  //progress_circle->showBasedOn( this );
  //setChildrenEnabled(false);
}

void WVRemoteWidget::operationStop() {
  if (progress_circle != NULL) delete progress_circle;
  progress_circle = NULL;
  setChildrenEnabled(true);
}

void WVRemoteWidget::operationProgress( const QString &s, int total, int pos ) {
  if (time_prgs.elapsed() < 200 ) return;

  if (progress_circle == NULL) {
    progress_circle = new DProgressCircle(this);
    progress_circle->showBasedOn( this );
    setChildrenEnabled(false);
  }

  if (progress_circle != NULL) {
    progress_circle->setText(s);
    progress_circle->setProgress( pos*100.0/total );
  }

  QApplication::processEvents();
  time_prgs.start();
}

void WVRemoteWidget::operationMessage( MsgTextType id, const QString &s ) {

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

//-------------------------------------------------------------------------------
// internals
//-------------------------------------------------------------------------------

void WVRemoteWidget::about() {
  QString msg;
  msg.sprintf( "<b>bioView Remote ver: %s</b>, 2005-2010<br>", WV_REMOTE_VERSION );
  msg += "Center for BioImage Informatics, UCSB<br />";
  msg += "www.bioimage.ucsb.edu<br />";
  msg += "Author: Dima Fedorov";
  QMessageBox::about(this, tr("About bioView remote"), msg);
}
  
void WVRemoteWidget::help() {
  QString docs = "file:///" + DSysConfig::applicationPath() + "/doc/bioview_remote.html"; 
  QDesktopServices::openUrl(docs);
}

void WVRemoteWidget::fullVirtualScreen() {
  messager->sendFullVirtualScreen( );
}

void WVRemoteWidget::createStatusBar() {
  status_bar = new QStatusBar(this);
  statusImageInf = new QLabel(status_bar);
  status_bar->addWidget ( statusImageInf );
  statusImageInf->setMinimumWidth( 250 );
  setStatusBar(status_bar);
}

void WVRemoteWidget::initMessager() {
  
  connect( messager, SIGNAL( channelsListChanged( const QStringList &) ), 
           this,       SLOT( nmChannelsListChanged(const QStringList &) ));

  connect( messager, SIGNAL( enhancementListChanged( const QStringList &) ), 
           this,       SLOT( nmEnhancementListChanged(const QStringList &) ));

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

  connect( messager, SIGNAL( enhancementChanged( const qint32 &) ), 
           this,       SLOT( nmEnhancementChanged(const qint32 &) ));

  connect( messager, SIGNAL( pageChanged( const qint32 &) ), 
           this,       SLOT( nmPageChanged(const qint32 &) ));

  connect( messager, SIGNAL( numPagesChanged( const qint32 &) ), 
           this,       SLOT( nmNumPagesChanged(const qint32 &) ));

  connect( messager, SIGNAL( imageSizeChanged( const QSize &) ), 
           this,       SLOT( nmImageSizeChanged(const QSize &) ));

  connect( messager, SIGNAL( metadataChanged(const QString &) ), 
           this,       SLOT( nmMetadataChanged(const QString &) ));

  connect( messager, SIGNAL( thumbnailChanged(const QPixmap &) ), 
           this,       SLOT( nmThumbnailChanged(const QPixmap &) ));

  connect( messager, SIGNAL( visibleAreaChanged(const QRectF &) ), 
           this,       SLOT( nmVisibleAreaChanged(const QRectF &) ));

  connect( messager, SIGNAL( scaleChanged( const double &) ), 
           this,       SLOT( nmScaleChanged( const double &) ));

  connect( messager, SIGNAL( operationStart(const QString &) ), 
           this,       SLOT( nmOperationStart(const QString &) ));

  connect( messager, SIGNAL( operationEnd() ), 
           this,       SLOT( nmOperationEnd( ) ));

  connect( messager, SIGNAL( operationProgress(const qint32 &) ), 
           this,       SLOT( nmOperationProgress(const qint32 &) ));

  connect( messager, SIGNAL( physPixelSizeX(const double &) ), 
           this,       SLOT( nmPhysPixelSizeX(const double &) ));

  connect( messager, SIGNAL( physPixelSizeUnit(const QString &) ), 
           this,       SLOT( nmPhysPixelSizeUnit(const QString &) ));

  connect( messager, SIGNAL( imageInfoText(const QString &) ), 
           this,       SLOT( nmImageInfoText(const QString &) ));

  connect( messager, SIGNAL( showSmoothZoom(bool) ), 
           ui.actionUse_Smooth_Zoom,       SLOT( setChecked(bool) ));

  connect( messager, SIGNAL( showDynEnhance(bool) ), 
           ui.actionUse_Dynamic_Enhancement, SLOT( setChecked(bool) ));

}

void WVRemoteWidget::setChildrenEnabled(bool v) {
  ui.redCombo->setEnabled(v);
  ui.greenCombo->setEnabled(v);
  ui.blueCombo->setEnabled(v);
  ui.yellowCombo->setEnabled(v);
  ui.magentaCombo->setEnabled(v);
  ui.cyanCombo->setEnabled(v);
  ui.enhanceCombo->setEnabled(v);
  ui.browseBtn->setEnabled(v);
  ui.zoomOutBtn->setEnabled(v);
  ui.zoom11Btn->setEnabled(v);
  ui.zoomInBtn->setEnabled(v);
  
  ui.horizontalSlider->setEnabled( (num_pages > 1) & v );
  ui.pageZeroBtn->setEnabled( (num_pages > 1) & v );
  ui.pageMinsBtn->setEnabled( (num_pages > 1) & v );
  ui.pagePlusBtn->setEnabled( (num_pages > 1) & v );
}

void WVRemoteWidget::initChannels( const QStringList &l, int r, int g, int b ) {
  ui.redCombo->clear();
  ui.greenCombo->clear();
  ui.blueCombo->clear();
  ui.yellowCombo->clear( );
  ui.magentaCombo->clear( );
  ui.cyanCombo->clear( );

  ui.redCombo->addItems( l );
  ui.greenCombo->addItems( l );
  ui.blueCombo->addItems( l );
  ui.yellowCombo->addItems( l );
  ui.magentaCombo->addItems( l );
  ui.cyanCombo->addItems( l );

  ui.redCombo->setCurrentIndex( r );
  ui.greenCombo->setCurrentIndex( g );
  ui.blueCombo->setCurrentIndex( b );
  ui.yellowCombo->setCurrentIndex( 0 );
  ui.magentaCombo->setCurrentIndex( 0 );
  ui.cyanCombo->setCurrentIndex( 0 );

  display_channels_visibility[0] = r;
  display_channels_visibility[1] = g;
  display_channels_visibility[2] = b;
  display_channels_visibility[3] = 0;
  display_channels_visibility[4] = 0;
  display_channels_visibility[5] = 0;
}

void WVRemoteWidget::setChannelRed( int v ) {
  ui.redCombo->setCurrentIndex( v );
}

void WVRemoteWidget::setChannelGreen( int v ) {
  ui.greenCombo->setCurrentIndex( v );
}

void WVRemoteWidget::setChannelBlue( int v ) {
  ui.blueCombo->setCurrentIndex( v );
}

void WVRemoteWidget::setChannelYellow( int v ) {
  ui.yellowCombo->setCurrentIndex( v );
}

void WVRemoteWidget::setChannelMagenta( int v ) {
  ui.magentaCombo->setCurrentIndex( v );
}

void WVRemoteWidget::setChannelCyan( int v ) {
  ui.cyanCombo->setCurrentIndex( v );
}

void WVRemoteWidget::initEnhancement( const QStringList &l, int def ) {
  ui.enhanceCombo->clear();
  ui.enhanceCombo->addItems( l );
  ui.enhanceCombo->setCurrentIndex( def );
}

void WVRemoteWidget::setEnhancement( int v ) {
  ui.enhanceCombo->setCurrentIndex( v );
}

void WVRemoteWidget::onChannelsChanged(int) {

  int r = ui.redCombo->currentIndex();
  int g = ui.greenCombo->currentIndex();
  int b = ui.blueCombo->currentIndex();
  int y = ui.yellowCombo->currentIndex();
  int m = ui.magentaCombo->currentIndex();
  int c = ui.cyanCombo->currentIndex();

  emit channelsChanged(r, g, b);
  //messager->sendRedChanged(r);
  //messager->sendBlueChanged(b);
  //messager->sendGreenChanged(g);
  //messager->sendViewChanChanged( QColor(r,g,b) );
  QList<QVariant> cl;
  cl << r << g << b << y << m << c;
  messager->sendViewChannelsChanged( cl );
}

bool setStates( QCheckBox *chb, QComboBox *cmb, int *chan_vis ) {
  int current_display = 0;
  if (chb->checkState() == Qt::Unchecked) {
    *chan_vis = cmb->currentIndex();
    current_display = 0;
  } else
    current_display = *chan_vis;

  if (cmb->currentIndex() != current_display) {
    cmb->setCurrentIndex( current_display );
    return true;
  }
  return false;
}

void WVRemoteWidget::onChannelsVisChanged(int) {
  bool vis_changed = false;
  vis_changed = vis_changed || setStates( ui.redCheck,     ui.redCombo,     &display_channels_visibility[0] );
  vis_changed = vis_changed || setStates( ui.greenCheck,   ui.greenCombo,   &display_channels_visibility[1] );
  vis_changed = vis_changed || setStates( ui.blueCheck,    ui.blueCombo,    &display_channels_visibility[2] );
  vis_changed = vis_changed || setStates( ui.yellowCheck,  ui.yellowCombo,  &display_channels_visibility[3] );
  vis_changed = vis_changed || setStates( ui.magentaCheck, ui.magentaCombo, &display_channels_visibility[4] );
  vis_changed = vis_changed || setStates( ui.cyanCheck,    ui.cyanCombo,    &display_channels_visibility[5] );
  if (vis_changed) onChannelsChanged(0);
}

void WVRemoteWidget::onEnhancementChanged(int) {

  int e = ui.enhanceCombo->currentIndex();
  emit enhancementChanged(e);
  messager->sendEnhancementChanged(e);
}

void WVRemoteWidget::onThumbAreaPosChanged(const QPointF &center) {
  emit visCenterChanged( center );
  messager->sendPositionChanged(center);
}

void WVRemoteWidget::mouseMoveEvent( QMouseEvent *event ) {
  if (in_mouse_move)
    this->move( this->pos() + ( event->pos() - mousePoint ) );
}

void WVRemoteWidget::mousePressEvent( QMouseEvent *event ) {
  mousePoint = event->pos();
  in_mouse_move = true;
}

void WVRemoteWidget::mouseReleaseEvent( QMouseEvent *event ) {
  in_mouse_move = false;
}

void WVRemoteWidget::resizeEvent(QResizeEvent *) {
  if ( (scaleBar != NULL) && show_scale_bar && (pixel_scale>0.0) )
    scaleBar->showBasedOn( ui.thumbLabel );
}

void WVRemoteWidget::closeEvent ( QCloseEvent * ) {
  if ( conf->getValue( "CloseWallOnExit", false ).toBool() ) {
    messager->sendCloseWall();
    QApplication::processEvents();
  }
  qApp->quit();
}

/*
void WVRemoteWidget::keyPressEvent ( QKeyEvent * e ) {
  ui.thumbLabel->keyPressEvent(e);
}

void WVRemoteWidget::keyReleaseEvent ( QKeyEvent * e ) {
  ui.thumbLabel->keyReleaseEvent(e);
}
*/

void WVRemoteWidget::onSliderValueChanged ( int value ) {
  if (value != cur_page) {
    emit pageChanged( value );
    messager->sendPageChanged( value );
  }
}

void WVRemoteWidget::onNextPageReleased() {
  if (cur_page < num_pages-1) {
    emit pageChanged( cur_page+1 );
    messager->sendPageChanged( cur_page+1 );
  }
}

void WVRemoteWidget::onPrevPageReleased() {
  if (cur_page > 0) {
    emit pageChanged( cur_page-1 );
    messager->sendPageChanged( cur_page-1 );
  }
}

void WVRemoteWidget::onInitPageReleased() {
  if (cur_page != 0) {
    emit pageChanged( 0 );
    messager->sendPageChanged(0);
  }
}

void WVRemoteWidget::onZoomOutReleased() {
  emit zoomChanged(-1);
  messager->sendScaleChanged(-1);
}

void WVRemoteWidget::onZoomInReleased() {
  emit zoomChanged(1);
  messager->sendScaleChanged(1);
}

void WVRemoteWidget::onZoom11Released() {
  emit zoomChanged(0);
  messager->sendScaleChanged(0);
}

void WVRemoteWidget::onToolButtonPressed( const QPointF &center, Qt::MouseButton button ) {
  int zoom = 0;
  if (button == Qt::LeftButton) zoom = 1;
  if (button == Qt::RightButton) zoom = -1;  
  //emit zoomChanged(zoom);
  //messager->sendScaleChanged(zoom);
  if (button == Qt::LeftButton)  messager->sendZoomIn( center );
  if (button == Qt::RightButton) messager->sendZoomOut( center );
}


void WVRemoteWidget::onCursorPositionChanged ( const QPointF &p ) {
  messager->sendCursorPosition(p);
  statusImageInf->setText( tr("[%1,%2]").arg(p.x()).arg(p.y()) );
}

void WVRemoteWidget::setThumbnail( const QPixmap &pm, unsigned int orig_w, unsigned int orig_h ) {
  if (!pm.isNull()) ui.thumbLabel->setPixmap( pm );
  setOriginalImageSize( orig_w, orig_h );
  ui.thumbLabel->setVisAreaShow( true );
}

void WVRemoteWidget::setOriginalImageSize( unsigned int orig_w, unsigned int orig_h ) {
  ui.thumbLabel->init( QSize(orig_w, orig_h), QRectF(0,0,0,0) );
}

void WVRemoteWidget::setMetadata( const QString &txt ) {
  //ui.metaEdit->setTextColor( QColor(255,255,255) ); 
  //ui.metaEdit->setPlainText( txt );
  ui.metaEdit->setHtml( txt );
}

void WVRemoteWidget::setPages( unsigned int _num_pages ) {
  num_pages = _num_pages;
  ui.horizontalSlider->setMaximum( num_pages-1 );
  onPageChanged(0);
  cur_page = 0;
  
  bool enable_page_btns = false;
  if (num_pages > 1) enable_page_btns = true;
  ui.pageZeroBtn->setEnabled( enable_page_btns );
  ui.pageMinsBtn->setEnabled( enable_page_btns );
  ui.pagePlusBtn->setEnabled( enable_page_btns );
  ui.pagesGroup->setTitle( tr("Page %1/%2").arg(cur_page+1).arg(num_pages) );
}

void WVRemoteWidget::updateView(const QRectF &r) {
  // draw visible region
  ui.thumbLabel->updateVisArea( r );
  ui.thumbLabel->repaint();
}

void WVRemoteWidget::updateScale(double scale) {
  QString s;
  if (scale < 1.0)
    s.sprintf("Scale: [1:%.0f]", 1.0/scale);
  else
    s.sprintf("Scale: [%.0f:1]", scale);
  ui.scaleGroup->setTitle( s );
}

void WVRemoteWidget::onViewChanged(double scale, const QRectF &r) {
  updateScale(scale);
  updateView(r);
}

void WVRemoteWidget::onPageChanged(const qint32 _page) {
  if (cur_page == _page) return;
  cur_page = _page;
  ui.horizontalSlider->setValue( cur_page );
  ui.pagesGroup->setTitle( tr("Page %1/%2").arg(cur_page+1).arg(num_pages) );
}

void WVRemoteWidget::onConfigureWallHost() {
  QString hostName = conf->getValue( "ServerName", "localhost" ).toString();
  
  bool ok;
  hostName = QInputDialog::getText(this, tr("Wall host name"), tr("Wall host name:"), QLineEdit::Normal, hostName, &ok);
  if (ok && !hostName.isEmpty())
    conf->setValue( "ServerName", hostName );
}

void WVRemoteWidget::clientConnect() {
  QString hostName = conf->getValue( "ServerName", "localhost" ).toString();
  int hostPort = conf->getValue( "ServerPort", 0 ).toInt();

  operationMessage( mttInfo, tr("Attempting connection to %1:%2").arg(hostName).arg(hostPort) );
  client.connectToServer( hostName, hostPort );
}

void WVRemoteWidget::clientDisconnect() {
  client.disconnect();
}

void WVRemoteWidget::muHostClose() {
  messager->sendCloseWall();
  QApplication::processEvents();
  qApp->quit();
}

void WVRemoteWidget::muHostShowMetadata() {
  messager->sendShowMetadata( ui.actionShowMetaData->isChecked() );
}

void WVRemoteWidget::muHostShowScaleBar() {
  messager->sendShowScaleBar( ui.actionShowScaleBar->isChecked() );
}


void WVRemoteWidget::onClientConnected() {
  ui.actionConnect->setEnabled(false);
  ui.actionDisconnect->setEnabled(true);

  setChildrenEnabled(true);

  QString s;
  s += "Connected to ";
  s += client.socket()->peerName();
  s += " ";
  s += client.socket()->peerAddress().toString();
  s += tr(":%1").arg( client.socket()->peerPort() );
  operationMessage( mttInfo, s );

  // now we have to let the server know of the thumbnail size needed
  //QDesktopWidget *desktop = QApplication::desktop();
  //QRect r = desktop->screenGeometry( desktop->primaryScreen() );
  //messager->sendThumbSizeChanged( r.size() );
  messager->sendThumbSizeChanged( ui.thumbLabel->size() );
  
}

void WVRemoteWidget::onClientDisconnected() {
  ui.actionConnect->setEnabled(true);
  ui.actionDisconnect->setEnabled(false);
  setChildrenEnabled(false);
  operationMessage( mttInfo, tr("Disconnected...") );

  if ( conf->getValue( "CloseOnServerDisconnected", false ).toBool() )
    qApp->quit();    
}

void WVRemoteWidget::onClientError() {
  operationMessage( mttWarning, tr("Network error: %1").arg( client.socket()->errorString() ) );
}

void WVRemoteWidget::open() {
  QString path = conf->loadPath( "ImagePath" );
  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), path);
  if (!fileName.isEmpty()) {
    
    // if local send file name, otherwise the whole file
    if ( client.socket()->localAddress() == client.socket()->peerAddress() )
      messager->sendLocalFileName( fileName );
    else
      messager->sendFile( fileName );
    
    ui.fileEdit->setText( fileName );
    conf->savePath( "ImagePath", fileName );
  }
}


void WVRemoteWidget::onExtChannels() {
  if (ui.exChannelsWidget->isVisible()) {
    ui.exChannelsWidget->setVisible(false);
    ui.moreChannelsLabel->setText("<a href=\"#\">more...</a>");
    //ui.chanGroup->setMinimumHeight(110);
  } else {
    ui.exChannelsWidget->setVisible(true);
    ui.moreChannelsLabel->setText("<a href=\"#\">less...</a>");
    //ui.chanGroup->setMinimumHeight(180);
  }
}

//-------------------------------------------------------------------------------
// network messages
//-------------------------------------------------------------------------------

void WVRemoteWidget::nmChannelsListChanged( const QStringList &v ) {
  initChannels( v, 0, 0, 0 );
}

void WVRemoteWidget::nmRedChanged( const qint32 &v ) {
  setChannelRed( v );
}

void WVRemoteWidget::nmGreenChanged( const qint32 &v ) {
  setChannelGreen( v );
}

void WVRemoteWidget::nmBlueChanged( const qint32 &v ) {
  setChannelBlue( v );
}

void WVRemoteWidget::nmYellowChanged( const qint32 &v ) {
  setChannelYellow( v );
}

void WVRemoteWidget::nmMagentaChanged( const qint32 &v ) {
  setChannelMagenta( v );
}

void WVRemoteWidget::nmCyanChanged( const qint32 &v ) {
  setChannelCyan( v );
}

void WVRemoteWidget::nmEnhancementChanged( const qint32 &v ) {
  setEnhancement( v );
}

void WVRemoteWidget::nmEnhancementListChanged ( const QStringList &v ) {
  initEnhancement( v );
}

void WVRemoteWidget::nmPageChanged( const qint32 &v ) {
  messager->blockSending(true);  
  onPageChanged(v);
  messager->blockSending(false);
}

void WVRemoteWidget::nmNumPagesChanged( const qint32 &v ) {
  messager->blockSending(true);
  setPages(v);
  messager->blockSending(false);
}

void WVRemoteWidget::nmImageSizeChanged( const QSize &v ) {
  setOriginalImageSize( v.width(), v.height() );
}

void WVRemoteWidget::nmMetadataChanged( const QString &v ) {
  setMetadata( v );
}

void WVRemoteWidget::nmThumbnailChanged( const QPixmap &v ) {
  QSize s = ui.thumbLabel->imageSize();
  setThumbnail( v, s.width(), s.height() );

  if ( show_scale_bar && (pixel_scale!=0) && (!v.isNull()) ) {
    thumb_ratio = s.width() / v.width();
    scaleBar->setScale( pixel_scale * thumb_ratio );
    scaleBar->showBasedOn( ui.thumbLabel );
  }
  else scaleBar->hide();
}

void WVRemoteWidget::nmVisibleAreaChanged( const QRectF &v ) {
  updateView(v);
}

void WVRemoteWidget::nmScaleChanged( const double &v ) {
  updateScale(v);
}

void WVRemoteWidget::nmOperationStart( const QString &v ) {
  operation_message = v;
  operationStart();
}

void WVRemoteWidget::nmOperationEnd( ) {
  operationStop();
}

void WVRemoteWidget::nmOperationProgress( const qint32 &v ) {
  operationProgress( operation_message, 100, v );
}
  
void WVRemoteWidget::nmPhysPixelSizeX( const double &v ) {
  
  pixel_scale = v;
  if (scaleBar != NULL) 
    scaleBar->setScale( pixel_scale * thumb_ratio );
}

void WVRemoteWidget::nmPhysPixelSizeUnit( const QString &v ) {
  if (scaleBar != NULL) 
    scaleBar->setUnits( v );
}

void WVRemoteWidget::nmImageInfoText( const QString &v ) {
  statusImageInf->setText( v );
}




