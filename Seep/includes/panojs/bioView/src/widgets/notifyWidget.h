/*******************************************************************************
  NotificationWidget (Non-Intrusive dialog box) 
  it is used to notify the user about errors, warnings or
  other things without capturing the focus and breaking the workflow

  Author: 
    Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    10/05/2005 22:01 - dima - updates
    12/11/2006 18:58 - dima - updates

  Ver : 4
  
*******************************************************************************/

#ifndef NOTIFICATION_WIDGET_H
#define NOTIFICATION_WIDGET_H

#include <QWidget>
#include <QMessageBox>

#include <QtGui>

class QLabel;
class QTimer;
class QPicture;
class QWidget;

//------------------------------------------------------------------------------
// DPictureButton
//------------------------------------------------------------------------------

class DPictureButton : public QWidget {
  Q_OBJECT
  
public:
  DPictureButton(QWidget* parent=0);
  void setPicture ( const QPicture &pic );
signals:
  void clicked ( bool checked = false );

protected:
  QPicture picture;

  void paintEvent ( QPaintEvent * event );
  void mousePressEvent ( QMouseEvent * event );
};

//------------------------------------------------------------------------------
// DNotificationWidget
//------------------------------------------------------------------------------

class DNotificationWidget : public QWidget {
  
  Q_OBJECT
  
public:
  DNotificationWidget(const QString &text, QMessageBox::Icon icon, QWidget* parent=0, Qt::WFlags f=0);
  void show( int previewTime );

  static void information(const QString &text, int previewTime, QWidget* parent=0, Qt::WFlags f=0);
  static void warning(const QString &text, int previewTime, QWidget* parent=0, Qt::WFlags f=0);
  static void error(const QString &text, int previewTime, QWidget* parent=0, Qt::WFlags f=0);

private:
  QLabel *textLabel;
  QLabel *iconLabel;
  DPictureButton *closeBtn;

};

#endif //NOTIFICATION_WIDGET_H


