/*******************************************************************************
  
  DScrollImageView is an image pixel view
  
  This widget creates a virtual painting area, the fixed qwidget "view" is used for painting into
  although the scrollbars will define the necessary translation which will give virtual huge window
  that dispays the image in order to overcome qt's problem in painitng into widgets larger than 
  32000 pixels... (although it should be a bug)
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  Acknowledgemnts: Special thanks to Mike Quinn for constant borrowing of his 
                   nice linux boxes for testing!!!

  History:
    12/08/2006 18:09 - First creation
    2007-10-09 16:46 - centering of small images
      
  ver: 6
       
*******************************************************************************/


#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include "qresource.h"

#include <BioImageCore>

#include <appconfig.h>
#include "wvimagemanager.h"

#include "scrollImageView.h"
#include "tileviewer.h"


// following two blocks of includes are needed for filtering native messages
// in slow painting windget
#if defined(Q_WS_WIN)
#include <windows.h>
UINT WM_QT_REPAINT = RegisterWindowMessageA("WM_QT_REPAINT");
#endif

#if defined(Q_WS_X11)
  #include <QtGui/qx11info_x11.h>
  #if !defined(_XLIB_H_)
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
  #endif
#endif

// define WV_PAINT_DEBUG in order to generate colorful blocks corresponding to each paint event
// instead of actual image information, only needed for debuging painting
//#define WV_PAINT_DEBUG

// define WV_PAINT_DEBUG_SLOW in order to simulate slow painting device like a DMX wall
//#define WV_PAINT_DEBUG_SLOW

// define WV_PAINT to appropriate painting function: "repaint" or "update"
// with "repaint" Qt will force immediate screen action
// with "update"  Qt will enqueue painting event and execute when the time comes:) 
#define WV_PAINT update
//#define WV_PAINT repaint

#define WV_PAINT_TILE_SIDE 1024 // 128 256 512 1024

// define WV_DO_PROGRESSIVE_PAINT to 'true' to force progressive paint at all moments
#define WV_DO_PROGRESSIVE_PAINT false

// 30' Apple Cinema HD Display: 2560 x 1600
// let's choose big monitor as the limit to start progressive updates
// progressive updates paint blocks of the screen progressively instead of generating 
// the whole view in off-screen buffer first and then transfering it to the screen
#define WV_BIG_W 4000
#define WV_BIG_H 3000

#ifdef WV_PAINT_DEBUG_SLOW
#if defined( WIN32 ) || defined( Q_WS_WIN )
  #include <windows.h>
  #include <process.h>
  #define getpid() _getpid()
#else
  #include <unistd.h>
  #include <sched.h>  
#endif 

inline void dim_sleep(int milisec) {
  #if defined( WIN32 ) || defined( Q_WS_WIN )
    Sleep(milisec);
  #else
    sleep(ceil(milisec/1000.0));
  #endif 
}
#endif // WV_PAINT_DEBUG_SLOW


DScrollImageView::DScrollImageView(QWidget *parent)
: QWidget(parent) {

  tv = NULL;
  im = NULL;
  graphics_layer = NULL;

  huge_screen = false;
  progressive_paint = WV_DO_PROGRESSIVE_PAINT; // false
  paint_tile_side   = WV_PAINT_TILE_SIDE; // 1024
  current_tool        = DScrollImageView::mtZoom;
  current_scroll_tool = DScrollImageView::mstZoom;

  QSize screen_size = DSysConfig::virtualScreenMaxSize();
  if ( screen_size.width()*screen_size.height() > WV_BIG_W*WV_BIG_H )
    huge_screen = true;

  // start progressive paint on huge displays
  if (huge_screen) progressive_paint = true;

  #ifdef Q_WS_X11
  // on X11 draw_image inside paint event is reflected instantaneously, making it
  // unnecessary simulate progressive paint events
  // i.e. on X11 it is always progressive, thus for small displays we have to force
  // tile size equal to the screen size
  progressive_paint = false;
  if ( !huge_screen ) {
    unsigned int max_monitor_side = qMax( screen_size.width(), screen_size.height() );
    paint_tile_side = max_monitor_side;
  }
  #endif

  view_rotation_angle = 0;
  center_small_images = true;
  centering_offset = QPoint(0,0);

  // --------------------------------------
  // create all view widget and scroll bars  
  // --------------------------------------
  setObjectName(QString::fromUtf8("ImageViewForm"));
  gridLayout = new QGridLayout(this);
  gridLayout->setSpacing(0);
  gridLayout->setMargin(0);
  gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
  horizontalScrollBar = new QScrollBar(this);
  horizontalScrollBar->setObjectName(QString::fromUtf8("horizontalScrollBar"));
  horizontalScrollBar->setOrientation(Qt::Horizontal);
  horizontalScrollBar->setTracking(true);

  gridLayout->addWidget(horizontalScrollBar, 1, 0, 1, 1);

  verticalScrollBar = new QScrollBar(this);
  verticalScrollBar->setObjectName(QString::fromUtf8("verticalScrollBar"));
  verticalScrollBar->setOrientation(Qt::Vertical);
  verticalScrollBar->setTracking(true);

  gridLayout->addWidget(verticalScrollBar, 0, 1, 1, 1);

  //view = new QWidget(this);
  view = new DSlowPaintWidget(this);
  view->setObjectName(QString::fromUtf8("widget"));
  view->installEventFilter(this);
  this->setMouseTracking(true);
  view->setFocusProxy(this);

  gridLayout->addWidget(view, 0, 0, 1, 1);

  connect(horizontalScrollBar, SIGNAL( valueChanged(int) ), this, SLOT( horizontalValueChanged(int) ));
  connect(verticalScrollBar, SIGNAL( valueChanged(int) ), this, SLOT( verticalValueChanged(int) ));

  // --------------------------------------
  // vars
  // --------------------------------------
  offset = QPointF(0,0);
  size_virtual = QSize(0,0);
  policy_vertical = Qt::ScrollBarAsNeeded;
  policy_horizontal = Qt::ScrollBarAsNeeded;
  ignore_scroll_signals = false;
  in_mouse_move = false;

  setVirtualViewSize( QSize(1,1) );

  QPixmap cursor_xmp( ( const char** ) arrow_cursor_128px_pixmap );
  cursor_arrow_large = QCursor(cursor_xmp, 9, 8);

  cursor_xmp = QPixmap( ( const char** ) sizeall_cursor_128px_pixmap );
  cursor_sizeall_large = QCursor(cursor_xmp, 64, 64);

  #if defined( Q_WS_X11 )
  extern void qt_x11_set_global_double_buffer(bool);
  #ifndef WV_EMBEDDED_CONTROLS
  qt_x11_set_global_double_buffer(false);
  #endif
  #endif

  setAcceptDrops(true);
  setEnabled(true);
  setFocus();
} 

