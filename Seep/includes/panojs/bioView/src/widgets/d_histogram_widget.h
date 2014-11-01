/*******************************************************************************

  DHistogramWidget is a histogram plotting widget
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    2007-07-26 12:33 - First creation
      
  ver: 1
       
*******************************************************************************/

#ifndef D_HISTOGRAM_WIDGET_H
#define D_HISTOGRAM_WIDGET_H

#include <QAction>
#include <QWidget>
#include <QVector>
#include <QString>
#include <QPen>

#ifndef D_NO_DHISTOGRAM
class DimHistogram;
#endif

class DHistogramWidget : public QWidget {
  Q_OBJECT

public:

  DHistogramWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);

  void plot( const std::vector<unsigned int> &in, const std::vector<unsigned int> &out );
  #ifndef D_NO_DHISTOGRAM
  void plot( const std::vector<DimHistogram> &, const std::vector<DimHistogram> & );
  void plot( const DimHistogram &, const DimHistogram & );
  #endif

protected:
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);
  void mouseMoveEvent (QMouseEvent *);
  void mousePressEvent (QMouseEvent *); 
  void mouseReleaseEvent (QMouseEvent *);
  void contextMenuEvent(QContextMenuEvent *);

private:
  QAction *logScaleAct;
  QAction *bcInterpolationAct;
  void createActions();

private slots:
  void setLogScale( bool v ) { log_scale=v; logScaleAct->setChecked( log_scale ); interpolate_plot_vectors(); repaint(); }
  void triggerLogScale() { setLogScale(!log_scale); }

  void setBcInterpolation( bool v ) { bicubic_interpolation=v; bcInterpolationAct->setChecked( bicubic_interpolation ); interpolate_plot_vectors(); repaint(); }
  void triggerBcInterpolation() { setBcInterpolation(!bicubic_interpolation); }

private:
  QVector< QVector<QPoint> > plots;
  QVector< std::vector<unsigned int> > data;
  QVector<QPen> pens;

  bool log_scale;
  bool bicubic_interpolation;

  int plot_padding_x;
  int plot_padding_y;
  int line_width;
  QPoint mousePoint;

  void interpolate_plot_vectors();
};

#endif // D_HISTOGRAM_WIDGET_H
