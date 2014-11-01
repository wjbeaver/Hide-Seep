/*******************************************************************************

BQWebAccessDialog - download dialog in a form of a mini web browser 

Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
        Center for Bio-image Informatics, UCSB

History:
2008-01-01 17:02 - First creation

ver: 2

*******************************************************************************/

#ifndef BISQUIK_WEB_ACCESS_H
#define BISQUIK_WEB_ACCESS_H

#include <ui_bisquikWebAccess.h>

#include <QtCore>
#include <QtGui>
#include <QtXml>
#include <QtNetwork>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QWebView>

#include "bisquikAccess.h"


//---------------------------------------------------------------------------
// NetworkAccessManager required for the webview to access SSL secure sites
//---------------------------------------------------------------------------

class BQNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT

public:
    BQNetworkAccessManager(QObject *parent = 0);

    //virtual QNetworkReply* createRequest ( Operation op, const QNetworkRequest & req, QIODevice * outgoingData = 0 );

private:
    QList<QString> sslTrustedHostList;
    //qint64 requestFinishedCount;
    //qint64 requestFinishedFromCacheCount;
    //qint64 requestFinishedPipelinedCount;
    //qint64 requestFinishedSecureCount;

public slots:
    //void loadSettings();
    //void requestFinished(QNetworkReply *reply);

private slots:
    //void authenticationRequired(QNetworkReply *reply, QAuthenticator *auth);
    //void proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth);
#ifndef QT_NO_OPENSSL
    void sslErrors(QNetworkReply *reply, const QList<QSslError> &error);
#endif
};

//---------------------------------------------------------------------------
// BQWebView required for the webview to access SSL secure sites
//---------------------------------------------------------------------------
class BQWebView : public QWebView
{
    Q_OBJECT

public:
    BQWebView(QWidget *parent = 0);

protected:
    virtual QWebView* createWindow ( QWebPage::WebWindowType type );
};

//---------------------------------------------------------------------------
// BQWebAccessDialog
//---------------------------------------------------------------------------

class BQWebAccessDialog : public QDialog
{
  Q_OBJECT

public:
  BQWebAccessDialog();
  ~BQWebAccessDialog();

public slots:
  void onSearch();  
  void onReject();
  void onAccept();
  void onDownload();

  void setPath( const QString &v ) { file_path = v; ui.pathLabel->setText( "<a href='#path'>Downloading to: </a>"+file_path ); }

  QString downloadPath() const { return file_path; }
  QString imageFileName() const { return image_file_name; }
  QString gobFileName()   const { return gob_file_name; }

  void startProcess();
  void inProcess( int done, int total, const QString &text );
  void finishProcess();

private:
  Ui::BisquikWebAccessDialog ui;
  BQAccess bqAccess;
  BQNetworkAccessManager netman;
  BQWebView *webView;

  QTime time_progress;
  QString file_path;
  QString image_file_name, gob_file_name;

  void showErrors();
  void showRequest( const QString & );

private slots:
  void doDownload( const QUrl & url ); 

  void onWebReadProgress ( int done );
  void onHttpReadProgress ( int done, int total );
  void onUrlChanged ( const QUrl & url );
  void onPathLabelLinkActivated ( const QString & link );
};




#endif // BISQUIK_WEB_ACCESS_H
