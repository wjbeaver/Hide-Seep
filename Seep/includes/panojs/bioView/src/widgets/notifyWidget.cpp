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

#include <QtCore>
#include <QtGui>

#include "notifyWidget.h"

//------------------------------------------------------------------------------
// misc
//------------------------------------------------------------------------------

QPicture closePicture(int w, int h) {
  QPicture pm;
  QPainter painter;
  painter.begin(&pm);
  painter.setRenderHint ( QPainter::Antialiasing, true );

  int d=3;
  painter.setPen( QPen(QColor(255, 204, 102), d) );
  painter.drawEllipse(d,d,w-d*2.0,h-d*2.0);  
  painter.translate( w/2.0, (-1.0*h/4.0)+(1.0*d/2.0) );
  painter.rotate( 45.0 );
  painter.drawLine(d, h/2.0, w-d, h/2.0 );
  painter.drawLine(w/2.0, d, w/2.0, h-d );
  painter.end();
  return pm;
}

//------------------------------------------------------------------------------
// DPictureButton
//------------------------------------------------------------------------------

DPictureButton::DPictureButton(QWidget* parent)
: QWidget(parent) { 
  setCursor( Qt::PointingHandCursor );
}

void DPictureButton::setPicture ( const QPicture &pic ) {
  picture = pic;
}

void DPictureButton::paintEvent ( QPaintEvent * event ) {
  QPainter painter;
  painter.begin(this);
  painter.drawPicture( 0, 0, picture );
  painter.end();
}

void DPictureButton::mousePressEvent ( QMouseEvent * event ) {
  emit clicked( false );
}


//------------------------------------------------------------------------------
// DNotificationWidget
//------------------------------------------------------------------------------

DNotificationWidget::DNotificationWidget(const QString &text, QMessageBox::Icon icon, QWidget *parent, Qt::WFlags f)
//: QWidget(parent, f | Qt::Window | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint )
: QWidget(parent, f | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint )
{
  int wh = 80;

  if (parent != 0) {
    QRect pr = parent->geometry();
    this->resize( pr.width(), wh );
    this->move( 0, pr.height()-wh );
  } else {
    QDesktopWidget *desktop = QApplication::desktop();
	  QRect pr = desktop->availableGeometry( desktop->primaryScreen() );
    this->resize( 640, wh );
    this->move( pr.width()-640, pr.top()+pr.height()-wh );
  }

  this->setAttribute( Qt::WA_DeleteOnClose );

  textLabel = new QLabel( this );
  textLabel->setFrameShadow(QFrame::Plain);
  textLabel->setFrameShape(QFrame::Box);
  textLabel->setLineWidth(2);

  textLabel->setAlignment( Qt::AlignCenter );
  textLabel->setTextFormat( Qt::RichText );

  QString color_html("#666666");
  if (icon == QMessageBox::Critical) color_html = "#FF0000";
  if (icon == QMessageBox::Warning)  color_html = "#FF8000";
  QString color_text( tr("<font color='%1'>%2</font>").arg(color_html).arg(text) );
  textLabel->setText( color_text );
  textLabel->setGeometry( 0, 0, this->width(), this->height() );

  QFont cf = textLabel->font();
  cf.setPointSize(14);
  textLabel->setFont(cf);

  QPalette pal( textLabel->palette() );
  pal.setColor( QPalette::Background, QColor(255, 255, 225) );
  pal.setColor( QPalette::Foreground, QColor(255, 204, 102) );
  //pal.setColor( QPalette::WindowText, QColor(0x66, 0x66, 0x66) );
  textLabel->setPalette( pal );

  #if (QT_VERSION >= 0x040100)
  textLabel->setAutoFillBackground( true );
  #endif

  // Icon label
  QPixmap ipm = QMessageBox::standardIcon(icon);
  iconLabel = new QLabel( this );
  int lh = this->height();
  int lw = ipm.width()*2;
  iconLabel->setGeometry( 0, 0, lw, lh );
  iconLabel->setAlignment( Qt::AlignCenter );
  iconLabel->setPixmap( ipm );

  // close button
  int bs = 40;
  closeBtn = new DPictureButton( this );
  closeBtn->setGeometry( this->width()-bs-20, 20, bs, bs );
  closeBtn->setPicture( closePicture(bs,bs) );
  connect(closeBtn, SIGNAL(clicked()), this, SLOT(close()));

}

void DNotificationWidget::show( int previewTime ) {
  QTimer::singleShot( previewTime, this, SLOT(close()) );
  QWidget::show();
}

//------------------------------------------------------------------------------
// Static members
//------------------------------------------------------------------------------

void DNotificationWidget::information(const QString &text, int previewTime, QWidget* parent, Qt::WFlags f) {
  DNotificationWidget *ww = new DNotificationWidget(text, QMessageBox::Information, parent, f);
  ww->show( previewTime );
}

void DNotificationWidget::warning(const QString &text, int previewTime, QWidget* parent, Qt::WFlags f) {
  DNotificationWidget *ww = new DNotificationWidget(text, QMessageBox::Warning, parent, f);
  ww->show( previewTime );
}

void DNotificationWidget::error(const QString &text, int previewTime, QWidget* parent, Qt::WFlags f) {
  DNotificationWidget *ww = new DNotificationWidget(text, QMessageBox::Critical, parent, f);
  ww->show( previewTime );
}
