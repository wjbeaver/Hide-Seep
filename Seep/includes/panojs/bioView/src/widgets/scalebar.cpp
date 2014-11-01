/*******************************************************************************

  DScaleBar is "stay on top" transparent scalebar
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
       
*******************************************************************************/

#include <QtGui>

#include "scalebar.h"

DScaleBar::DScaleBar(QWidget *parent, Qt::WindowFlags f)
//: QWidget( parent, Qt::Window | Qt::FramelessWindowHint )
: QWidget( parent, f | Qt::FramelessWindowHint )
{
  scale_x = 0;
  physical_size = 0;
  base_offset = QPoint(0,0);
  hide_number = false;
  hide_background = false;
  in_mouse_op = moNone;
  setMouseTracking(true);

  QColor colorBright(255,255,255,255);
  QColor colorDark(0,0,0,255);

  setWindowTitle(tr("Scale bar"));
  //resize(400, 400);
  hide();

  if (parentWidget() == 0) {
    this->setWindowOpacity( 0.7 );
    QPalette p = this->palette();
    p.setColor( QPalette::Dark, colorDark );
    this->setPalette(p);
    this->setBackgroundRole(QPalette::Dark);
  } else { 
    this->setAttribute( Qt::WA_ContentsPropagated, true );
    #if (QT_VERSION >= 0x040100)    
    this->setAutoFillBackground(false);
    #endif
  }
  createActions();
  this->setCursor(Qt::PointingHandCursor);
  setBrightColorWhite();
}

void DScaleBar::setScale( double scale, const QString &units ) {

  scale_x = scale;
  if (units != "") units_str = units;
  update();
}

void DScaleBar::setUnits( const QString &units ) {
  units_str = units;
}

bool DScaleBar::isEmpty() const {
  return (scale_x <= 0);
}

