/*******************************************************************************
  
  WallView main application, provides command line parsing and instance handling
    
  Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
  Copyright (C) BioImage Informatics <www.bioimage.ucsb.edu>

  History:
    12/08/2006 18:09 - First creation
      
  ver: 1
       
*******************************************************************************/

#include <QApplication>
#include <QSettings>
#include <QDesktopWidget>
#include <QWidget>

#include "viewer/tileviewer.h"

#include "dsingleapplication.h"

int main(int argc, char *argv[]) {

  QApplication app(argc, argv);

  bool start_full_screen  = false;
  bool start_server       = false;
  bool no_controls        = false;
  bool start_full_virtual = false;
  bool one_instance       = false;

  // lists are not supported yet
  QString file_to_load;

  QStringList argl = QApplication::arguments(); 
  for (int i=1; i<argl.size(); ++i) {
    if (argl[i] == "-fullscreen") start_full_screen = true;
    else
    if (argl[i] == "-fs") start_full_screen = true;
    else
    if (argl[i] == "-startserver") start_server = true;
    else
    if (argl[i] == "-srv") start_server = true;
    else
    if (argl[i] == "-nocontrols") no_controls = true;
    else
    if (argl[i] == "-noc") no_controls = true;
    else
    if (argl[i] == "-fullvirtual") start_full_virtual = true;
    else
    if (argl[i] == "-fv") start_full_virtual = true;
    else
    if (argl[i] == "-oneinstance") one_instance = true;
    else
    if (argl[i] == "-one") one_instance = true;
    else {
      // should be treated as a file name to load
      file_to_load = argl[i];
    }
  }

  DSingleApplication instance( "WV", false );
  if (one_instance) {
    instance.initialize();
    if ( instance.isRunning() ) {
      // if other instance is running
      instance.sendMessage( QString("-file ")+ file_to_load );
      app.exit(0);
      return 0;
    }
  } // only one instance is allowed

  TileViewer wv;

  // if no other instance running we should listen for messages
  QObject::connect( &instance, SIGNAL( messageReceived(const QString &) ), 
                    &wv, SLOT( onOtherInstanceMessage(const QString &) ) );

  if (start_full_screen) wv.fullScreen();
  if (start_full_virtual) wv.fullVirtualScreen();
  if (start_server) wv.serverStart();
  wv.show();
  if (!no_controls) wv.showControls();
  if (!file_to_load.isEmpty()) {
    QApplication::processEvents();
    wv.loadImage( file_to_load );
  }

  return app.exec();
}
