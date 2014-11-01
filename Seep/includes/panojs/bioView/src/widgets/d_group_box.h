/*******************************************************************************

  DGroupBox that shrinks
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    2007-07-17 19:15 - First creation
      
  ver: 1
       
*******************************************************************************/

#ifndef DGROUPBOX_H
#define DGROUPBOX_H

#include <QtGui>

class DWidgetTitleButton;

//*****************************************************************************
// DGroupBox that shrinks
//*****************************************************************************

static const int box_min_height = 20;

class DGroupBox : public QGroupBox {
  Q_OBJECT

public:
  DGroupBox(QWidget *parent = 0);
  inline int shrunkHeight( ) { return box_min_height; }
  inline QSize shrunkSize( ) { return QSize(0, box_min_height); }

signals:
  void resizedMinimum();

public slots:
  void setShrunk( bool );
  void toggleShrink() { setShrunk(!shrunk); }

protected:
  void resizeEvent ( QResizeEvent * event );
  void showEvent ( QShowEvent * event );

private slots:
  void onToggled( bool ); 

private:
  DWidgetTitleButton *hideButton;
  bool shrunk;
  int min_height;
};

//*****************************************************************************
// DWidgetTitleButton shrink button for groupbox title
//*****************************************************************************

class DWidgetTitleButton : public QAbstractButton
{
    Q_OBJECT

public:
    DWidgetTitleButton(QWidget *widget);

    QSize sizeHint() const;
    inline QSize minimumSizeHint() const
    { return sizeHint(); }

    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
};

#endif
