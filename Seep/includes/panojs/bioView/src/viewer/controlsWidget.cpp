/*******************************************************************************
  
  WVControlsWidget is an additional widget for tools 
    
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
       
*******************************************************************************/

#include <QtGui>
//#include <QMouseEvent>
//#include <QPainter>

#include "wvimagemanager.h"

#include "controlsWidget.h"

WVControlsWidget::WVControlsWidget(QWidget *parent, Qt::WindowFlags f)
//: QWidget(parent, f)//, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint )
: QWidget(parent, f | Qt::Tool )
{
  setObjectName( "ControlWidget" );
  ui.setupUi(this);
  in_mouse_move = false;
  cur_page=0;
  num_pages=1;

  scrollArea = new QScrollArea();
  scrollArea->setWidget(ui.baseFrame);
  ui.baseLayout->addWidget(scrollArea);
  ui.enhanceParamsGroup->setVisible(false);
  ui.exChannelsWidget->setVisible(false);

  this->setAttribute( Qt::WA_ContentsPropagated, true );
  this->setWindowOpacity( 0.7 );

  //this->setAttribute( Qt::WA_NoBackground, true );
  //this->setAttribute( Qt::WA_NoSystemBackground, true );
  //this->setAttribute( Qt::WA_StaticContents, true );
  //this->setAttribute( Qt::WA_PaintOnScreen, false );

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

  connect(ui.thumbLabel, SIGNAL( areaPositionChanged(const QPointF &) ), 
          this,          SLOT( onThumbAreaPosChanged(const QPointF &) ));

  connect(ui.thumbLabel, SIGNAL( areaPositionSelected(const QPointF &) ), 
          this,          SLOT( onThumbAreaPosSelected(const QPointF &) ));

  connect(ui.horizontalSlider, SIGNAL( valueChanged(int) ), this, SLOT( onSliderValueChanged(int) ));

  connect(ui.pageMinsBtn, SIGNAL( released() ), this, SLOT( onPrevPageReleased() ));
  connect(ui.pagePlusBtn, SIGNAL( released() ), this, SLOT( onNextPageReleased() ));
  connect(ui.pageZeroBtn, SIGNAL( released() ), this, SLOT( onInitPageReleased() ));

  

  ui.chanGroup->installEventFilter(this);
  ui.enhanceGroup->installEventFilter(this);
  ui.enhanceParamsGroup->installEventFilter(this);
  ui.scaleGroup->installEventFilter(this);
  ui.pagesGroup->installEventFilter(this);
  ui.metaGroup->installEventFilter(this);

  scrollArea->widget()->installEventFilter(this);

  connect(ui.chanGroup, SIGNAL(resizedMinimum() ), this, SLOT( boxResizedMinimum() ));
  connect(ui.enhanceGroup, SIGNAL(resizedMinimum() ), this, SLOT( boxResizedMinimum() ));
  connect(ui.enhanceParamsGroup, SIGNAL(resizedMinimum() ), this, SLOT( boxResizedMinimum() ));
  connect(ui.scaleGroup, SIGNAL(resizedMinimum() ), this, SLOT( boxResizedMinimum() ));
  connect(ui.pagesGroup, SIGNAL(resizedMinimum() ), this, SLOT( boxResizedMinimum() ));
  connect(ui.metaGroup, SIGNAL(resizedMinimum() ), this, SLOT( boxResizedMinimum() ));

  // platform specific code for correct window generation
#ifdef Q_WS_X11
  #ifndef WV_EMBEDDED_CONTROLS
  //QPalette pal = QApplication::palette();
  QPalette pal = this->palette();
  ui.pageMinsBtn->setPalette( pal );
  ui.pagePlusBtn->setPalette( pal );
  ui.pageZeroBtn->setPalette( pal );
  ui.zoomOutBtn->setPalette( pal );
  ui.zoomInBtn->setPalette( pal );
  ui.zoom11Btn->setPalette( pal );
  #endif //WV_EMBEDDED_CONTROLS
#endif

#ifdef Q_WS_MAC

  QFont headerFont = ui.chanGroup->font();

  headerFont.setPointSize(11);
  headerFont.setBold(true);
  ui.chanGroup->setFont( headerFont );
  ui.enhanceGroup->setFont( headerFont );
  ui.scaleGroup->setFont( headerFont );
  ui.pagesGroup->setFont( headerFont );
  ui.metaGroup->setFont( headerFont );

  #ifndef WV_EMBEDDED_CONTROLS
  QPalette pal = this->palette();
  ui.redLabel->setPalette( pal );
  ui.greenLabel->setPalette( pal );
  ui.blueLabel->setPalette( pal );
  ui.yellowLabel->setPalette( pal );
  ui.magentaLabel->setPalette( pal );
  ui.cyanLabel->setPalette( pal );
  ui.enhanceLabel->setPalette( pal );
  #endif //WV_EMBEDDED_CONTROLS

  QFont labelFont = ui.redLabel->font();
  labelFont.setPointSize(12);
  ui.redLabel->setFont( labelFont );
  ui.greenLabel->setFont( labelFont );
  ui.blueLabel->setFont( labelFont );
  ui.magentaLabel->setFont( labelFont );
  ui.cyanLabel->setFont( labelFont );
  ui.enhanceLabel->setFont( labelFont );
  ui.enhanceLabel->setFont( labelFont );

  QFont comboFont = ui.redCombo->font();
  comboFont.setPointSize(12);
  comboFont.setBold(false);
  ui.redCombo->setFont( comboFont );
  ui.greenCombo->setFont( comboFont );
  ui.blueCombo->setFont( comboFont );
  ui.yellowCombo->setFont( comboFont );
  ui.magentaCombo->setFont( comboFont );
  ui.cyanCombo->setFont( comboFont );
  ui.enhanceCombo->setFont( comboFont );

  int height_constant = 25;
  ui.chanGroup->setMinimumHeight   ( ui.chanGroup->minimumHeight()+height_constant );
  ui.enhanceGroup->setMinimumHeight( ui.enhanceGroup->minimumHeight()+height_constant );
  ui.scaleGroup->setMinimumHeight  ( ui.scaleGroup->minimumHeight()+height_constant );
  ui.pagesGroup->setMinimumHeight  ( ui.pagesGroup->minimumHeight()+height_constant );
  ui.metaGroup->setMinimumHeight   ( ui.metaGroup->minimumHeight()+height_constant );

#endif

#ifdef Q_WS_WIN
  QPalette pal = ui.redCombo->palette();
  pal.setColor( QPalette::Text, QColor(0,0,0) );
  pal.setColor( QPalette::Base, QColor(128,128,128) );
  ui.redCombo->setPalette( pal );
  ui.greenCombo->setPalette( pal );
  ui.blueCombo->setPalette( pal );
  ui.yellowCombo->setPalette( pal );
  ui.magentaCombo->setPalette( pal );
  ui.cyanCombo->setPalette( pal );
  ui.enhanceCombo->setPalette( pal );
#endif

}