void DScrollImageView::offsetViewTo(const QPointF &o, bool needs_full_repaint) {

  if ( imageFits() ) return;

  QPointF oadj;
  oadj.setX( dim::trim<double>(o.x(), 0, horizontalScrollBar->maximum() ) );
  oadj.setY( dim::trim<double>(o.y(), 0, verticalScrollBar->maximum() ) );

  QPointF d = offset-oadj; 
  if (!needs_full_repaint && d == QPointF(0,0)) return;
  offset = oadj;
  view->offset = offset.toPoint(); 

  // the 50% for full repaint are needed only beacuse we have to generate tiled paint events
  // ourselves and in case of scroll the paint event is generated for the whole region
  // that needs repaint and we'll have to paint it to the screen at once
  //if (fabs(d.x()) / view->width() >= 0.5) || (fabs(d.y()) / view->height()>= 0.5) needs_full_repaint = true;

  if ( needs_full_repaint || (im && im->needFullViewportPaint()) )
    repaintView();
  else
    view->scroll( d.x(), d.y() );


  // now adjust scroll bars to correct positions
  ignore_scroll_signals = true;
  horizontalScrollBar->setValue(offset.x());
  verticalScrollBar->setValue(offset.y());
  ignore_scroll_signals = false;

  emit viewAreaMoved( viewArea() );
}

void DScrollImageView::scrollViewBy(const QPointF &d) {
  offsetViewTo(offset+d);
}

void DScrollImageView::centerViewAt(const QPointF &nc, bool needs_full_repaint) {

  QSizeF half( view->width()/2.0, view->height()/2.0 );
  QPointF p( nc.x()-half.width(), nc.y()-half.height() );
  offsetViewTo( p, needs_full_repaint );
}

void DScrollImageView::updateOffset() {

  double x = 0;
  double y = 0;

  if (!imageWidthFits())
    x = horizontalScrollBar->value();

  if (!imageHeightFits())
    y = verticalScrollBar->value();

  offset = QPointF(x, y);
}

// returns the available space for drawing image
QRectF DScrollImageView::viewScreen() const {
  return QRectF( 0,0,view->width(),view->height() );
}

QRectF DScrollImageView::viewArea() const {
  return QRectF( viewOffset(), viewSize() );
}

QPointF DScrollImageView::viewOffset() const {
  return offset;
}

QSize DScrollImageView::viewSize() const {
  QRect vr( 0,0,view->width(),view->height() );
  QRect ir( 0,0,size_virtual.width(),size_virtual.height() );
  vr = vr.intersected( ir );
  return QSize( vr.width(), vr.height() );
}

QPointF DScrollImageView::viewCenter() const {
  QRect vr( 0,0,view->width(),view->height() );
  QRect ir( 0,0,size_virtual.width(),size_virtual.height() );
  vr = vr.intersected( ir );
  return offset + QPointF( vr.width()/2.0, vr.height()/2.0 );
}

