/*******************************************************************************

  DProgressCircle is "stay on top" transparent progress qwidget
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
       
*******************************************************************************/

#include <QtGui>

#include "xtypes.h"

#include "progresscircle.h"

#define COLOR_BRIGHT 255
#define COLOR_DIM    70
#define COLOR_DARK   0

DProgressCircle::DProgressCircle(QWidget *parent, Qt::WindowFlags f)
//: QWidget( parent, Qt::Window | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint )
: QWidget( parent, f | Qt::FramelessWindowHint )
{
  colors = QVector<unsigned char>(12, COLOR_DIM);
  progress_val_prev = 0;
  setWindowTitle(tr("Progress"));

  //resize(400, 400);
  hide();
  this->setAttribute( Qt::WA_StaticContents, true );

  if (parentWidget() == 0) {
    this->setWindowOpacity( 0.7 );
    QPalette p = this->palette();
    p.setColor( QPalette::Dark, QColor(COLOR_DARK, COLOR_DARK, COLOR_DARK) );
    this->setPalette(p);
    this->setBackgroundRole(QPalette::Dark);
  } else { 
    this->setAttribute( Qt::WA_ContentsPropagated, true );
    #if (QT_VERSION >= 0x040100)
    this->setAutoFillBackground(false);
    #endif
  }

}

void DProgressCircle::setProgress( int progress, const QString &s ) {

  progress = dim::trim<int>(progress, 0, 100);
  int val = dim::round<int>( progress / 100.0 * 12.0 );

  for (int i = 0; i < 12; ++i)
    colors[i] = COLOR_DIM;

  for (int i = 0; i < val; ++i)
    colors[i] = COLOR_BRIGHT;

  str = s;
  if (progress_val_prev != val) {
    update();
    progress_val_prev = val;
  }
}

void DProgressCircle::setText( const QString &s ) {
  str = s;
}

void DProgressCircle::paintEvent(QPaintEvent *) {

  int side = qMin(width(), height());

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  // draw rounded background only if widget is parented
  if (parentWidget()) {
    #if (QT_VERSION >= 0x040200)    
    painter.setOpacity( 0.8 );
    #endif
    painter.setPen( QColor(COLOR_DARK, COLOR_DARK, COLOR_DARK, 200) );
    painter.setBrush( QColor(COLOR_DARK, COLOR_DARK, COLOR_DARK, 200) );  
    painter.drawRoundRect( QRectF(0.0, 0.0, width()-1, height()-1), 8, 8 );
  }
  
  #if (QT_VERSION >= 0x040200)    
  painter.setOpacity( 1.0 );
  #endif

  painter.translate(width() / 2.0, height() / 2.0);
  painter.scale(side / 200.0, side / 200.0);
  painter.rotate(-90.0);

  for (int i = 0; i < 12; ++i) {
    QColor c( colors[i], colors[i], colors[i], 255 );
    painter.setPen( QPen(c, 14, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin ) );
    painter.drawLine(50.0, 0, 75.0, 0);
    painter.rotate(30.0);
  }

  if (str.size() > 0) {
    painter.rotate(90.0);
    painter.setPen( QColor( COLOR_BRIGHT, COLOR_BRIGHT, COLOR_BRIGHT, 255 ) );
    painter.setFont(QFont("Helvetica [Cronyx]", 40, QFont::Bold));
    QRectF r( -15,-15, 30, 30 );
    painter.drawText(r, Qt::AlignCenter, str);
  }
}

void DProgressCircle::showBasedOn(QWidget *parent, const QPoint &offset) {
  int side = qMin(parent->width(), parent->height());
  side /= 5.0;
  if (parent != 0) {
    this->resize( side, side );
    int half_w = parent->width()/2.0 - side/2.0; 
    int half_h = parent->height()/2.0 - side/2.0;
    
    if (parentWidget() != 0)
      this->move( half_w+offset.x(), half_h+offset.y() );
    else
      this->move( parent->x()+half_w+offset.x(), parent->y()+half_h+offset.y() );
  }
  setProgress( 0 );
  this->show();
}

void DProgressCircle::resizeEvent(QResizeEvent *) {
  if (parentWidget() != 0) return;
  QBitmap msk( size() );
  msk.clear();

  QPainter painter(&msk);
  painter.setPen(Qt::color1);
  painter.setBrush(Qt::color1);  
  painter.drawRoundRect( QRectF(0.0, 0.0, width()-1, height()-1), 20, 20 );

  setMask(msk);
}