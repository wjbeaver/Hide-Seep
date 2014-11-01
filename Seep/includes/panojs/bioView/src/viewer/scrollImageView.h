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

#ifndef SCROLL_IMAGE_VIEW_H
#define SCROLL_IMAGE_VIEW_H

#include <Qt>
#include <QtCore>
#include <QtGui>

#include <BioImage>

#include "graphicsLayer.h"

#if defined(Q_WS_X11)
  //#if !defined(_XLIB_H_)
  //#include <X11/Xlib.h>
  //#endif
  #include <QtGui/qx11info_x11.h>
#endif

class WVImageManager;
class TileViewer;

typedef Qt::CursorShape WVCursorShape;

//*****************************************************************************
// DSlowPaintWidget
//*****************************************************************************

class DPendingPaintRect {
public:
  DPendingPaintRect( QRect r, QPoint o, int z ): rect(r), offset(o), zoom(z) { }

  QRect  rect;
  QPoint offset;
  int    zoom;
};

class DSlowPaintWidget : public QWidget {
  Q_OBJECT

public:
  DSlowPaintWidget(QWidget *parent = 0);

  bool   in_paint_event;
  QPoint offset;
  int    zoom;

  QList<DPendingPaintRect> pending_paints;

protected:

  #if defined(Q_WS_WIN) /*
  virtual bool winEvent(MSG *message, long *result); */
  #endif

  #if defined(Q_WS_X11)
  virtual bool x11Event(XEvent *e);
  #endif
};

//*****************************************************************************
// DScrollImageView
//*****************************************************************************

class DScrollImageView : public QWidget {
    Q_OBJECT

public:
  enum MouseTools {
    mtZoom=0, 
    mtDraw 
  };

  enum MouseScrollTool {
    mstZoom=0, 
    mstPan
  };

  DScrollImageView(QWidget *parent = 0);

  QRectF  viewScreen() const;  // returns the available space for drawing image
  QRectF  viewArea() const;    // returns the space occupied by the image
  QPointF viewOffset() const;
  QSize   viewSize() const;
  QPointF viewCenter() const;
  QPointF screenToView(const QPointF &) const;
  QPointF viewToScreen(const QPointF &) const;
  QPointF screenToImage(const QPointF &) const;
  QPointF imageToScreen(const QPointF &) const;

  void    setVirtualViewSize(const QSize &, const QPointF &cent = QPointF(0,0));

  void    scrollViewBy(const QPointF &);
  void    centerViewAt(const QPointF &, bool needs_full_repaint = false);
  void    offsetViewTo(const QPointF &, bool needs_full_repaint = false);
  void    scrollViewByKey(const int x, const int y );

  void    setHorizontalScrollBarPolicy ( Qt::ScrollBarPolicy );
  void    setVerticalScrollBarPolicy ( Qt::ScrollBarPolicy );

  void    setUseLargeCursors( bool );

  QWidget *viewport() { return static_cast<QWidget *>(view); }
  void    repaintView();

  void    setViewCursor    ( WVCursorShape _shape );
  void    setImageManager  ( WVImageManager *_im ) { im = _im; }
  void    setViewerParent  ( TileViewer *_tv ) { tv = _tv; }
  void    setGraphicsLayer ( DGraphicsLayer *_glayer ) { 
    graphics_layer = _glayer; 
    connect( graphics_layer, SIGNAL( changed(const QRegion &) ), this, SLOT( onGraphicsLayerChanged(const QRegion &) ));
  }

  void    setMouseTool     ( MouseTools nt ) { current_tool = nt; }
  MouseTools mouseTool     () const          { return current_tool; }

  void    setMouseScrollTool     ( MouseScrollTool nt ) { current_scroll_tool = nt; }
  MouseScrollTool mouseScrollTool     () const          { return current_scroll_tool; }

  int view_rotation_angle;

  void    setCenterSmallImages   ( bool c ) { if (center_small_images == c) return; center_small_images = c; this->repaintView(); }
  bool    centerSmallImages      () const          { return center_small_images; }
  QPoint  centeringOffset        () { return centering_offset; }

signals:
  void viewAreaMoved( const QRectF & );
  void zoomChangeRequest( int z, bool z_is_absolute, const QPointF &center );
  void cursorMoved( const QPoint & );

public slots:
  void onUpdateScreen();
  void onSetViewCursor( WVCursorShape _shape );

protected:
  bool eventFilter(QObject *obj, QEvent *);

  void mouseMoveEvent ( QMouseEvent * );
  void mousePressEvent ( QMouseEvent * );
  void mouseReleaseEvent ( QMouseEvent * );
  void mouseDoubleClickEvent (QMouseEvent * );
  void keyPressEvent ( QKeyEvent * );
  void keyReleaseEvent ( QKeyEvent *);
  void wheelEvent ( QWheelEvent *);  

protected:
  // gui
  QGridLayout *gridLayout;
  QScrollBar  *horizontalScrollBar;
  QScrollBar  *verticalScrollBar;
  //QWidget     *view;
  DSlowPaintWidget *view; 
  DGraphicsLayer *graphics_layer;

  // internals
  Qt::ScrollBarPolicy policy_vertical;
  Qt::ScrollBarPolicy policy_horizontal;

  QPointF  offset;
  QSize    size_virtual;
  bool     ignore_scroll_signals;    
  QPoint   mousePoint;
  bool     in_mouse_move;
  Qt::MouseButton mouseBtn;
  MouseTools current_tool;
  MouseScrollTool current_scroll_tool;

  QPoint   centering_offset;
  bool     center_small_images;

  bool     use_large_cursors;
  QCursor  cursor_arrow_large;
  QCursor  cursor_sizeall_large;

  int paint_tile_side;
  bool progressive_paint;
  bool huge_screen;
  
  QTime time_prgs;

  WVImageManager *im;
  TileViewer     *tv;

  void doPaint( QPaintEvent *e );
  void setScrollBars();
  void updateOffset();

  inline bool imageWidthFits() const {
    return size_virtual.width()<=view->width();
  }
  
  inline bool imageHeightFits() const {
    return size_virtual.height()<=view->height();
  }

  inline bool imageFits() const { return imageWidthFits() && imageHeightFits(); }

  inline void longPaintBegin();
  inline bool longPaintTestAbort();
  inline void longPaintEnd();

protected slots:
  
  void horizontalValueChanged ( int value );
  void verticalValueChanged ( int value );

  void onGraphicsLayerChanged ( const QRegion &region );

};



#endif // SCROLL_IMAGE_VIEW_H