QPointF DScrollImageView::screenToView(const QPointF &p) const {
  return p + offset;
}

QPointF DScrollImageView::viewToScreen(const QPointF &p) const {
  return p - offset;
}

QPointF DScrollImageView::screenToImage(const QPointF &p) const {
  if (!im) return screenToView(p);
  return im->viewToImage( screenToView(p) );
}

QPointF DScrollImageView::imageToScreen(const QPointF &p) const {
  if (!im) return viewToScreen(p);
  return viewToScreen( im->imageToView(p) );
}

void DScrollImageView::setScrollBars() {

  ignore_scroll_signals = true;

  horizontalScrollBar->setMinimum(0);
  int mx = dim::trim<int>(size_virtual.width()-view->width(), 0, INT_MAX );
  horizontalScrollBar->setMaximum(mx);
  horizontalScrollBar->setPageStep( view->width() );

  /*
  if ( s.width()<=view->width() )
    horizontalScrollBar->setVisible( false );
  else
    horizontalScrollBar->setVisible( true );
    */

  verticalScrollBar->setMinimum(0);
  mx = dim::trim<int>(size_virtual.height()-view->height(), 0, INT_MAX );
  verticalScrollBar->setMaximum(mx);
  verticalScrollBar->setPageStep( view->height() );

  /*
  if ( s.height()<=view->height() )
    verticalScrollBar->setVisible( false );
  else
    verticalScrollBar->setVisible( true );
    */

  ignore_scroll_signals = false;
}

void DScrollImageView::setVirtualViewSize(const QSize &s, const QPointF &cent) {
  size_virtual = s;
  setScrollBars();
  if (im) view->zoom = im->zoomCurrent();  
  
  centerViewAt( cent, true ); 
  if (imageFits()) {
    offset = QPointF(0,0);
    repaintView();
  }
  emit viewAreaMoved( viewArea() );
}

void DScrollImageView::setHorizontalScrollBarPolicy ( Qt::ScrollBarPolicy p ) {

}

void DScrollImageView::setVerticalScrollBarPolicy ( Qt::ScrollBarPolicy p ) {

}

void DScrollImageView::horizontalValueChanged ( int value ) {
  if (ignore_scroll_signals) return;
  offsetViewTo( QPointF(value, offset.y()) );
}

void DScrollImageView::verticalValueChanged ( int value ) {
  if (ignore_scroll_signals) return;
  offsetViewTo( QPointF(offset.x(), value) );
}

void DScrollImageView::repaintView() { 
  if ( !im || im->imageEmpty()) {
    view->repaint();
    return;
  }
  if (!progressive_paint) {
    view->WV_PAINT();
    return;
  }
  
  // in case of progressive paint - simulated tiled paint events
  bool allow_interrupt = true;
  if ( longPaintTestAbort() ) allow_interrupt = false;

  if (allow_interrupt) longPaintBegin();
  QRect vr = viewScreen().toRect();
  for (int y=0; y<vr.height(); y+=paint_tile_side) {
    if ( allow_interrupt && longPaintTestAbort() ) break;
    for (int x=0; x<vr.width(); x+=paint_tile_side) {
       if ( allow_interrupt && longPaintTestAbort() ) break;
       QRect bound(x,y,paint_tile_side,paint_tile_side);
       bound = vr.intersected(bound);
       view->repaint( bound );
    } // for x
  } // for y
  if (allow_interrupt) longPaintEnd();
}


void DScrollImageView::mouseMoveEvent( QMouseEvent *event ) {

  if (time_prgs.elapsed() > 100 ) {
    QPointF viewp = viewOffset() + event->pos();
    emit cursorMoved( viewp.toPoint() - centering_offset );
    time_prgs.start();
  }

  if ( (current_tool == DScrollImageView::mtZoom  || (event->modifiers() & Qt::AltModifier) ) && in_mouse_move) {
    QPointF dp = event->pos() - mousePoint;

    QPointF c = viewOffset();
    offsetViewTo( c - dp );

    mouseBtn = Qt::NoButton; // we move now, forget zoom buttons 
    mousePoint = event->pos();
  }

  if (current_tool == DScrollImageView::mtDraw  && !(event->modifiers() & Qt::AltModifier) ) {
    if (graphics_layer) {
      QMouseEvent ge(event->type(), event->pos()+offset.toPoint()-centering_offset, event->button(),	event->buttons(), event->modifiers());
      graphics_layer->mouseMoveEvent( &ge );
    }
  }

}

void DScrollImageView::mousePressEvent( QMouseEvent *event ) {
  setFocus();  
  mousePoint = event->pos();
  in_mouse_move = true;
  mouseBtn = event->button(); 

  if (current_tool == DScrollImageView::mtZoom || (event->modifiers() & Qt::AltModifier) ) {
    setViewCursor( Qt::SizeAllCursor );
  }

  if (current_tool == DScrollImageView::mtDraw && !(event->modifiers() & Qt::AltModifier)) {
    if (graphics_layer) {
      QMouseEvent ge(event->type(), event->pos()+offset.toPoint()-centering_offset, event->button(),	event->buttons(), event->modifiers());
      graphics_layer->mousePressEvent( &ge );
    }
  }
}

