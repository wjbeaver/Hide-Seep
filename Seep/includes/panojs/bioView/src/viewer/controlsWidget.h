/*******************************************************************************
  
  WVControlsWidget is an additional widget for tools 
    
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
       
*******************************************************************************/

#ifndef CONTROLSWIDGET_H
#define CONTROLSWIDGET_H

#include <QWidget>
#include <QPixmap>

#include <ui_controls.h>

#if (QT_VERSION >= 0x040100)
//#define signals Q_SIGNALS 
//#define slots Q_SLOTS
#endif

class QMouseEvent;
class DGraphicsLayer;
class WVLiveEnhancement;

class WVControlsWidget : public QWidget {
  Q_OBJECT

public:
  WVControlsWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);

  void initChannels( const QStringList &l, int r, int g, int b );
  void initEnhancement( const QStringList &l, int def );

  void setThumbnail( const QPixmap &pm, unsigned int orig_w, unsigned int orig_h );
  void setMetadata( const QString &txt );
  void setMetadata( const QHash<QString, QVariant> &hash );
  void setPages( unsigned int _num_pages );

  void setGraphicsLayer ( DGraphicsLayer *_glayer ) {
    ui.thumbLabel->setGraphicsLayer( _glayer );
  }

  void insertLutModifier( WVLiveEnhancement *le );

public slots:
  void onViewChanged(double scale, const QRectF &r);
  void onViewChanged(double scale, int x, int y, int w, int h) {
    onViewChanged( scale, QRectF(x,y,w,h) ); }
  void onThumbAreaPosChanged(const QPointF &center);
  void onThumbAreaPosSelected(const QPointF &center);
  void onPageChanged(int);
  
  void setEnhancementControlsWidget( QWidget *w );
  void boxResizedMinimum();

public slots:
  void onChannelsChanged(int);
  void onChannelsVisChanged(int);
  void onEnhancementChanged(int);
  void onZoomOutReleased();
  void onZoomInReleased();
  void onZoom11Released();
  void onSliderValueChanged(int);
  void onNextPageReleased();
  void onPrevPageReleased();
  void onInitPageReleased();
  void onExtChannels();

signals:
  void channelsChanged(const int &r, const int &g, const int &b, const int &y, const int &m, const int &c);
  void enhancementChanged(int e);
  void zoomChanged(int z);
  void visCenterChanged( const QPointF &center );
  void visCenterSelected( const QPointF &center );
  void pageChanged( int page );

protected:
  void mouseMoveEvent ( QMouseEvent * event );
  void mousePressEvent ( QMouseEvent * event );
  void mouseReleaseEvent ( QMouseEvent * event );
  bool eventFilter(QObject *obj, QEvent *event);
  void resizeEvent ( QResizeEvent * event );
  void viewportResizeEvent ( QResizeEvent * event );
  void showEvent ( QShowEvent * event );

private:
  Ui::ControlsForm ui;
  QPoint mousePoint;
  bool in_mouse_move;
  QScrollArea *scrollArea;
  int display_channels_visibility[6];

  unsigned int cur_page;
  unsigned int num_pages;
};

#endif // CONTROLSWIDGET_H
