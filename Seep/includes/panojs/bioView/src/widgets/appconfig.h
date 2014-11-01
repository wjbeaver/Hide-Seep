/*******************************************************************************

  This file provides two configuration assistants: application and system.

  DConfig - simple way of storing widget and application custom configuration
  DSysConfig - extendes QSysInfo with additional info like number of CPUs

  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  Licence: GPL

  History:
    02/08/2007 17:14 - First creation
      
  ver: 1       
*******************************************************************************/

#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <QHash>
#include <QString>
#include <QVariant>
#include <QSysInfo>

class QWidget;

//------------------------------------------------------------------------------
// General Global Variables
//------------------------------------------------------------------------------

class DConfig {
public:
  DConfig( const QString &organization, const QString &application, const QString &config_file );
  ~DConfig();

  // Some default static methods
  static void loadWidgetConfig( QWidget *w, const QString &org, const QString &app, bool pre_pos = false );
  static void saveWidgetConfig( QWidget *w, const QString &org, const QString &app );  

  static QString loadPath( const QString &name, const QString &org, const QString &app );
  static void savePath( const QString &name, const QString &path, const QString &org, const QString &app );  

  static QVariant getValue( const QString &org, const QString &app, const QString &tag, const QVariant &def = QVariant() );
  static void     setValue( const QString &org, const QString &app, const QString &tag, const QVariant &var );

  // generic methods
  virtual void loadConfig();
  virtual void saveConfig() const;

  QVariant getValue( const QString &tag, const QVariant &def = QVariant() ) const;
  void     setValue( const QString &tag, const QVariant &var );

  // specific methods
  virtual void loadWidgetConfig( QWidget *w, bool pre_pos = false );
  virtual void saveWidgetConfig( QWidget *w ) const;  

  virtual QString loadPath( const QString &name );
  virtual void savePath( const QString &name, const QString &path );  

private:
  QHash<QString, QVariant> tags;

  QString organ;
  QString app;
  QString confFile;
};


//------------------------------------------------------------------------------
// Additional System Configuration
//------------------------------------------------------------------------------

class DSysConfig: public QSysInfo {
public:
  static int      numberCPUs();
  static QRect    virtualScreenMaxGeometry();
  static QSize    virtualScreenMaxSize();
  static QString  applicationPath();
};

#endif //APP_CONFIG_H