void DScrollImageView::mouseReleaseEvent( QMouseEvent *event ) {
  setFocus();
  bool was_moving = in_mouse_move;
  in_mouse_move = false;

  QPointF offset = viewOffset();

  if (current_tool == DScrollImageView::mtZoom || (event->modifiers() & Qt::AltModifier)) {
    setViewCursor( Qt::ArrowCursor );
    if (mouseBtn == Qt::LeftButton) 
      emit zoomChangeRequest( 1, false, offset+event->pos() );
    else
    if (mouseBtn == Qt::RightButton) 
      emit zoomChangeRequest( -1, false, offset+event->pos() );
    else
    if ( was_moving /*&& im->zoomCurrent()>0*/ ) 
      repaintView();
  } // mtZoom

  if (current_tool == DScrollImageView::mtDraw && !(event->modifiers() & Qt::AltModifier)) {
    if (graphics_layer) {
      QMouseEvent ge(event->type(), event->pos()+offset.toPoint()-centering_offset, event->button(),	event->buttons(), event->modifiers());
      graphics_layer->mouseReleaseEvent( &ge );
    }
  }

}

void DScrollImageView::mouseDoubleClickEvent (QMouseEvent *event) {

  if (current_tool == DScrollImageView::mtDraw) {
    QMouseEvent ge(event->type(), event->pos()+offset.toPoint()-centering_offset, event->button(),	event->buttons(), event->modifiers());
    if (graphics_layer) graphics_layer->mouseDoubleClickEvent( &ge );
  }
}

void DScrollImageView::scrollViewByKey(const int x, const int y ) {
  QRectF r = viewArea();
  double dx = r.width() / 10.0;
  double dy = r.height() / 10.0; 
  scrollViewBy( QPointF( dx*x, dy*y) );
}

void DScrollImageView::keyPressEvent ( QKeyEvent *e ) {
  if (current_tool == DScrollImageView::mtDraw) {
    if (graphics_layer) graphics_layer->keyPressEvent( e );
  } else {
    switch ( e->key() ) {
       case Qt::Key_Left:  scrollViewByKey( -1, 0 ); e->accept(); break;
       case Qt::Key_Right: scrollViewByKey( 1, 0 );  e->accept(); break;
       case Qt::Key_Up:    scrollViewByKey( 0, -1 ); e->accept(); break;
       case Qt::Key_Down:  scrollViewByKey( 0, 1 );  e->accept(); break;
       default: tv->keyPressEvent( e );
    } // switch
  } // zoom tool
}

void DScrollImageView::keyReleaseEvent ( QKeyEvent *e) {
  if (current_tool == DScrollImageView::mtDraw) {
    if (graphics_layer) graphics_layer->keyReleaseEvent( e );
  } else
    tv->keyReleaseEvent( e );

}

void DScrollImageView::wheelEvent ( QWheelEvent *e ) {  

  QPointF offset = viewOffset();

  int numDegrees = e->delta() / 8;
  //int numSteps = numDegrees / 15;

  //if (current_tool == DScrollImageView::mtZoom) {
  if (e->orientation() == Qt::Vertical) {
    
    if (current_scroll_tool == DScrollImageView::mstZoom) {
      if ( e->delta()<0 ) 
        emit zoomChangeRequest( 1, false, offset+e->pos() );
      if ( e->delta()>0 )
        emit zoomChangeRequest( -1, false, offset+e->pos() );
    } // zoom scroll

    if (current_scroll_tool == DScrollImageView::mstPan) {
      QPointF c = QPointF(0, -1*numDegrees);
      scrollViewBy(c);
    } // pan scroll

  } // Vertical

  if (e->orientation() == Qt::Horizontal) {
    QPointF c = QPointF(-1*numDegrees, 0);
    scrollViewBy(c);
  } // Horizontal

}

void DScrollImageView::onGraphicsLayerChanged ( const QRegion &region ) {
  repaintView();
}

void DScrollImageView::onUpdateScreen() {
  repaintView();
}

void DScrollImageView::onSetViewCursor( WVCursorShape _shape ) {
  this->setViewCursor(_shape);
}

bool DScrollImageView::eventFilter(QObject *obj, QEvent *e) {

  if ( (obj == view) && (e->type() == QEvent::Paint) ) {
    doPaint( static_cast<QPaintEvent *>(e) );
    return true;
  }

  if ( (obj == view) && (e->type() == QEvent::Resize) ) {
    setScrollBars();
    updateOffset();
    emit viewAreaMoved( viewArea() );
    return true;
  }

  return false;
}

inline void DScrollImageView::longPaintBegin() {
  if ( tv ) tv->operationStart( false );
}

