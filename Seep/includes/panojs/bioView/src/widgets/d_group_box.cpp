/*******************************************************************************

  DGroupBox that shrinks
  
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    2007-07-17 19:15 - First creation
      
  ver: 1
       
*******************************************************************************/

#include <QtGui>

#include "d_group_box.h"


//*****************************************************************************
// DGroupBox that shrinks
//*****************************************************************************

DGroupBox::DGroupBox(QWidget *) {
  min_height = box_min_height;
  hideButton = new DWidgetTitleButton(this);
  QObject::connect(hideButton, SIGNAL(clicked()), this, SLOT(toggleShrink()));
  QObject::connect(this, SIGNAL(toggled(bool)), this, SLOT(onToggled(bool)));
  this->setMinimumHeight(box_min_height);
  setShrunk( false );

  if (!this->isHidden())
    hideButton->show();
  min_height = this->minimumHeight();
}

void DGroupBox::resizeEvent ( QResizeEvent * ) {
  hideButton->move( this->width()-hideButton->width()-10, hideButton->y() );
}

void DGroupBox::showEvent ( QShowEvent * ) {
  resizeEvent(0);
}

void DGroupBox::onToggled( bool on ) {
  if (!on) setShrunk( true );
  if (on && shrunk) setShrunk( false );
}

void DGroupBox::setShrunk( bool v ) {

  // if this window has no other controls
  QObjectList ol = this->children();
  if (ol.size() == 1) {
    QObjectList ol2 = ol[0]->children();
    if (ol.size()<1) return;
  }

  //if (shrunk == v) return;
  shrunk = v;
  if (shrunk) {
    //hideButton->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarUnshadeButton, 0, this));
    hideButton->setIcon(this->style()->standardIcon(QStyle::SP_ArrowDown, 0, this));
    min_height = this->minimumHeight();
    this->setMinimumHeight(box_min_height);
    this->setMaximumHeight(box_min_height);
    emit resizedMinimum();
  } else {
    //hideButton->setIcon(this->style()->standardIcon(QStyle::SP_TitleBarShadeButton, 0, this));
    hideButton->setIcon(this->style()->standardIcon(QStyle::SP_ArrowUp, 0, this));
    this->setMaximumHeight(1000000);
    this->setMinimumHeight(min_height);
    emit resizedMinimum();
  }
  hideButton->resize( hideButton->sizeHint() );
}

//*****************************************************************************
// DWidgetTitleButton shrink button for groupbox title
//*****************************************************************************

DWidgetTitleButton::DWidgetTitleButton(QWidget *widget)
    : QAbstractButton(widget) 
{ 
  setFocusPolicy(Qt::NoFocus); 
}

QSize DWidgetTitleButton::sizeHint() const
{
    ensurePolished();

    int dim = 0;
    if (!icon().isNull()) {
        const QPixmap pm = icon().pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize), QIcon::Normal);
        dim = qMax(pm.width(), pm.height());
    }

    return QSize(dim + 4, dim + 4);
}

void DWidgetTitleButton::enterEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::enterEvent(event);
}

void DWidgetTitleButton::leaveEvent(QEvent *event)
{
    if (isEnabled()) update();
    QAbstractButton::leaveEvent(event);
}

void DWidgetTitleButton::paintEvent(QPaintEvent *)
{
    QPainter p(this);

    QRect r = rect();
    QStyleOption opt;
    opt.init(this);
    opt.state |= QStyle::State_AutoRaise;
    if (isEnabled() && underMouse() && !isChecked() && !isDown())
        opt.state |= QStyle::State_Raised;
    if (isChecked())
        opt.state |= QStyle::State_On;
    if (isDown())
        opt.state |= QStyle::State_Sunken;
    style()->drawPrimitive(QStyle::PE_PanelButtonTool, &opt, &p, this);

    int shiftHorizontal = opt.state & QStyle::State_Sunken ? style()->pixelMetric(QStyle::PM_ButtonShiftHorizontal, &opt, this) : 0;
    int shiftVertical = opt.state & QStyle::State_Sunken ? style()->pixelMetric(QStyle::PM_ButtonShiftVertical, &opt, this) : 0;

    r.adjust(2, 2, -2, -2);
    r.translate(shiftHorizontal, shiftVertical);

    QPixmap pm = icon().pixmap(style()->pixelMetric(QStyle::PM_SmallIconSize), isEnabled() ?
                                underMouse() ? QIcon::Active : QIcon::Normal : QIcon::Disabled, isDown() ? QIcon::On : QIcon::Off);
    
    style()->drawItemPixmap(&p, r, Qt::AlignCenter, pm);
}

