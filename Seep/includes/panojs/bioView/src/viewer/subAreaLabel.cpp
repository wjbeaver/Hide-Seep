/*******************************************************************************

  DSubAreaLabel is a specialized QLabel that shows small images, draws
  visualization regions on them and captures mouse events for
  movements  
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
    2007-03-22 18:08 - New signals
    2007-04-05 12:16 - Estimated area rect
      
  ver: 4
        
*******************************************************************************/

#include "subAreaLabel.h"
#include "graphicsLayer.h"

//------------------------------------------------------------------------------
// DSubAreaLabel
//------------------------------------------------------------------------------

DSubAreaLabel::DSubAreaLabel(QWidget *parent)
: QLabel(parent) {

  graphics_layer = NULL;
  orig_size = QSize(0,0);
  vis_area_rect   = QRectF(0,0,0,0);
  vis_area_scale  = 1.0;
  vis_area_show   = true;
  tracking        = true;
  setFocus();

  setMouseTracking( true );
}

bool DSubAreaLabel::isVisAreaRectValid() const {
  return ( (vis_area_rect.width()>0) && (vis_area_rect.height()>0) );
}

QPointF DSubAreaLabel::getOffset() {
  if (pixmap() == NULL || pixmap()->isNull()) return QPointF(0,0);
  double xoff = ( width()  - pixmap()->width()  ) / 2.0;
  double yoff = ( height() - pixmap()->height() ) / 2.0;
  return QPointF(xoff,yoff);
}

void DSubAreaLabel::setScaleFromOriginalImage( const QSize &orig_img_size ) { 
  orig_size = orig_img_size;
  double os = qMax( orig_img_size.width(), orig_img_size.height() );
  double ls = os;
  if ( pixmap() != NULL && !pixmap()->isNull() )
    ls = qMax( pixmap()->width(), pixmap()->height() );
  vis_area_scale = ls / os;
  thumb_offset = getOffset();
}

void DSubAreaLabel::updateVisArea( const QRectF &r ) {
  vis_area_rect = r; 
  repaint();
}

void DSubAreaLabel::setVisAreaShow( bool show ) { 
  vis_area_show = show; 
  repaint();
}

void DSubAreaLabel::init( const QSize &orig_size, const QRectF &r ) {
  bool s = vis_area_show;
  vis_area_show = false;
  setScaleFromOriginalImage( orig_size );
  updateVisArea( r );
  vis_area_show = s;
}

QRectF rectfScale( const QRectF &r, double scale ) {
  QRectF o = r;
  o.setTopLeft(o.topLeft() *= scale);
  o.setBottomRight(o.bottomRight() *= scale);
  return o;
}

QRectF rectfTranslate( const QRectF &r, const QPointF p ) {
  QRectF o = r;
  o.translate( p );
  return o;
}

void DSubAreaLabel::setGraphicsLayer ( DGraphicsLayer *_glayer ) { 
  graphics_layer = _glayer; 
  connect( graphics_layer, SIGNAL( changed(const QRegion &) ), this, SLOT( onGraphicsLayerChanged(const QRegion &) ));
}

void DSubAreaLabel::paintEvent ( QPaintEvent * event ) {
  QLabel::paintEvent(event);
  if ( !vis_area_show || !isVisAreaRectValid() ) return;

  thumb_offset = getOffset();
  QPainter p(this);
  p.setRenderHint(QPainter::Antialiasing, true);
 
  if (graphics_layer) {
    p.translate( thumb_offset );
    p.scale(vis_area_scale, vis_area_scale);
    graphics_layer->paint( &p, QRegion() );
    p.resetMatrix();
  }

  QRectF vr = rectfScale( vis_area_rect, vis_area_scale );
  vr.translate( thumb_offset );
  vis_rect = vr;

  p.setPen( QPen(Qt::red, 4, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );
  p.setBrush( Qt::NoBrush );
  p.drawRect(vr); 

  vr.setTopLeft( vr.topLeft() - QPointF(1,1) );
  vr.setBottomRight( vr.bottomRight() + QPointF(1,1) );
  p.setPen( QPen(Qt::yellow, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );
  p.setBrush( Qt::NoBrush );
  p.drawRect(vr); 

  // if drawing temporary rect
  if (in_mouse_move) {
    //Qt::DotLine Qt::DashLine
    p.setPen( QPen(Qt::yellow, 4, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin) );
    p.setBrush( Qt::NoBrush );
    p.drawRect(mov_rect); 
  }  

}

QPointF DSubAreaLabel::viewToImage( const QPointF &ip ) {
  QPointF p = ip;
  p -= thumb_offset;
  p /= vis_area_scale;
  return p;
}

// emits the signal of new location, providing actual image coordinates
// given the point in the label
void DSubAreaLabel::doPositionChanged( const QPointF &center_point ) {
  emit areaPositionChanged( viewToImage(center_point) ); 
}

void DSubAreaLabel::doPositionChanged( const QPoint &center_point ) {
  QPointF p = center_point;
  doPositionChanged( p );
  setFocus();
}

void DSubAreaLabel::mouseMoveEvent( QMouseEvent *event ) {
  was_moving = true;  
  //if (!was_moving) was_moving = mousePoint - event.pos();
  if ( in_mouse_move && tracking ) {
    this->setCursor( Qt::SizeAllCursor );
    doPositionChanged( event->pos() );
  } //else emit cursorPositionChanged( viewToImage(event->pos()) );

  // show predicted movement rectangle
  if ( in_mouse_move ) {
    setOnMoveRect( event->pos() );
    update();
  }

  emit cursorPositionChanged( viewToImage(event->pos()) );
}

void DSubAreaLabel::mousePressEvent( QMouseEvent *event ) {
  mousePoint = event->pos();
  in_mouse_move = true;
  was_moving = false;
  //doPositionChanged( event->pos() );
  setOnMoveRect( event->pos() );
}

void DSubAreaLabel::mouseReleaseEvent( QMouseEvent *event ) {
  in_mouse_move = false;
  this->unsetCursor();

  if (was_moving) {
    doPositionChanged( event->pos() );
    emit areaPositionSelected( viewToImage(event->pos()) ); 
    update();
  } else {
    emit toolButtonPressed( viewToImage(event->pos()), event->button() );
  }
} 

void DSubAreaLabel::keyPressEvent ( QKeyEvent * e ) {
  QPointF d(vis_rect.width()/10.0, vis_rect.height()/10.0);
  QPointF c = vis_rect.center();

  if (e->key() == Qt::Key_Left) {
    c -= QPointF(d.x(),0);
    emit areaPositionSelected( viewToImage(c) );
    e->accept();
    return;
  } else
  if (e->key() == Qt::Key_Right) {
    c += QPointF(d.x(),0);
    emit areaPositionSelected( viewToImage(c) );
    e->accept();
    return;
  } else
  if (e->key() == Qt::Key_Down) {
    c += QPointF(0, d.y());
    emit areaPositionSelected( viewToImage(c) );
    e->accept();
    return;
  } else
  if (e->key() == Qt::Key_Up) {
    c -= QPointF(0, d.y());
    emit areaPositionSelected( viewToImage(c) );
    e->accept();
    return;
  }

  e->ignore();
}

void DSubAreaLabel::keyReleaseEvent ( QKeyEvent * e ) {

}

void DSubAreaLabel::enterEvent ( QEvent * e ) {
  in_mouse_move = false;
}

void DSubAreaLabel::setOnMoveRect( const QPoint &p ) {
  mov_rect = QRectF(0,0,vis_rect.width(),vis_rect.height());
  mov_rect.moveCenter( p );
}