inline bool DScrollImageView::longPaintTestAbort() {
  bool res = true;
  if (tv) {
    res = tv->operationTestAbort();
  }
  return res;
}

inline void DScrollImageView::longPaintEnd() {
  if ( tv ) tv->operationStop();
}

static QList<QRect> bounded_rects( const QRegion &region, int side_w, int side_h ) {

  QList<QRect> list;
  QVector<QRect> rects = region.rects();

  for (int i=0; i<rects.size(); ++i) {

    QRect bound = rects[i];
    for (int y=bound.y(); y<bound.bottom(); y+=side_h)
      for (int x=bound.x(); x<bound.right(); x+=side_w) {
        QRect r(x,y,side_w,side_h);
        r = r.intersected(bound);
        list.append(r);
      }
  } // for i rects

  return list;
}

static unsigned int region_volume( const QVector<QRect> &rects ) {
  unsigned int vol=0; 
  for (int i=0; i<rects.size(); ++i) {
    QRect r = rects[i];
    vol += r.width() * r.height();
  }
  return vol;
}

#if defined(Q_WS_X11)
extern Drawable qt_x11Handle(const QPaintDevice *pd);
void convertToXImage(const QImage &img, XImage *ximage);
#endif

void DScrollImageView::doPaint( QPaintEvent *e ) {
  view->in_paint_event = true;

  int prev_zoom = im->zoomCurrent();
  QPointF prev_offset = offset;

  // if actual image is smaller than the available screen size then first remove areas
  // not covered by the image from the repaint region and then paint the background color
  // the rest of available area
  QRegion im_reg = e->region();
  centering_offset = QPoint(0,0);

  if ( !im || im->imageEmpty()) size_virtual = QSize(0,0);
  if ( size_virtual.width()<view->width() || size_virtual.height()<view->height() ) {

    if (center_small_images) {
      if ( size_virtual.width()<view->width() )   centering_offset.setX( (view->width()-size_virtual.width()) / 2 );
      if ( size_virtual.height()<view->height() ) centering_offset.setY( (view->height()-size_virtual.height()) / 2 );
    }

    //QRect virt(0,0,size_virtual.width(),size_virtual.height());
    QRect virt(centering_offset.x(), centering_offset.y(), size_virtual.width(), size_virtual.height());
    im_reg = e->region().intersected( QRegion(virt) );
    QRegion bg_reg = e->region().subtracted( QRegion(virt) );

    QColor bg_color(128,128,128);
    QPainter p(view);
    QVector<QRect> rects = bg_reg.rects();
    for (int i=0; i<rects.size(); ++i)
      p.fillRect( rects[i], QBrush(bg_color, Qt::SolidPattern) );
    p.end();
  }

  //------------------------------------
  // now actually paint the image
  //------------------------------------
  QList<QRect> rects = bounded_rects( im_reg, paint_tile_side, paint_tile_side );
  bool allow_interrupt = false;
  #ifdef Q_WS_X11
  if (huge_screen) allow_interrupt = longPaintTestAbort();
  #endif

  QPainter p(view);

  // we havea special treatment for X11 to make fast paintings with 16bpp
  // qt painting will abolished and direct X commands will be used instead
  #if defined(Q_WS_X11)
  QX11Info x11i = view->x11Info();
  Drawable d = qt_x11Handle( p.device() );
  GC gc = XCreateGC( (Display *) x11i.display(), d, 0, 0);
  #endif


  if (allow_interrupt) longPaintBegin();
  if (!im->imageEmpty())
  while ( rects.size() > 0 ) {

    if (allow_interrupt && longPaintTestAbort() ) break;
    if (allow_interrupt && prev_zoom != im->zoomCurrent()) break;
    if (allow_interrupt && prev_offset != offset) break;

    // painting debug - paint random colors
    #ifdef WV_PAINT_DEBUG
    double v;
    v = (double) qrand() / (double) RAND_MAX;
    int rc = v * 255;
    v = (double) qrand() / (double) RAND_MAX;
    int gc = v * 255;
    v = (double) qrand() / (double) RAND_MAX;
    int bc = v * 255;
    #endif

    QRect r = rects[0];
    QRectF draw_rect(0,0,r.width(),r.height());

    #ifdef WV_PAINT_DEBUG_SLOW
    int sleep_time = r.width()*r.height() / 250;
    dim_sleep( sleep_time );
    #endif

    #ifdef WV_PAINT_DEBUG
    p.fillRect( r, QBrush(QColor(rc,gc,bc), Qt::SolidPattern) );
    #else
    //QRect imr = r.translated( offset.x(), offset.y() );
    QRect imr = r.translated( offset.x()-centering_offset.x(), offset.y()-centering_offset.y() );
    
    #if !defined(Q_WS_X11)
    QImage *img = im->getDisplayRoi(imr);
    p.drawImage( r, *img, draw_rect, Qt::ColorOnly );
    #else
    // X11 specific painting code

    if (x11i.depth() != 16) {
      // fall back to qt cross platform implementation
      QImage *img = im->getDisplayRoi(imr);
      p.drawImage( r, *img, draw_rect, Qt::ColorOnly );       
    } else { // if X server running on 16 bits
      // here we have to get 16 bit image
      QImage *img = im->getDisplayRoi(imr);
      unsigned int w = imr.width();
      unsigned int h = imr.height();
      XImage *xi = XCreateImage( (Display *) x11i.display(), (Visual *) x11i.visual(), 
                                16, ZPixmap,
                                0, (char *) img->bits(), 
                                w, h, 16, img->bytesPerLine() );

      // just to test let's make a slower function that converts, later we would call
      // a variation of getDisplayRoi for X11 16 bits that will generate a buffer directly
      QImage temp_img = img->copy( draw_rect.toRect() );
      convertToXImage( temp_img, xi );

      XPutImage( (Display *) x11i.display(), d, gc, xi, 0, 0, r.x(), r.y(), r.width(), r.height() );
      xi->data = 0; // QImage owns these bits);
      XDestroyImage(xi);
    }
    #endif // X11 specific

    #endif // no debug code
    
    rects.removeFirst();
  } // while rects

  // X11 specific painting code  
  #if defined(Q_WS_X11)
  XFreeGC( (Display *) x11i.display(), gc );
  #endif

  // if some painting commands were not executed, in zoom case - forget all
  // in move case, offset and then enqueue
  if (rects.size()>0 && prev_offset != offset) {
    QRegion rgn( rects[0] );
    for (int i=1; i<rects.size(); ++i)
      rgn = rgn.united( rects[i] );
    QPointF d = offset - prev_offset;
    rgn.translate( d.toPoint() );
    view->update( rgn );
  }

/*
  // now paint all pendingevents
  if (!im->imageEmpty())
  while ( view->pending_paints.size() > 0 ) {
    
    //view->setAttribute(Qt::WA_UpdatesDisabled, true);
    //QApplication::processEvents();
    //view->setAttribute(Qt::WA_UpdatesDisabled, false);

    //if (allow_interrupt && longPaintTestAbort() ) break;
    //if (allow_interrupt && prev_zoom != im->zoomCurrent()) break;
    //if ( allow_interrupt && longPaintTestAbort() ) break;;

    // painting debug - paint random colors
    //#ifdef WV_PAINT_DEBUG
    double v;
    v = (double) qrand() / (double) RAND_MAX;
    int rc = v * 255;
    v = (double) qrand() / (double) RAND_MAX;
    int gc = v * 255;
    v = (double) qrand() / (double) RAND_MAX;
    int bc = v * 255;
    //#endif

    DPendingPaintRect pp = view->pending_paints[0];
    QRect r = pp.rect;
    QRectF draw_rect(0,0,r.width(),r.height());

    //#ifdef WV_PAINT_DEBUG
    r = QRect( 0,0, view->width(), view->height() );
    p.fillRect( r, QBrush(QColor(rc,gc,bc), Qt::SolidPattern) );
    //#else
    //QRect imr = r.translated( offset.x(), offset.y() );
    //QImage *img = im->getDisplayRoi(imr);
    //p.drawImage( r, *img, draw_rect, Qt::ColorOnly );
    //if (graphics_layer) graphics_layer->paint( &p, imr );
    //#endif

    printf("now drawing pendning [%d,%d,%d,%d]\n", r.x(), r.y(), r.width(), r.height());
    fflush(stdout);

    view->pending_paints.removeFirst();
     
  }

*/


  // now paint graphics layer stuff
  if ( im && !im->imageEmpty() && graphics_layer ) {

    QMatrix graphicsMatrix; 
    double sx = 1.0;
    if (im) sx = pow( 2.0, im->zoomCurrent() );
    //graphicsMatrix.translate( -offset.x(), -offset.y() );
    graphicsMatrix.translate( -offset.x()+centering_offset.x(), -offset.y()+centering_offset.y() );
    graphicsMatrix.scale(sx, sx);
    p.setMatrix(graphicsMatrix);

    //gscene rotation-----------------------------
    graphicsMatrix.rotate(view_rotation_angle);

    int res = view_rotation_angle % 360;
    int w = im->imageHeight();
    int h = im->imageWidth();
    if( res == 90 || res == -270 )
    {
      graphicsMatrix.translate( 0, -h );
    }
    else if( res == 180 || res == -180 )
    {
      graphicsMatrix.translate( -w, -h );
    }
    else if( res == 270 || res == -90 )
    {
      graphicsMatrix.translate( -w, 0 );
    }
    
    //----------------------------------------------

	  QRegion rgn(this->viewArea().toRect());
	  graphics_layer->paint( &p, rgn );
    //graphics_layer->paint( &p, e->region() );
  }

  if (allow_interrupt) longPaintEnd();
  view->in_paint_event = false;
}

