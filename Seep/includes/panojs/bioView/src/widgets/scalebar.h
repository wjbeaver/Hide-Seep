/*******************************************************************************

  DScaleBar is "stay on top" transparent scalebar
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
       
*******************************************************************************/

#ifndef SCALE_BAR_H
#define SCALE_BAR_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QTextBrowser>

#define D_RESIZE_MARGIN 5

class DScaleBar : public QWidget {
  Q_OBJECT

public:
  enum DMouseOperation { moNone, moMove, moResizeLeft, moResizeRight, moResizeTop, moResizeBot };

  DScaleBar(QWidget *parent = 0, Qt::WindowFlags f = 0);
  void showBasedOn(QWidget *parent = 0, const QPoint &offset = QPoint(0,0) );
  void resizeBasedOn(const QSize &);
  bool isEmpty() const; 
  QPoint barOffset() const { return base_offset; }

public slots:
  void setScale( double, const QString &units = "" );
  void setUnits( const QString &units = "" );
  void drawBar( QPainter *p );

protected:
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);
  void contextMenuEvent(QContextMenuEvent *);
  void mouseMoveEvent ( QMouseEvent * event );
  void mousePressEvent ( QMouseEvent * event );
  void mouseReleaseEvent ( QMouseEvent * event );

private:
  QAction *sizeAutoAct;
  QAction *size1Act;
  QAction *size5Act;
  QAction *size10Act;
  QAction *size20Act;
  QAction *size50Act;
  QAction *sizeManualAct;
  QAction *hideNumberAct;
  QAction *hideBackgroundAct;
  QAction *colorWhiteAct;
  QAction *colorGrayAct;
  QAction *colorBlackAct;
  void createActions();

private slots:
  void setSize( double v ) { physical_size = v; this->update(); }
  void setHideNumber( bool v ) { hide_number = v; this->update(); }
  void triggerHideNumber() { setHideNumber(!hide_number); }
  void setHideBackground( bool v ) { hide_background = v; this->update(); }
  void triggerHideBackground() { setHideBackground(!hide_background); }
  void setSize0() { setSize(0); }
  void setSize1() { setSize(1); }
  void setSize5() { setSize(5); }
  void setSize10() { setSize(10); }
  void setSize20() { setSize(20); }
  void setSize50() { setSize(50); }
  void setSizeManual();

  void setBrightColorBlack() { setBrightColor( QColor(0,0,0) ); }
  void setBrightColorGray() { setBrightColor( QColor(128,128,128) ); }
  void setBrightColorWhite() { setBrightColor( QColor(255,255,255) ); }
  void setBrightColor( const QColor &c ) { colorBright = c; this->update(); }  

private:
  DMouseOperation in_mouse_op;

  QPoint base_offset;
  QString units_str;
  double scale_x;
  double physical_size;
  QPoint mousePoint;
  void showContextMenu( const QPoint & );
  bool hide_number;
  bool hide_background;

  QColor colorBright;
  QColor colorDark;

  inline bool isInResizeTop(const QPoint & pos) const { return (pos.y() <= D_RESIZE_MARGIN); }
  inline bool isInResizeBot(const QPoint & pos) const { return (this->height()-pos.y() <= D_RESIZE_MARGIN); }
  inline bool isInResizeLeft(const QPoint & pos) const { return (pos.x() <= D_RESIZE_MARGIN); }
  inline bool isInResizeRight(const QPoint & pos) const { return (this->width()-pos.x() <= D_RESIZE_MARGIN); }
  inline bool isInResize(const QPoint & pos) const { 
    return (isInResizeTop(pos) || isInResizeBot(pos) || isInResizeLeft(pos) || isInResizeRight(pos)); }
};


//*****************************************************************************
// DBarWidget
// TODO: Add: HideIfNoData
//*****************************************************************************

class DBarWidget : public QWidget {
  Q_OBJECT

public:
  DBarWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
  void showBasedOn(QWidget *parent = 0, const QPoint &offset = QPoint(0,0) );
  void resizeBasedOn(const QSize &);
  virtual bool isEmpty() const; 
  QPoint barOffset() const { return base_offset; }

protected:
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);

  void mouseMoveEvent ( QMouseEvent * event );
  void mousePressEvent ( QMouseEvent * event );
  void mouseReleaseEvent ( QMouseEvent * event );

protected:
  QColor colorBright;
  QColor colorDark;

private:
  QPoint mousePoint;
  bool in_mouse_move;
  QPoint base_offset;
};

//*****************************************************************************
// DMetaDataBar
//*****************************************************************************

class DMetaDataBar : public DBarWidget {
  Q_OBJECT

public:
  DMetaDataBar(QWidget *parent = 0, Qt::WindowFlags f = 0);
  void resizeBasedOn(const QSize &);
  void showBasedOn(QWidget *parent = 0, const QPoint &offset = QPoint(0,0) );
  bool isEmpty() const; 

public slots:
  void setMetadata( const QString &m = "" );

protected:
  void paintEvent(QPaintEvent *);

private:
  QString metadata;

  QTextBrowser *tb;

};


#endif // SCALE_BAR_H
