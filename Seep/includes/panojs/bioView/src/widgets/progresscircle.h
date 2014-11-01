/*******************************************************************************

  DProgressCircle is "stay on top" transparent progress qwidget
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
       
*******************************************************************************/

#ifndef PROGRESS_CIRCLE_H
#define PROGRESS_CIRCLE_H

#include <QWidget>
#include <QVector>
#include <QString>

class DProgressCircle : public QWidget {
  Q_OBJECT

public:
  DProgressCircle(QWidget *parent = 0, Qt::WindowFlags f = 0);
  void showBasedOn(QWidget *parent = 0, const QPoint &offset = QPoint(0,0));

public slots:
  void setProgress( int, const QString &s = "" );
  void setText( const QString &s = "" );

protected:
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);

private:
  QVector<unsigned char> colors;
  QString str;
  int progress_val_prev; // 0-12 progress bar number
};

#endif // PROGRESS_CIRCLE_H