void DScrollImageView::setUseLargeCursors( bool u) {
  use_large_cursors = u;
  setViewCursor( view->cursor().shape() );
}

void DScrollImageView::setViewCursor( WVCursorShape _shape ) {

  if (_shape == Qt::ArrowCursor) {
    if (use_large_cursors)
      view->setCursor( cursor_arrow_large );
    else
      view->unsetCursor();
    return;
  }

  if (_shape == Qt::SizeAllCursor) {
    if (use_large_cursors)
      view->setCursor( cursor_sizeall_large );
    else
      view->setCursor( _shape );
    return;
  }

  view->setCursor( _shape );
}


//*****************************************************************************
// DSlowPaintWidget
//*****************************************************************************

DSlowPaintWidget::DSlowPaintWidget(QWidget *parent)
: QWidget(parent) {
  
  in_paint_event = false;
  offset = QPoint(0,0);
  zoom = 0;

  // Indicates that the widget has no background, i.e. when the widget receives paint events,
  // the background is not automatically repainted. Note: Unlike WA_OpaquePaintEvent, 
  // newly exposed areas are never filled with the background (e.g after showing a window 
  // for the first time the user can see "through" it until the application processes the 
  // paint events). Setting this flag implicitly disables double buffering for the widget.
  setAttribute( Qt::WA_NoSystemBackground, true );
  
  //Indicates that the widget contents are north-west aligned and static. On resize, 
  //such a widget will receive paint events only for the newly visible part of itself.
  setAttribute( Qt::WA_StaticContents, true );
  
  //Indicates that the widget wants to draw directly onto the screen. Widgets with this
  // attribute set do not participate in composition management, i.e. they cannot be 
  //semi-transparent or shine through semi-transparent overlapping widgets. This is only
  // supported on X11. The flag is set or cleared by the widget's author. This flag is 
  //required for rendering outside of Qt's paint system; 
  //e.g. if you need to use native X11 painting primitives.
  
  #ifdef Q_WS_X11
  // on X11 disable this feature, it will disable transparency of windows on top but
  // fix the wired bug of generating full screen repaint in qwidget::scroll
  setAttribute( Qt::WA_PaintOnScreen, true );
  #else
  setAttribute( Qt::WA_PaintOnScreen, false );
  #endif

  #if (QT_VERSION >= 0x040100)
  // Indicates that the widget paints all its pixels when it receives a paint event. 
  //It is thus not required for operations like updating, resizing, scrolling and focus 
  //changes to call erase the widget before generating paint events. Using WA_OpaquePaintEvent
  // is a small optimization. It can help to reduce flicker on systems that do not provide 
  //double buffer support, and it avoids the computational cycles necessary to erase the 
  //background prior to paint. Note: Unlike WA_NoSystemBackground, WA_OpaquePaintEvent 
  //makes an effort to avoid transparent window backgrounds. 
  //setAttribute(Qt::WA_OpaquePaintEvent, true);
  #else
  //This value is obsolete. Use WA_OpaquePaintEvent instead.
  //setAttribute( Qt::WA_NoBackground, true ); 
  #endif

  setAttribute( Qt::WA_PaintOutsidePaintEvent, false ); 
  setAttribute( Qt::WA_PaintUnclipped, false ); 

  setMouseTracking(true);
  setAcceptDrops(true);
  setContextMenuPolicy(Qt::PreventContextMenu);
  setEnabled(true);
  setFocus();
}

