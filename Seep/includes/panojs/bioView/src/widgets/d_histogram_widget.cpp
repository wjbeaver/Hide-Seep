/*******************************************************************************

  DRangeSlider is a dual handle sliding sub-range selector, as output gives a 
  sub-range defined by two values two values of min and max, such as min<max, 
  selectd within a given range
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    2007-07-26 12:33 - First creation
      
  ver: 1
       
*******************************************************************************/

#include <QtGui>

#include "xtypes.h"
#include "resize.h"

#include "d_histogram_widget.h"

#ifndef D_NO_DHISTOGRAM
#include "dim_histogram.h"
#endif

DHistogramWidget::DHistogramWidget(QWidget *parent, Qt::WindowFlags f)
: QWidget( parent, f )
{
  plot_padding_x = 10;
  plot_padding_y = 1;
  line_width = 1;
  log_scale = true;
  bicubic_interpolation = true;
  createActions();
}

void DHistogramWidget::createActions() {
  logScaleAct = new QAction(tr("Log scale"), this);
  logScaleAct->setCheckable(true);
  logScaleAct->setChecked( log_scale );
  connect(logScaleAct, SIGNAL(triggered()), this, SLOT(triggerLogScale()));

  bcInterpolationAct = new QAction(tr("Cubic Interpolation"), this);
  bcInterpolationAct->setCheckable(true);
  bcInterpolationAct->setChecked( bicubic_interpolation );
  connect(bcInterpolationAct, SIGNAL(triggered()), this, SLOT(triggerBcInterpolation()));
}

void DHistogramWidget::paintEvent(QPaintEvent *) {

  QPainter painter(this); 
  painter.setRenderHint(QPainter::Antialiasing, true);
  int line_thikness=1;

  QColor c = this->palette().mid().color();
  painter.setPen( QPen(c, 2, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin) );
  painter.drawLine( plot_padding_x, height(), width()-plot_padding_x, height() );

  // draw histograms
  for (unsigned int i=0; i<plots.size(); ++i) {
    painter.setPen( pens[i] );

    QBrush br = painter.brush();
    QPainterPath path;
    path.moveTo( plots[i][0] );
    for (unsigned int x=0; x<plots[i].size(); ++x)
      path.lineTo( plots[i][x] );

    // if there are only two vectors, draw the background for the first one
    if (i==0 && plots.size()==2) {
      path.lineTo( plots[i][plots[i].size()-1].x(), height()-plot_padding_y );
      path.lineTo( plots[i][0].x(), height()-plot_padding_y );
      path.lineTo( plots[i][0] );

      //QLinearGradient myGradient;
      QBrush myGradient( this->palette().mid().color(), Qt::SolidPattern ); 
      painter.setBrush(myGradient);
    }

    painter.drawPath(path);
    painter.setBrush(br);
  } // for i
}

void DHistogramWidget::resizeEvent(QResizeEvent *) {
  interpolate_plot_vectors();
  repaint();
}

void DHistogramWidget::mouseMoveEvent (QMouseEvent *e) {
/*
  if (moving == homNone) return;

  QPoint pd(e->pos() - mousePoint);

  if (moving == homMin)
    x_min = dim::trim( x_min+pd.x(), x_lim_min, x_max-1);

  if (moving == homMax)
    x_max = dim::trim( x_max+pd.x(), x_min+1, x_lim_max);

  updateRangeValues();
  repaint();
  mousePoint = e->pos();
 */
}

void DHistogramWidget::mousePressEvent (QMouseEvent *e) {
  /*
  mousePoint = e->pos();
  if (e->pos().y()-y_pos > handle_size/2) return;
  int d1 = abs( e->pos().x()-x_min );
  int d2 = abs( e->pos().x()-x_max );
  if (d1 <= d2)
    moving = homMin;
  else
    moving = homMax;
    */
}

void DHistogramWidget::mouseReleaseEvent (QMouseEvent *e) {
  //moving = homNone;
}

void DHistogramWidget::contextMenuEvent(QContextMenuEvent *event) {
  QMenu menu(this);
  menu.addAction(logScaleAct);
  menu.addAction(bcInterpolationAct);
  menu.exec(event->globalPos());
}

