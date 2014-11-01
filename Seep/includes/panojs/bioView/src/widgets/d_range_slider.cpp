/*******************************************************************************

  DRangeSlider is a dual handle sliding sub-range selector, as output gives a 
  sub-range defined by two values two values of min and max, such as min<max, 
  selectd within a given range
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
       
*******************************************************************************/

#include <QtGui>
#include <BioImageCore>

#include "d_range_slider.h"

DRangeSlider::DRangeSlider(QWidget *parent, Qt::WindowFlags f)
: QWidget( parent, f )
{
  range_min = 0.0;
  range_max = 100.0;
  sub_min = 0.0;
  sub_max = 100.0;  

  handle_size = 10;
  y_pos = handle_size/2.0+2.0;
  x_lim_min = handle_size;
  x_lim_max = width()-handle_size;

  x_min = x_lim_min;
  x_max = x_lim_max;
  int_strings = false;
  int_strings = true;
}

void DRangeSlider::drawHandle(QPainter *p, int x, int y, int size) {
  
  QPointF handle_vertices[3] = { QPointF(0.0, 0.0), QPointF(0.0, 0.0), QPointF(0.0, 0.0) };
  
  double hs = size / 2.0;
  handle_vertices[0] = QPointF(x-hs, y+hs);
  handle_vertices[1] = QPointF(x, y-hs);
  handle_vertices[2] = QPointF(x+hs, y+hs);
  
  p->drawConvexPolygon(handle_vertices, 3);
}

void DRangeSlider::paintEvent(QPaintEvent *) {

  //int side = width();
  int line_thikness=2;

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing, true);

  QColor c = this->palette().text().color();
  painter.setPen( QPen(c, line_thikness, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin) );

  QLineF line( x_lim_min, y_pos, x_lim_max, y_pos);
  painter.drawLine(line);

  c = this->palette().dark().color();
  painter.setBrush( QBrush(c, Qt::SolidPattern) );
  drawHandle(&painter, x_min, y_pos, handle_size);

  c = this->palette().base().color();
  painter.setBrush( QBrush(c, Qt::SolidPattern) );
  drawHandle(&painter, x_max, y_pos, handle_size); 
  
  QString str;
  if (int_strings)
    str.sprintf("%.0f", sub_min);
  else
    str.sprintf("%.2f", sub_min);
  painter.drawText( QPoint(x_lim_min+handle_size, y_pos+2*handle_size), str );
  if (int_strings)
    str.sprintf("%.0f", sub_max);
  else
    str.sprintf("%.2f", sub_max);
  painter.drawText( QPoint(x_lim_max-handle_size-15, y_pos+2*handle_size), str );
}

void DRangeSlider::resizeEvent(QResizeEvent *) {
  y_pos = handle_size/2.0+2.0;

  x_lim_min = handle_size;
  x_lim_max = width()-handle_size;
  updateRangeCoordinates();
  repaint();
}

void DRangeSlider::mouseMoveEvent (QMouseEvent *e) {
  if (moving == homNone) return;

  QPoint pd(e->pos() - mousePoint);

  if (moving == homMin)
    x_min = dim::trim( x_min+pd.x(), x_lim_min, x_max-1);

  if (moving == homMax)
    x_max = dim::trim( x_max+pd.x(), x_min+1, x_lim_max);

  updateRangeValues();
  repaint();
  mousePoint = e->pos();
}

void DRangeSlider::mousePressEvent (QMouseEvent *e) {
  mousePoint = e->pos();
  if (e->pos().y()-y_pos > handle_size/2) return;
  int d1 = abs( e->pos().x()-x_min );
  int d2 = abs( e->pos().x()-x_max );
  if (d1 <= d2)
    moving = homMin;
  else
    moving = homMax;
}

void DRangeSlider::mouseReleaseEvent (QMouseEvent *) {
  moving = homNone;
}

void DRangeSlider::updateRangeValues() {

  double nval = (x_min - x_lim_min) / (double) (x_lim_max - x_lim_min);
  double sub_min_tmp = nval*(range_max-range_min) + range_min;

  nval = (x_max - x_lim_min) / (double) (x_lim_max - x_lim_min);
  double sub_max_tmp = nval*(range_max-range_min) + range_min;

  if (sub_min_tmp!=sub_min || sub_max_tmp!=sub_max) {
    sub_min = sub_min_tmp;
    sub_max = sub_max_tmp;
    emit valuesChanged();
    emit rangeChanged( sub_min, sub_max );
    emit sliderChanged( this );
  }
}

void DRangeSlider::updateRangeCoordinates() {

  double nval = (sub_min - range_min) / (double) (range_max-range_min);
  x_min = nval*(x_lim_max - x_lim_min) + x_lim_min;

  nval = (sub_max - range_min) / (double) (range_max-range_min);
  x_max = nval*(x_lim_max - x_lim_min) + x_lim_min;
}


