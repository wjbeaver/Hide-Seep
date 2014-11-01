/*******************************************************************************

  This file provides two configuration assistants: application and system.

  DConfig - simple way of storing widget and application custom configuration
  DSysConfig - extendes QSysInfo with additional info like number of CPUs...

  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  Licence: GPL

  History:
    02/08/2007 17:14 - First creation
      
  ver: 1       
*******************************************************************************/

#include "appconfig.h"

#include <QtCore>
#include <QtGui>

#ifndef Q_WS_WIN
  #ifdef Q_WS_MACX
  #include <sys/param.h>
  #include <sys/sysctl.h>
  #else  
  #include <unistd.h>
  #endif
#else
#include <windows.h>
#endif

//------------------------------------------------------------------------------
// General Global Variables
//------------------------------------------------------------------------------

DConfig::DConfig( const QString &organization, const QString &application, const QString &config_file ) {

  organ    = organization;
  app      = application;
  confFile = config_file;
}

DConfig::~DConfig() {

}

void DConfig::loadWidgetConfig( QWidget *w, const QString &org, const QString &app, bool pre_pos  ) {

  if (w == NULL) return;
  QPoint ps(0,0);
  QSize  sz(640, 480);

	QDesktopWidget *desktop = QApplication::desktop();
	QRect r = desktop->availableGeometry();

	// select x and y to centralize the window
  if (!pre_pos)
    ps = QPoint( (r.right()-r.left()-sz.width()) / 2, (r.bottom()-r.top()-sz.height()) / 2 );

	QSettings conf( QSettings::IniFormat, QSettings::UserScope, org, app );
	ps = conf.value( QObject::tr("%1/pos").arg( w->objectName() ), ps).toPoint();
	sz = conf.value( QObject::tr("%1/size").arg( w->objectName() ), sz).toSize();

  /*
  if (ps.y() < r.top())    ps.setY( r.top() );
  if (ps.y() > r.bottom()) ps.setY( r.top() );
  if (ps.x() > r.right())  ps.setX( r.left() );
  if (ps.x() < r.left())   ps.setX( r.left() );
  */

	w->move( ps );
	w->resize( sz );
}

void DConfig::saveWidgetConfig( QWidget *w, const QString &org, const QString &app ) {
  if (w == NULL) return;
  QSettings conf( QSettings::IniFormat, QSettings::UserScope, org, app );

  conf.setValue( QObject::tr("%1/pos").arg( w->objectName() ), w->pos() );
  conf.setValue( QObject::tr("%1/size").arg( w->objectName() ), w->size() );
}

QString DConfig::loadPath( const QString &name, const QString &org, const QString &app ) {
	
  QSettings conf( QSettings::IniFormat, QSettings::UserScope, org, app );
	QString path = conf.value( name, QDir::currentPath() ).toString();
	return path;
}

void DConfig::savePath( const QString &name, const QString &path, const QString &org, const QString &app ) {
  QSettings conf( QSettings::IniFormat, QSettings::UserScope, org, app );
  conf.setValue( name, path );
}

QVariant DConfig::getValue( const QString &org, const QString &app, const QString &tag, const QVariant &def ) {
  QSettings conf( QSettings::IniFormat, QSettings::UserScope, org, app );
  return conf.value( tag, def );
}

void DConfig::setValue( const QString &org, const QString &app, const QString &tag, const QVariant &var ) {
  QSettings conf( QSettings::IniFormat, QSettings::UserScope, org, app );
  conf.setValue( tag, var );
}

QVariant DConfig::getValue( const QString &tag, const QVariant &def ) const {
  return tags.value( tag, def );
}

void DConfig::setValue( const QString &tag, const QVariant &var ) {
  tags.insert( tag, var );
}

void DConfig::loadConfig() {

  {  // first try tp load default app supplied data
    QString fileName = DSysConfig::applicationPath() + "/" + confFile;          
    QSettings preset( fileName, QSettings::IniFormat );

    QStringList keys = preset.allKeys();
    for (int i=0; i<keys.size(); ++i)
      setValue( keys[i], preset.value( keys[i], tags.value(keys[i]) ) );
  }

  {  // load user settings
    QSettings conf( QSettings::IniFormat, QSettings::UserScope, organ, app );

    QStringList keys = conf.allKeys();
    for (int i=0; i<keys.size(); ++i)
      setValue( keys[i], conf.value( keys[i], tags.value(keys[i]) ) );
  }

}

void DConfig::saveConfig() const {

  QSettings conf( QSettings::IniFormat, QSettings::UserScope, organ, app );
  QHash<QString, QVariant>::const_iterator it = tags.begin();
  while ( it != tags.end() ) {
    conf.setValue( it.key(), it.value() );
    ++it;
  }
}

void DConfig::loadWidgetConfig( QWidget *w, bool pre_pos ) {
  loadWidgetConfig( w, organ, app, pre_pos );
}

void DConfig::saveWidgetConfig( QWidget *w ) const {
  saveWidgetConfig( w, organ, app );
}

QString DConfig::loadPath( const QString &name ) {
  return getValue( name, QDir::currentPath() ).toString();
}

void DConfig::savePath( const QString &name, const QString &path ) {
  setValue( name, path );
}

//------------------------------------------------------------------------------
// Additional System Configuration
//------------------------------------------------------------------------------

int DSysConfig::numberCPUs() {
  int num_cpus=1;

#ifdef Q_WS_WIN
  SYSTEM_INFO win_sys_info;
  GetSystemInfo( &win_sys_info );
  num_cpus = win_sys_info.dwNumberOfProcessors;
#else
  #ifdef Q_WS_MACX
  static int hw_ncpu[2] = { CTL_HW, HW_NCPU };
  size_t sz = sizeof(int);
  sysctl (hw_ncpu, 2, &num_cpus, &sz, NULL, 0);
  #else  
  num_cpus = sysconf(_SC_NPROCESSORS_ONLN);
  #endif
#endif

  return num_cpus;
}

QRect DSysConfig::virtualScreenMaxGeometry() {
  QDesktopWidget *desktop = QApplication::desktop();
  QRect max_rect(0,0,0,0);

  if (!desktop->isVirtualDesktop()) {
    for (int i=0; i<desktop->numScreens(); ++i ) {
      QRect r = desktop->screenGeometry( i );
      max_rect = max_rect.united( r ); 
    }
  } else
    max_rect = desktop->screen()->geometry();

  return max_rect;
}

QSize DSysConfig::virtualScreenMaxSize() {
  QRect max_rect = virtualScreenMaxGeometry();
  return max_rect.size();
}

QString DSysConfig::applicationPath() {

  // On macx applicationDirPath points to physical path to the binary
  // which may be inside of an application bundle (if the application is bundled)
  // the path will look like this: xdma.app/Contents/MacOS/binary_here
  QString appPath = QApplication::applicationDirPath();

#ifdef Q_WS_MACX
  appPath = appPath + "/../../..";
  appPath = QDir::cleanPath( appPath );
#endif	

  return appPath;
}


