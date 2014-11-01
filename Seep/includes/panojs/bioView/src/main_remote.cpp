
#include <QApplication>
#include <QSettings>
#include <QDesktopWidget>
#include <QWidget>

#include "viewer/remoteWidget.h"

int main(int argc, char *argv[]) {

  QApplication app(argc, argv);

  bool connect_on_start = false;

  QStringList argl = QApplication::arguments(); 
  for (int i=1; i<argl.size(); ++i) {
    if (argl[i] == "-connect") connect_on_start = true;
  }

  WVRemoteWidget wv;
  if (connect_on_start) wv.clientConnect();
  wv.show();
  app.connect(&app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()));

  return app.exec();
}

