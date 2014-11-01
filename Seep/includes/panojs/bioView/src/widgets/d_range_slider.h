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

#ifndef D_RANGE_SLIDER_H
#define D_RANGE_SLIDER_H

#include <QWidget>
#include <QVector>
#include <QString>

class DRangeSlider : public QWidget {
  Q_OBJECT

public:
  enum HandleOnMove { 
    homNone=0, 
    homMin=1,
    homMax=2
  };

  DRangeSlider(QWidget *parent = 0, Qt::WindowFlags f = 0);

  QSizeF range() const { return QSizeF(range_min, range_max); }
  QSizeF sub() const { return QSizeF(sub_min, sub_max); }
  double subMin() const { return sub_min; }
  double subMax() const { return sub_max; }
  
public slots:
  void setRange( double _min, double _max ) { range_min =_min; range_max=_max; setSub(_min,_max); }
  void setSub( double _min, double _max ) { sub_min =_min; sub_max=_max; updateRangeCoordinates(); this->update(); }

signals:
  void valuesChanged();
  void rangeChanged( double _min, double _max );
  void sliderChanged( const DRangeSlider *slider );

protected:
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);
  void mouseMoveEvent (QMouseEvent *);
  void mousePressEvent (QMouseEvent *); 
  void mouseReleaseEvent (QMouseEvent *);

private:
  double range_min;
  double range_max;
  double sub_min;
  double sub_max;

  bool int_strings;

  int handle_size;
  int y_pos;
  int x_min;
  int x_max;
  int x_lim_min;
  int x_lim_max;

  QPoint mousePoint;
  HandleOnMove moving;
  
  void drawHandle(QPainter *p, int x, int y, int size);
  void updateRangeValues();
  void updateRangeCoordinates();
};

#endif // D_RANGE_SLIDER_H
