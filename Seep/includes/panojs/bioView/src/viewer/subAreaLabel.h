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

#ifndef SUBAREALABEL_H
#define SUBAREALABEL_H

#include <QtGui>

class DGraphicsLayer;

class DSubAreaLabel : public QLabel {
    Q_OBJECT

public:
  DSubAreaLabel(QWidget *parent = 0);

  void init( const QSize &orig_size, const QRectF &r );

  // shows area visible in Original Image coordinates
  void updateVisArea( const QRectF &r );
  void setVisAreaShow( bool show );
  void setAreaTracking( bool track );
  QSize imageSize() { return orig_size; }

  QPointF viewToImage( const QPointF & );

  void setGraphicsLayer ( DGraphicsLayer *_glayer );

signals:
  void areaPositionChanged( const QPointF &center );
  // the difference of this two signals is: "Selected" is only emitted when the button is released
  void areaPositionSelected( const QPointF &center );
  void cursorPositionChanged( const QPointF &center );
  void toolButtonPressed( const QPointF &center, Qt::MouseButton  );

protected slots:
  void onGraphicsLayerChanged ( const QRegion &region ) { update(); }

private:
  QSize orig_size;
  void setScaleFromOriginalImage( const QSize &orig_img_size );
  QPointF thumb_offset;
  QRectF  vis_rect;
  QRectF  mov_rect;

  DGraphicsLayer *graphics_layer;

  QRectF vis_area_rect;
  bool isVisAreaRectValid() const;
  double vis_area_scale;
  bool   vis_area_show;
  bool   in_mouse_move;
  bool   was_moving;
  bool   tracking;
  QPoint mousePoint;

  void   doPositionChanged( const QPointF & );
  void   doPositionChanged( const QPoint & );
  QPointF getOffset();
  void   setOnMoveRect( const QPoint & );

protected:
  void paintEvent ( QPaintEvent * );
  void mouseMoveEvent ( QMouseEvent * );
  void mousePressEvent ( QMouseEvent * );
  void mouseReleaseEvent ( QMouseEvent * );
  void enterEvent ( QEvent * );

  void keyPressEvent ( QKeyEvent * );
  void keyReleaseEvent ( QKeyEvent * );
};

#endif // SUBAREALABEL_H
