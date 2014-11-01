/*******************************************************************************
  
  DGraphicsLayer is a stub class for posterior inheritance
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    2008-03-17 12:10 - first implementation
      
  ver: 1
       
*******************************************************************************/

#ifndef D_GRAPHICS_LAYER_H
#define D_GRAPHICS_LAYER_H

#include <QtCore>
#include <QtGui>

class DGraphicsLayer : public QObject {
    Q_OBJECT

public:
  DGraphicsLayer(QWidget *parent = 0) : QObject(parent) {}

  virtual void paint ( QPainter *painter, const QRegion &region ) = 0;

signals:
  void changed( const QRegion &region );

public slots:
  virtual void mouseMoveEvent ( QMouseEvent * ) {}
  virtual void mousePressEvent ( QMouseEvent * ) {}
  virtual void mouseReleaseEvent ( QMouseEvent * ) {}
  virtual void mouseDoubleClickEvent (QMouseEvent *) {}
  virtual void keyPressEvent ( QKeyEvent * ) {}
  virtual void keyReleaseEvent ( QKeyEvent *) {}
  virtual void wheelEvent ( QWheelEvent *) {}
};

#endif // D_GRAPHICS_LAYER_H