void DHistogramWidget::interpolate_plot_vectors() {
  
  std::vector<unsigned int> res;
  plots.resize( data.size() );

  // Normalize by the maximum value for individual plots
  double range = this->height() - 2*plot_padding_y;
  for (unsigned int i=0; i<plots.size(); ++i) {
    plots[i].resize( this->width()-2*plot_padding_x );

    if (bicubic_interpolation)
      res = vector_resample_BC( data[i], plots[0].size() );
    else
      res = vector_resample_NN( data[i], plots[0].size() );
    double maxval = dim::max(res);

    if (log_scale) {
      for (unsigned int x=0; x<plots[i].size(); ++x)
        res[x] = log( ((double)res[x]/maxval)*10.0 + 1.0 ) * 100.0;
      maxval = dim::max(res);
    }

    for (unsigned int x=0; x<plots[i].size(); ++x) {
      QPoint p( x+plot_padding_x, this->height() - (((double) res[x] / (double) maxval) * range + plot_padding_y) );
      plots[i][x] = p;
    }
  }

  /*
  // Normalize by the maximum value for all plots
  double maxval = 0;
  for (unsigned int i=0; i<plots.size(); ++i) {
    plots[i].resize( this->width()-2*plot_padding_x );

    res = vector_resample_BC( data[i], plots[0].size() );
    maxval = dim::max( dim::max(res), maxval );
    
    for (unsigned int x=0; x<plots[i].size(); ++x) {
      QPoint p( x+plot_padding_x, res[x] );
      plots[i][x] = p;
    }
  }

  if (log_scale) {
    for (unsigned int i=0; i<plots.size(); ++i)
      for (unsigned int x=0; x<plots[i].size(); ++x) {
        QPoint p = plots[i][x];
        p.setY( log( ((double)p.y()/maxval)*10.0 + 1.0 ) * 100.0 );
        plots[i][x] = p;
      }
  }

  double range = this->height() - 2*plot_padding_y;
  for (unsigned int i=0; i<plots.size(); ++i) {
    for (unsigned int x=0; x<plots[i].size(); ++x) {
      QPoint p = plots[i][x];
      p.setY( this->height() - (((double) p.y() / (double) maxval) * range + plot_padding_y) );
      plots[i][x] = p;
    }
  }
  */
} 

void DHistogramWidget::plot( const std::vector<unsigned int> &in, const std::vector<unsigned int> &out ) {
  
  data.resize( 2 );
  pens.resize(2);

  // init data and colors
  data[0].resize( in.size() );
  data[0].assign( in.begin(), in.end() );
  pens[0] = QPen(this->palette().text().color(), line_width+1, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);

  data[1].resize( out.size() );
  data[1].assign( out.begin(), out.end() );
  pens[1] = QPen(this->palette().highlight().color(), line_width, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);

  // generate data
  interpolate_plot_vectors();
  repaint();
}

#ifndef D_NO_DHISTOGRAM
void DHistogramWidget::plot( const DimHistogram &in,  const DimHistogram &out ) {
  this->plot( in.get_hist(), out.get_hist() );
}

void DHistogramWidget::plot( const std::vector<DimHistogram> &in, const std::vector<DimHistogram> &out ) {
  
  data.resize( in.size()+out.size() );
  pens.resize( data.size() );

  // init data
  for (unsigned int i=0; i<in.size(); ++i) {
    data[i].resize( in[i].size() );
    data[i].assign( in[i].get_hist().begin(), in[i].get_hist().end() );
    pens[i] = QPen(this->palette().text().color(), line_width+1, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
  }

  if (in.size() >= 3) {
    pens[0] = QPen(QColor(255,0,0), line_width+1, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
    pens[1] = QPen(QColor(0,255,0), line_width+1, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
    pens[2] = QPen(QColor(0,0,255), line_width+1, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
  }

  unsigned int p=0;
  for (unsigned int i=in.size(); i<in.size()+out.size(); ++i) {
    data[i].resize( out[p].size() );
    data[i].assign( out[p].get_hist().begin(), out[p].get_hist().end() );
    pens[i] = QPen(this->palette().highlight().color(), line_width, Qt::SolidLine, Qt::SquareCap, Qt::RoundJoin);
    ++p;
  }

  // generate data
  interpolate_plot_vectors();
  repaint();
}
#endif