void WVControlsWidget::initChannels( const QStringList &l, int r, int g, int b ) {
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

void WVControlsWidget::initEnhancement( const QStringList &l, int def ) {
  ui.enhanceCombo->clear();
  ui.enhanceCombo->addItems( l );
  ui.enhanceCombo->setCurrentIndex( def );
}

void WVControlsWidget::onChannelsChanged(int) {

  int r = ui.redCombo->currentIndex();
  int g = ui.greenCombo->currentIndex();
  int b = ui.blueCombo->currentIndex();
  int y = ui.yellowCombo->currentIndex();
  int m = ui.magentaCombo->currentIndex();
  int c = ui.cyanCombo->currentIndex();
  emit channelsChanged(r, g, b, y, m, c);
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

void WVControlsWidget::onChannelsVisChanged(int) {
  bool vis_changed = false;
  vis_changed = vis_changed || setStates( ui.redCheck,     ui.redCombo,     &display_channels_visibility[0] );
  vis_changed = vis_changed || setStates( ui.greenCheck,   ui.greenCombo,   &display_channels_visibility[1] );
  vis_changed = vis_changed || setStates( ui.blueCheck,    ui.blueCombo,    &display_channels_visibility[2] );
  vis_changed = vis_changed || setStates( ui.yellowCheck,  ui.yellowCombo,  &display_channels_visibility[3] );
  vis_changed = vis_changed || setStates( ui.magentaCheck, ui.magentaCombo, &display_channels_visibility[4] );
  vis_changed = vis_changed || setStates( ui.cyanCheck,    ui.cyanCombo,    &display_channels_visibility[5] );
  if (vis_changed) onChannelsChanged(0);
}

void WVControlsWidget::onEnhancementChanged(int) {

  int e = ui.enhanceCombo->currentIndex();
  emit enhancementChanged(e);
}

void WVControlsWidget::onThumbAreaPosChanged(const QPointF &center) {
  emit visCenterChanged( center );
}

void WVControlsWidget::onThumbAreaPosSelected(const QPointF &center) {
  emit visCenterSelected( center );
}

void WVControlsWidget::mouseMoveEvent( QMouseEvent *event ) {
  
#ifndef WV_EMBEDDED_CONTROLS	
  if (in_mouse_move)
    this->move( this->pos() + ( event->pos() - mousePoint ) );
#endif
}

void WVControlsWidget::mousePressEvent( QMouseEvent *event ) {
  mousePoint = event->pos();
  in_mouse_move = true;
}

void WVControlsWidget::mouseReleaseEvent( QMouseEvent *event ) {
  in_mouse_move = false;
}

void WVControlsWidget::onSliderValueChanged ( int value ) {
  if (value != cur_page) {
   if ( value < (num_pages - 1))
    emit pageChanged( value );
   else if (value >= (num_pages - 1))
	emit pageChanged( num_pages - 1 );
  }
}

void WVControlsWidget::onNextPageReleased() {
  if (cur_page < num_pages-1)
    emit pageChanged( cur_page+1 );
}

void WVControlsWidget::onPrevPageReleased() {
  if (cur_page > 0)
    emit pageChanged( cur_page-1 );
}

void WVControlsWidget::onInitPageReleased() {
  if (cur_page != 0)
    emit pageChanged( 0 );
}

bool WVControlsWidget::eventFilter(QObject *obj, QEvent *event) {

  if (event->type()==QEvent::Resize && obj==scrollArea->widget()) {
    this->viewportResizeEvent( static_cast<QResizeEvent *> (event) );
    return false;
  }

  if (event->type() == QEvent::MouseButtonPress) {
    this->mousePressEvent( static_cast<QMouseEvent *> (event) );
    return true;
  }

  if (event->type() == QEvent::MouseButtonRelease) {
    this->mouseReleaseEvent( static_cast<QMouseEvent *>(event) );
    return true;
  }

  if (event->type() == QEvent::MouseMove) {
    this->mouseMoveEvent( static_cast<QMouseEvent *>(event) );
    return true;
  }

  return false;
}

void WVControlsWidget::onZoomOutReleased() {
  emit zoomChanged(-1);
}

void WVControlsWidget::onZoomInReleased() {
  emit zoomChanged(1);
}

void WVControlsWidget::onZoom11Released() {
  emit zoomChanged(0);
}

void WVControlsWidget::setThumbnail( const QPixmap &pm, unsigned int orig_w, unsigned int orig_h ) {
  ui.thumbLabel->setPixmap( pm );
  ui.thumbLabel->init( QSize(orig_w, orig_h), QRectF(0,0,0,0) );
  ui.thumbLabel->setVisAreaShow( true );
}

void WVControlsWidget::setMetadata( const QString &txt ) {
  /*
  #ifndef WV_EMBEDDED_CONTROLS	
  ui.metaEdit->setTextColor( QColor(255,255,255) ); 
  #endif
  //ui.metaEdit->setPlainText( txt );
  ui.metaEdit->setHtml( txt );
  */
}

void WVControlsWidget::setMetadata( const QHash<QString, QVariant> &tags ) {
  QList<QTreeWidgetItem *> items;
  QHash<QString, QVariant>::const_iterator it = tags.begin();
  while ( it != tags.end() ) {
    QStringList l;
    l << it.key() <<it.value().toString();
    items.append(new QTreeWidgetItem(l));
    ++it;
  }
  ui.metaEdit->clear();
  ui.metaEdit->insertTopLevelItems(0, items);
  ui.metaEdit->sortItems(0, Qt::AscendingOrder);
}

void WVControlsWidget::setPages( unsigned int _num_pages ) {
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

void WVControlsWidget::onViewChanged(double scale, const QRectF &r) {
  QString s;
  if (scale < 1.0)
    s.sprintf("Scale: [1:%.0f]", 1.0/scale);
  else
    s.sprintf("Scale: [%.0f:1]", scale);
  ui.scaleGroup->setTitle( s );

  // draw visible region
  ui.thumbLabel->updateVisArea( r );
  ui.thumbLabel->repaint();
}

void WVControlsWidget::onPageChanged(int _page) {
  if (cur_page == _page) return;
  cur_page = _page;
  ui.pagesGroup->setTitle( tr("Page %1/%2").arg(cur_page+1).arg(num_pages) );
  ui.horizontalSlider->setValue( cur_page );
}

void WVControlsWidget::resizeEvent( QResizeEvent * event ) {
  QSize s = ui.baseFrame->frameSize();
  s.setWidth( scrollArea->viewport()->width() );
  s.setHeight( scrollArea->viewport()->height() );
  ui.baseFrame->resize(s);
}

void WVControlsWidget::viewportResizeEvent ( QResizeEvent * event ) {
  QSize s = ui.baseFrame->frameSize();
  s.setWidth( scrollArea->viewport()->width() );
  ui.baseFrame->resize(s);
}

void WVControlsWidget::showEvent ( QShowEvent * event ) {
  boxResizedMinimum();
  QSize s1,s2;
  QResizeEvent re(s1,s2);
  this->resizeEvent( &re );
  QApplication::processEvents();
  this->viewportResizeEvent( &re );
}

void WVControlsWidget::boxResizedMinimum() {
  int new_min_height = 0;
  int height_constant = 0;

  #ifdef Q_WS_X11
  height_constant = 20;
  #endif

  #ifdef Q_WS_MAC
  height_constant = 20;
  #endif

  #ifdef Q_WS_WIN
  height_constant = 5;
  #endif

  // iterate over children, check for a type DGroupBox and accumulate min height
  QObjectList ol = ui.baseFrame->children();
  for (unsigned int i=0; i<ol.size(); ++i) {
    DGroupBox *gb = qobject_cast<DGroupBox *>( ol[i] );
    if (gb && gb->isVisible())
      new_min_height += gb->minimumHeight() + height_constant;
  }
  ui.baseFrame->setMinimumHeight( new_min_height );

  QSize s1,s2;
  QResizeEvent re(s1,s2);
  QApplication::processEvents();
  this->viewportResizeEvent( &re );
}

void WVControlsWidget::setEnhancementControlsWidget( QWidget *w ) {
  unsigned int min_height = ui.chanGroup->minimumHeight() + ui.enhanceGroup->minimumHeight() +
                            ui.scaleGroup->minimumHeight() + ui.pagesGroup->minimumHeight() +
                            ui.metaGroup->minimumHeight();

  if (!w) {
    ui.enhanceParamsGroup->setVisible(false);
    ui.baseFrame->setMinimumHeight( min_height );
    return;
  }
  ui.enhanceLayout->addWidget(w);
  ui.enhanceParamsGroup->setMinimumSize( w->minimumSize() );
  ui.baseFrame->setMinimumHeight( min_height + ui.enhanceParamsGroup->minimumHeight() );

  ui.enhanceParamsGroup->setVisible(true);
  w->show();
}

void WVControlsWidget::insertLutModifier( WVLiveEnhancement *le ) {
  DGroupBox *b = new DGroupBox(ui.baseFrame);

  b->setTitle( le->name() );
  b->setFlat(false);
  b->setCheckable(true);
  b->setChecked( le->isEnabled() );
  ui.mainLayout->insertWidget( 3, b );
  connect(b, SIGNAL(toggled(bool) ), le, SLOT( setEnabled(bool) ));

  QFont headerFont = ui.chanGroup->font();
  b->setFont( headerFont );
  QPalette pal = ui.chanGroup->palette();
  b->setPalette( pal );

  // if there's parameters widget
  if (!le->widget()) return;
  QVBoxLayout *vboxLayout = new QVBoxLayout(b);
  vboxLayout->setSpacing(2);
  vboxLayout->setMargin(2);
  vboxLayout->addWidget(le->widget());

  QFont font = ui.redLabel->font();
  pal = ui.redLabel->palette();
  QObjectList ol = le->widget()->children();
  for (unsigned int i=0; i<ol.size(); ++i) {
    QLabel *l = qobject_cast<QLabel *>( ol[i] );
    if (l) {
      l->setFont( font );
      l->setPalette( pal );
    }
  }

  b->setMinimumSize( le->widget()->minimumSize() + b->shrunkSize() );
  le->widget()->show();
  b->setShrunk( !le->isEnabled() );
  
  //ui.baseFrame->setMinimumHeight( min_height + ui.enhanceParamsGroup->minimumHeight() );
  connect(b, SIGNAL(resizedMinimum() ), this, SLOT( boxResizedMinimum() ));
}

void WVControlsWidget::onExtChannels() {
  if (ui.exChannelsWidget->isVisible()) {
    ui.exChannelsWidget->setVisible(false);
    ui.moreChannelsLabel->setText("<a href=\"#\">more...</a>");
    ui.chanGroup->setMinimumHeight(110);
  } else {
    ui.exChannelsWidget->setVisible(true);
    ui.moreChannelsLabel->setText("<a href=\"#\">less...</a>");
    ui.chanGroup->setMinimumHeight(180);
  }
  boxResizedMinimum();
}