#if defined(Q_WS_X11)
bool DSlowPaintWidget::x11Event(XEvent *event) {

  // do not interpret any other messages, return event for further procssesing
  if (event->type != Expose && event->type != GraphicsExpose) return false;

  /*
  if (this->in_paint_event && event->type == Expose) {
    //qWarning("discarding event...");
    return true;
  }
  */

  QRect  paintRect(event->xexpose.x, event->xexpose.y,
                   event->xexpose.width, event->xexpose.height);

  if (this->in_paint_event) {
    //pending_paints.append( DPendingPaintRect( paintRect, offset, zoom ) );
    update( QRegion(paintRect) );
    //qWarning(" event postponed...");
    return true;
  }

  return false;
}
#endif

#if defined(Q_WS_WIN)
/*
bool DSlowPaintWidget::winEvent(MSG *msg, long *result) {

  // do not interpret any other messages, return event for further procssesing
  if (msg->message != WM_PAINT && msg->message != WM_QT_REPAINT) return false;

  return false;
}
*/
#endif

//*****************************************************************************
// Additional funcs needed
//*****************************************************************************

#if defined(Q_WS_X11)
/*
The followinf portion of the code was partially taken from KDE kpixmapio.cpp
Therefore this portion of the code MUST to be GPLed
Next code iteration should move this code directly to buffer generation part
saving time on buffer copies
*/
enum ByteOrders {
	bo32_ARGB, bo32_BGRA, bo24_RGB, bo24_BGR,
	bo16_RGB_565, bo16_BGR_565, bo16_RGB_555,
	bo16_BGR_555, bo8
};