void DScaleBar::drawBar( QPainter *p ) {

  p->setRenderHint(QPainter::Antialiasing, true);
  
  int side = qMax(width(), height());
  double bar_height = height() / 8.0;
  double bar_offset = width() / 10.0;

  // draw rounded background only if widget is parented
  if (parentWidget() != 0 && !hide_background) {
    #if (QT_VERSION < 0x040200) || defined(Q_WS_X11)
    p->setOpacity( 1.0 );
    #else
    p->setOpacity( 0.7 );
    #endif
    QColor dc = colorDark;
    dc.setAlpha(100);
    QPen pen(dc);
    pen.setWidth(0);
    p->setPen( pen );
    p->setBrush( dc );  
    int xRound = 8, yRound = 8;
    if (width() > height()) 
      yRound = xRound * (width()/(double)height());
    else 
      xRound = yRound * (height()/(double)width());
    p->drawRoundRect( QRectF(0.0, 0.0, width()-1, height()-1), xRound, yRound );
  }

  #if (QT_VERSION >= 0x040200)    
  p->setOpacity( 1.0 );
  #endif

  double bar_phys_size = scale_x*(side-bar_offset*2.0);
  QColor c = colorBright;
  p->setPen( QPen(c, bar_height, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin) );
  QLineF line;
  QString str;
  if (physical_size <= 0) {
    line = QLineF( bar_offset, bar_offset*1.2, side-bar_offset, bar_offset*1.2);
    p->drawLine(line);
    str.sprintf("%0.2f ", bar_phys_size );
  } else {
    int pix_in_unit = 1.0 / scale_x;
    int pix_in_bar = pix_in_unit * physical_size;
    //if (pix_in_bar < 1) pix_in_bar = 1;
    line = QLineF( bar_offset, bar_offset*1.2, pix_in_bar+bar_offset, bar_offset*1.2);
    p->drawLine(line);
    if (pix_in_bar > side-2*bar_offset) {
      // draw black dots for continuation
      QColor pc = colorDark;
      p->setPen( QPen(pc, 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );  
      p->drawPoint( side-20, bar_offset*1.2 );
      p->drawPoint( side-15, bar_offset*1.2 );
      p->drawPoint( side-10, bar_offset*1.2 );
    }
    bar_phys_size = physical_size;
    if ( (bar_phys_size - (int)bar_phys_size)*10.0 < 1 )
      str.sprintf("~ %.0f ", bar_phys_size );
    else
      str.sprintf("~ %.2f ", bar_phys_size );
  }

  if (scale_x>0 && !hide_number) {
    p->setPen( colorBright );
    p->setFont(QFont("Helvetica [Cronyx]", bar_height*2, QFont::Bold));
    str += units_str;
    p->drawText(QPointF( bar_offset, bar_offset*2+bar_height*3 ), str);
  }
}

void DScaleBar::paintEvent(QPaintEvent *) {
  QPainter painter(this);
  drawBar( &painter );
}

void DScaleBar::resizeBasedOn(const QSize &s) {
  int side = qMin(s.width(), s.height());
  side /= 5.0;
  this->resize( side, side/2 );
}

void DScaleBar::showBasedOn(QWidget *parent, const QPoint &offset) {
  resizeBasedOn(parent->size());

  if (parent != 0) {
    base_offset.setX( parent->width() - width() - 40); 
    base_offset.setY( parent->height() - height() - 10); 

    if (parentWidget() != 0)
      this->move( base_offset.x()+offset.x(), base_offset.y()+offset.y() );
    else
      this->move( parent->x()+base_offset.x()+offset.x(), parent->y()+base_offset.y()+offset.y() );
  }

  this->show();
}

void DScaleBar::resizeEvent(QResizeEvent *) {
  if (parentWidget() != 0) return;
  QBitmap msk( size() );
  msk.clear();

  QPainter painter(&msk);
  painter.setPen(Qt::color1);
  painter.setBrush(Qt::color1);  
  painter.drawRoundRect( QRectF(0.0, 0.0, width()-1, height()-1), 8, 8 );

  setMask(msk);
}

void DScaleBar::setSizeManual() {
  bool ok;
  double d = QInputDialog::getDouble(this, tr("Custom size"), tr("Bar size (units):"), physical_size, 0, 10000, 2, &ok);
  if (ok) setSize(d);
}

void DScaleBar::createActions() {

  hideNumberAct = new QAction(tr("Hide value"), this);
  hideNumberAct->setCheckable(true);
  hideNumberAct->setChecked(false);
  connect(hideNumberAct, SIGNAL(triggered()), this, SLOT(triggerHideNumber()));

  hideBackgroundAct = new QAction(tr("Hide background"), this);
  hideBackgroundAct->setCheckable(true);
  hideBackgroundAct->setChecked(false);
  connect(hideBackgroundAct, SIGNAL(triggered()), this, SLOT(triggerHideBackground()));

  sizeAutoAct = new QAction(tr("Auto"), this);
  connect(sizeAutoAct, SIGNAL(triggered()), this, SLOT(setSize0()));

  size1Act = new QAction(tr("1"), this);
  connect(size1Act, SIGNAL(triggered()), this, SLOT(setSize1()));

  size5Act = new QAction(tr("5"), this);
  connect(size5Act, SIGNAL(triggered()), this, SLOT(setSize5()));

  size10Act = new QAction(tr("10"), this);
  connect(size10Act, SIGNAL(triggered()), this, SLOT(setSize10()));

  size20Act = new QAction(tr("20"), this);
  connect(size20Act, SIGNAL(triggered()), this, SLOT(setSize20()));

  size50Act = new QAction(tr("50"), this);
  connect(size50Act, SIGNAL(triggered()), this, SLOT(setSize50()));

  sizeManualAct = new QAction(tr("Custom"), this);
  connect(sizeManualAct, SIGNAL(triggered()), this, SLOT(setSizeManual()));

  colorWhiteAct = new QAction(tr("White"), this);
  connect(colorWhiteAct, SIGNAL(triggered()), this, SLOT(setBrightColorWhite()));

  colorGrayAct = new QAction(tr("Gray"), this);
  connect(colorGrayAct, SIGNAL(triggered()), this, SLOT(setBrightColorGray()));

  colorBlackAct = new QAction(tr("Black"), this);
  connect(colorBlackAct, SIGNAL(triggered()), this, SLOT(setBrightColorBlack()));
}

void DScaleBar::contextMenuEvent(QContextMenuEvent *event) {
  QMenu menu(this);

  QMenu *valMenu = menu.addMenu( "Value" );
  QMenu *colMenu = menu.addMenu( "Bar color" );
  menu.addSeparator();
  menu.addAction(hideNumberAct);
  menu.addAction(hideBackgroundAct);

  valMenu->addAction(sizeAutoAct);
  valMenu->addAction(sizeManualAct);
  valMenu->addSeparator();
  valMenu->addAction(size1Act);
  valMenu->addAction(size5Act);
  valMenu->addAction(size10Act);
  valMenu->addAction(size20Act);
  valMenu->addAction(size50Act);

  colMenu->addAction(colorWhiteAct);
  colMenu->addAction(colorGrayAct);
  colMenu->addAction(colorBlackAct);

  menu.exec(event->globalPos());
}

void DScaleBar::mouseMoveEvent( QMouseEvent *event ) {

  if (in_mouse_op == moNone) {
    if ( isInResizeBot(event->pos()) )
      this->setCursor(Qt::SizeVerCursor);
    else
    if ( isInResizeRight(event->pos()) )
      this->setCursor(Qt::SizeHorCursor);
    else
      this->setCursor(Qt::PointingHandCursor);
  }

  if (in_mouse_op == moMove) {
    this->move( this->pos() + ( event->globalPos() - mousePoint ) );
    base_offset += ( event->globalPos() - mousePoint );
  }

  if (in_mouse_op == moResizeBot) {
    QPoint dp = event->globalPos() - mousePoint;
    this->resize( this->width()+dp.x(), this->height()+dp.y() );
  }

  if (in_mouse_op == moResizeRight) {
    QPoint dp = event->globalPos() - mousePoint;
    this->resize( this->width()+dp.x(), this->height()+dp.y() );
  }

  mousePoint = event->globalPos();
}

void DScaleBar::mousePressEvent( QMouseEvent *event ) {
  if (event->button() != Qt::LeftButton) return;
  mousePoint = event->globalPos();

  if (isInResizeBot(event->pos())) {
    in_mouse_op = moResizeBot;
    this->setCursor(Qt::SizeVerCursor);
  } else
  if (isInResizeRight(event->pos())) {
    in_mouse_op = moResizeRight;
    this->setCursor(Qt::SizeHorCursor);
  } else {
    in_mouse_op = moMove;
    this->setCursor(Qt::SizeAllCursor);
  }
}

void DScaleBar::mouseReleaseEvent( QMouseEvent *event ) {
  in_mouse_op = moNone;
  this->setCursor(Qt::PointingHandCursor);
}


//*****************************************************************************
// DBarWidget
//*****************************************************************************

DBarWidget::DBarWidget(QWidget *parent, Qt::WindowFlags f)
: QWidget( parent, f | Qt::FramelessWindowHint )
{
  //resize(400, 400);
  hide();

  QColor colorBright(255,255,255);
  QColor colorDark(0,0,0);

  if (parentWidget() == 0) {
    this->setWindowOpacity( 0.7 );
    QPalette p = this->palette();
    p.setColor( QPalette::Dark, colorDark );
    this->setPalette(p);
    this->setBackgroundRole(QPalette::Dark);
  } else { 
    this->setAttribute( Qt::WA_ContentsPropagated, true );
    #if (QT_VERSION >= 0x040100)    
    this->setAutoFillBackground(false);
    #endif
  }
}

bool DBarWidget::isEmpty() const {
  return true;
}

void DBarWidget::paintEvent(QPaintEvent *) {

  int side = qMax(width(), height());

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // draw rounded background only if widget is parented
  if (parentWidget() != 0) {
    #if (QT_VERSION < 0x040200) || defined(Q_WS_X11)
    painter.setOpacity( 1.0 );
    #else
    painter.setOpacity( 0.7 );
    #endif
    QColor dc = colorDark;
    dc.setAlpha(100);
    painter.setPen( dc );
    painter.setBrush( dc );  
    painter.drawRoundRect( QRectF(0.0, 0.0, width()-1, height()-1), 4, 4 );
  }

}

void DBarWidget::resizeBasedOn(const QSize &s) {
  int side = qMin(s.width(), s.height());
  side /= 5.0;
  this->resize( side, side/2 );
}

void DBarWidget::showBasedOn(QWidget *parent, const QPoint &offset) {
  
  resizeBasedOn(parent->size());

  if (parent != 0) {
    int x = parent->width() - width() - 40; 
    int y = parent->height() - height() - 40; 
    if (parentWidget() != 0)
      this->move( x+offset.x(), y+offset.y() );
    else
      this->move( parent->x()+x+offset.x(), parent->y()+y+offset.y() );
  }

  this->show();
}

void DBarWidget::resizeEvent(QResizeEvent *) {
  if (parentWidget() != 0) return;
  QBitmap msk( size() );
  msk.clear();

  QPainter painter(&msk);
  painter.setPen(Qt::color1);
  painter.setBrush(Qt::color1);  
  painter.drawRoundRect( QRectF(0.0, 0.0, width()-1, height()-1), 8, 8 );

  setMask(msk);
}

void DBarWidget::mouseMoveEvent( QMouseEvent *event ) {
  if (in_mouse_move) {
    this->move( this->pos() + ( event->pos() - mousePoint ) );
    base_offset += ( event->pos() - mousePoint );
  }
}

void DBarWidget::mousePressEvent( QMouseEvent *event ) {
  mousePoint = event->pos();
  in_mouse_move = true;
}

void DBarWidget::mouseReleaseEvent( QMouseEvent *event ) {
  in_mouse_move = false;
}

//*****************************************************************************
// DMetaDataBar
//*****************************************************************************

DMetaDataBar::DMetaDataBar(QWidget *parent, Qt::WindowFlags f)
: DBarWidget( parent, f | Qt::FramelessWindowHint )
{
  metadata = "";
  /*
  tb = new QTextBrowser(this);
  tb->setWordWrapMode( QTextOption::WrapAtWordBoundaryOrAnywhere );
  tb->setReadOnly(true);  
  tb->setAutoFillBackground( false );
  tb->viewport()->setAutoFillBackground( false );
  */
}

void DMetaDataBar::setMetadata( const QString &m ) {
  metadata = m;
  //metadata.replace(QRegExp("<i>([^<]*)</i>"), "\\emph{\\1}");
  metadata.replace(QRegExp("<\\s*br\\s*>"), "\n");
  metadata.replace(QRegExp("<\\s*br\\s*/>"), "\n");
  metadata.replace(QRegExp("</\\s*[pP]\\s*>"), "\n");
  metadata.replace(QRegExp("<\\s*[hH]\\d\\s*>"), "\n\n");
  metadata.replace(QRegExp("</\\s*[hH]\\d\\s*>"), "\n");
  metadata.replace(QRegExp("<\\w+>"), "");
  metadata.replace(QRegExp("</\\w+>"), "");
  //tb->setHtml(m);
}

bool DMetaDataBar::isEmpty() const {
  return (metadata.size() == 0);
}

void DMetaDataBar::paintEvent(QPaintEvent *e) {
  
  DBarWidget::paintEvent(e);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);
  #if (QT_VERSION >= 0x040200)    
  painter.setOpacity( 1.0 );
  #endif

  if (metadata.size() > 0) {
    painter.setPen( colorBright );
    int font_size = qMax(width(),height())/50.0;
    painter.setFont(QFont("Helvetica [Cronyx]", font_size, QFont::Bold));
    painter.drawText(QRect(10,10,this->width()-20, this->height()-20), Qt::AlignLeft, metadata);
  }

}

void DMetaDataBar::resizeBasedOn(const QSize &s) {
  int w = s.width()*2.5/10.0;
  int h = s.height()*7.0/10.0;
  this->resize( w, h );
}

void DMetaDataBar::showBasedOn(QWidget *parent, const QPoint &offset) {
  
  resizeBasedOn(parent->size());

  if (parent != 0) {
    int x = parent->width() - width() - 40; 
    int y = parent->y() + 40; 
    if (parentWidget() != 0)
      this->move( x+offset.x(), y+offset.y() );
    else
      this->move( parent->x()+x+offset.x(), parent->y()+y+offset.y() );
  }

  this->show();
}