static int lowest_bit(uint val) {
  int i;
  uint test = 1;
  for (i=0; (!(val & test)) && i<32; i++, test<<=1);
  return (i == 32) ? -1 : i;
}

static int getByteOrder( XImage *ximage ) {
  int byteorder = 0;
  int bpp       = ximage->bits_per_pixel;
  if (ximage->byte_order == LSBFirst) bpp++;

  int red_shift   = lowest_bit(ximage->red_mask);
  int green_shift = lowest_bit(ximage->green_mask);
  int blue_shift  = lowest_bit(ximage->blue_mask);

  if ((bpp == 32) && (red_shift == 16) && (green_shift == 8) && (blue_shift == 0))
    byteorder = bo32_ARGB;
  else if ((bpp == 32) && (red_shift == 0) && (green_shift == 8) && (blue_shift == 16))
    byteorder = bo32_BGRA;
  else if ((bpp == 33) && (red_shift == 16) && (green_shift == 8) && (blue_shift == 0))
    byteorder = bo32_BGRA;
  else if ((bpp == 24) && (red_shift == 16) && (green_shift == 8) && (blue_shift == 0))
    byteorder = bo24_RGB;
  else if ((bpp == 24) && (red_shift == 0) && (green_shift == 8) && (blue_shift == 16))
    byteorder = bo24_BGR;
  else if ((bpp == 25) && (red_shift == 16) && (green_shift == 8) && (blue_shift == 0))
    byteorder = bo24_BGR;
  else if ((bpp == 16) && (red_shift == 11) && (green_shift == 5) && (blue_shift == 0))
    byteorder = bo16_RGB_565;
  else if ((bpp == 16) && (red_shift == 10) && (green_shift == 5) && (blue_shift == 0))
    byteorder = bo16_RGB_555;
  else if ((bpp == 17) && (red_shift == 11) && (green_shift == 5) && (blue_shift == 0))
    byteorder = bo16_BGR_565;
  else if ((bpp == 17) && (red_shift == 10) && (green_shift == 5) && (blue_shift == 0))
    byteorder = bo16_BGR_555;
  else if ((bpp == 8) || (bpp == 9))
    byteorder = bo8;

  return byteorder;
}

void convertToXImage(const QImage &img, XImage *ximage)
{
  int x, y;
  int width     = ximage->width;
  int height    = ximage->height;
  int bpl       = ximage->bytes_per_line;
  int byteorder = getByteOrder( ximage );
  char *data    = ximage->data;
  int bpp       = ximage->bits_per_pixel;
  if (ximage->byte_order == LSBFirst) bpp++;

  switch (byteorder) {

  case bo16_RGB_555:
  case bo16_BGR_555:

    if (img.depth() == 32)
    {
      QRgb *src, pixel;
      qint32 *dst, val;
      for (y=0; y<height; y++)
      {
        src = (QRgb *) img.scanLine(y);
        dst = (qint32 *) (data + y*bpl);
        for (x=0; x<width/2; x++)
        {
          pixel = *src++;
          val = ((pixel & 0xf80000) >> 9) | ((pixel & 0xf800) >> 6) |
            ((pixel & 0xff) >> 3);
          pixel = *src++;
          val |= (((pixel & 0xf80000) >> 9) | ((pixel & 0xf800) >> 6) |
            ((pixel & 0xff) >> 3)) << 16;
          *dst++ = val;
        }
        if (width%2)
        {
          pixel = *src++;
          *((qint16 *)dst) = ((pixel & 0xf80000) >> 9) |
            ((pixel & 0xf800) >> 6) | ((pixel & 0xff) >> 3);
        }
      }
    }
    break;

  case bo16_RGB_565:
  case bo16_BGR_565:

    if (img.depth() == 32)
    {
      QRgb *src, pixel;
      qint32 *dst, val;
      for (y=0; y<height; y++)
      {
        src = (QRgb *) img.scanLine(y);
        dst = (qint32 *) (data + y*bpl);
        for (x=0; x<width/2; x++)
        {
          pixel = *src++;
          val = ((pixel & 0xf80000) >> 8) | ((pixel & 0xfc00) >> 5) |
            ((pixel & 0xff) >> 3);
          pixel = *src++;
          val |= (((pixel & 0xf80000) >> 8) | ((pixel & 0xfc00) >> 5) |
            ((pixel & 0xff) >> 3)) << 16;
          *dst++ = val;
        }
        if (width%2)
        {
          pixel = *src++;
          *((qint16 *)dst) = ((pixel & 0xf80000) >> 8) |
            ((pixel & 0xfc00) >> 5) | ((pixel & 0xff) >> 3);
        }
      }
    }
    break;

  }
}

#endif // Q_WS_X11





