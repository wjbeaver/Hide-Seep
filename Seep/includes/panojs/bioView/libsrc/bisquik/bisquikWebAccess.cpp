/*******************************************************************************

BQWebAccessDialog - download dialog in a form of a mini web browser 

Author: Dima Fedorov Levit <dimin@dimin.net> <http://www.dimin.net/>
        Center for Bio-image Informatics, UCSB

History:
2008-01-01 17:02 - First creation

ver: 2

*******************************************************************************/

#include <QtCore>
#include <QtGui>
#include <QtXml>
#include <QtNetwork>
#include <QtNetwork/QSslSocket>

#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QAuthenticator>
#include <QtNetwork/QNetworkDiskCache>
#include <QtNetwork/QNetworkProxy>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QSslError>

#define BQ_ORGANIZATION    "UCSB"
#define BQ_APPLICATION     "Bisquik"
#define BQ_CONFIG_FILE     "bisquik.ini"
#define BQ_VERSION         "0.0.2"

#include "bisquikWebAccess.h"

//---------------------------------------------------------------------------
// BQWebAccessDialog
//---------------------------------------------------------------------------

BQWebAccessDialog::BQWebAccessDialog() {

  setObjectName( "BisquikAccess" );
  ui.setupUi(this);
  file_path = QDir::currentPath();

  webView = new BQWebView(ui.groupBox);
  webView->setObjectName(QString::fromUtf8("webView"));
  ui.verticalLayout->addWidget(webView);

  QSettings conf( QSettings::IniFormat, QSettings::UserScope, BQ_ORGANIZATION, BQ_APPLICATION );
  ui.urlEdit->setText( conf.value( "url", "dough.ece.ucsb.edu" ).toString() );
  ui.userEdit->setText( conf.value( "user", "" ).toString() );
  ui.passwordEdit->setText( conf.value( "passwd", "" ).toString() );

  ui.autoLoadCheck->setChecked( conf.value( "AutoLoad", false ).toBool() );

  connect(webView, SIGNAL(loadProgress(int)), this, SLOT(onWebReadProgress(int)));
  connect(webView, SIGNAL(urlChanged ( const QUrl &)), this, SLOT(onUrlChanged ( const QUrl &)));
  connect(ui.backButton, SIGNAL(clicked(bool)), webView, SLOT(back()));

  connect(ui.searchButton, SIGNAL(clicked(bool)), this, SLOT(onSearch()));

  connect(&bqAccess, SIGNAL(dataReadProgress(int,int)), this, SLOT(onHttpReadProgress(int,int)));
  connect(ui.cancelButton, SIGNAL(clicked(bool)), &bqAccess, SLOT(abort()));

  connect(ui.buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked(bool)), this, SLOT(onReject()));
  connect(ui.buttonBox->button(QDialogButtonBox::Open), SIGNAL(clicked(bool)), this, SLOT(onAccept()));
  connect(ui.pathLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(onPathLabelLinkActivated( const QString &)));

#ifndef QT_NO_OPENSSL
    if (!QSslSocket::supportsSsl()) {
      QMessageBox::information(0, "Bisque Browser",
		"This system does not support OpenSSL. SSL websites will not be available.");
    }
#endif
  webView->page()->setNetworkAccessManager( &netman );
}

BQWebAccessDialog::~BQWebAccessDialog() {
  QSettings conf( QSettings::IniFormat, QSettings::UserScope, BQ_ORGANIZATION, BQ_APPLICATION );  
  conf.setValue( "AutoLoad", ui.autoLoadCheck->isChecked() );
}

void BQWebAccessDialog::onReject() {
  bqAccess.abort();
  reject();
}

void BQWebAccessDialog::onAccept() {
  bqAccess.abort();
  onDownload();
  accept();
}

void BQWebAccessDialog::startProcess() {
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
  QApplication::processEvents();
}

void BQWebAccessDialog::inProcess( int done, int total, const QString &text ) {
  if (done!=total && time_progress.elapsed()<400 ) return;
  ui.progressBar->setMaximum( total );
  ui.progressBar->setValue( done );
  time_progress.start();
  if (!text.isEmpty()) ui.progressLabel->setText(text);
}

void BQWebAccessDialog::finishProcess() {
  QApplication::restoreOverrideCursor();
  QApplication::processEvents();
  ui.progressBar->setValue( 0 );
}

void BQWebAccessDialog::showErrors() {
  QString str;
  //ui.statusLabel->setText( str );
  ui.progressLabel->setText( str );
  /*
  int error = bqAccess.getHttp()->error();
  if (error > 0)
    ui.progressLabel->setText( bqAccess.getHttp()->errorString() );
  else {
    QHttpResponseHeader h = bqAccess.getHttp()->lastResponse();
    if ( h.statusCode() != 200 ) {
      str.sprintf("Status code: %d", h.statusCode() ); 
      ui.progressLabel->setText( str );
    }
  }
  */
}

void BQWebAccessDialog::showRequest( const QString &v ) {
  ui.progressLabel->setText( v );
}

void BQWebAccessDialog::onSearch() {
  QUrl url( ui.urlEdit->text() );
  if (ui.urlEdit->text().indexOf("://") == -1)
    url = QUrl( QString("http://")+ui.urlEdit->text() );

  QSettings conf( QSettings::IniFormat, QSettings::UserScope, BQ_ORGANIZATION, BQ_APPLICATION );
  conf.setValue( "url", url.toString() );
  conf.setValue( "user", ui.userEdit->text() );
  conf.setValue( "passwd", ui.passwordEdit->text() );

  bqAccess.setHost ( url  );
  bqAccess.setUserName ( ui.userEdit->text() );
  bqAccess.setPassword ( ui.passwordEdit->text() );

  url.setPath("/bisquik/browser");


  // set request parameters - user name and password
  QNetworkRequest request;
  request.setUrl( url );
  request.setRawHeader( "User-Agent", "BisqueBrowser 1.0" );

  QString authorization = ui.userEdit->text() + ":" + ui.passwordEdit->text();
  authorization = "Basic " + authorization.toLatin1().toBase64();
  request.setRawHeader( "Authorization", authorization.toLatin1() );

  // Load
  qDebug( url.toString().toLatin1().constData() );
  showRequest( url.toString() );
  //webView->load( url );
  webView->load( request );
}

void BQWebAccessDialog::onWebReadProgress ( int done ) {
  inProcess( done, 100, webView->url().toString() );
}

void BQWebAccessDialog::onHttpReadProgress ( int done, int total ) {
  QString s;
  QStringList state_strings;
  state_strings << "Unconnected" << "HostLookup" << "Connecting" << "Sending" << "Reading" << "Connected" << "Closing";
  QHttp *http = bqAccess.getHttp();
  s = state_strings[http->state()] + " " + bqAccess.currentUrl().toString();
  inProcess( done, total, s );
}

void BQWebAccessDialog::onUrlChanged ( const QUrl & url ) {
  QString u = url.toString();
  qDebug( url.toString().toLatin1().constData() );

  if ( ui.autoLoadCheck->isChecked() && u.indexOf("?resource=") != -1) {
    u.remove ( QRegExp("^.*\\?resource=") );
    doDownload( QUrl( u ) );
    accept();
  }
}

void BQWebAccessDialog::onDownload( ) {
  QUrl url = webView->url();
  QString u = url.toString();

  if (u.indexOf("?resource=") != -1) {
    u.remove ( QRegExp("^.*\\?resource=") );
    doDownload( QUrl( u ) );
    accept();
  }    
}

void BQWebAccessDialog::doDownload( const QUrl & url ) {
  
  qDebug( url.toString().toLatin1().constData() );

  startProcess();
  bqAccess.setUserName ( ui.userEdit->text() );
  bqAccess.setPassword ( ui.passwordEdit->text() );
  BQImageItem imageItem = bqAccess.getImage( url );

  QString fileName = bqAccess.getImageFileName( imageItem.src );
  fileName = file_path + "/" + fileName;
  if (bqAccess.getImageFile( imageItem.src, fileName ) == 0)
    image_file_name = fileName;

  fileName = fileName + ".gox";
  if (bqAccess.getImageGObjects( imageItem.uri, fileName ) == 0)
    gob_file_name = fileName;

  showErrors();
  finishProcess();
}

void BQWebAccessDialog::onPathLabelLinkActivated ( const QString & ) {
  
  QString dir = QFileDialog::getExistingDirectory( this, tr("Open Directory"), file_path );
  if (!dir.isEmpty())
    setPath( dir );
}

//---------------------------------------------------------------------------
// BQWebView required for the webview to access SSL secure sites
//---------------------------------------------------------------------------
BQWebView::BQWebView(QWidget *parent) : QWebView(parent) {

}

QWebView* BQWebView::createWindow ( QWebPage::WebWindowType /*type*/ ) {
  return this;
}

/***************************************************************************/
// BQNetworkAccessManager required for the webview to access SSL secure sites
/***************************************************************************/

BQNetworkAccessManager::BQNetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent)//,
    //requestFinishedCount(0), requestFinishedFromCacheCount(0), requestFinishedPipelinedCount(0),
    //requestFinishedSecureCount(0)
{
    //connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
    //        SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));
    //connect(this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
    //        SLOT(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
    //connect(this, SIGNAL(finished(QNetworkReply *)),
    //        SLOT(requestFinished(QNetworkReply *)));
#ifndef QT_NO_OPENSSL
    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
            SLOT(sslErrors(QNetworkReply*, const QList<QSslError>&)));
#endif
    //loadSettings();

    //QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
    //QString location = QDesktopServices::storageLocation(QDesktopServices::CacheLocation);
    //diskCache->setCacheDirectory(location);
    //setCache(diskCache);
}

/*
QNetworkReply* BQNetworkAccessManager::createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData)
{
    QNetworkRequest request = req; // copy so we can modify
    // this is a temporary hack until we properly use the pipelining flags from QtWebkit
    // pipeline everything! :)
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);
    return QNetworkAccessManager::createRequest(op, request, outgoingData);
}

void BQNetworkAccessManager::requestFinished(QNetworkReply *reply)
{
    requestFinishedCount++;

    if (reply->attribute(QNetworkRequest::SourceIsFromCacheAttribute).toBool() == true)
        requestFinishedFromCacheCount++;

    if (reply->attribute(QNetworkRequest::HttpPipeliningWasUsedAttribute).toBool() == true)
        requestFinishedPipelinedCount++;

    if (reply->attribute(QNetworkRequest::ConnectionEncryptedAttribute).toBool() == true)
        requestFinishedSecureCount++;

    if (requestFinishedCount % 10)
        return;

    double pctCached = (double(requestFinishedFromCacheCount) * 100.0/ double(requestFinishedCount));
    double pctPipelined = (double(requestFinishedPipelinedCount) * 100.0/ double(requestFinishedCount));
    double pctSecure = (double(requestFinishedSecureCount) * 100.0/ double(requestFinishedCount));
    qDebug("STATS [%lli requests total] [%3.2f%% from cache] [%3.2f%% pipelined] [%3.2f%% SSL/TLS]", requestFinishedCount, pctCached, pctPipelined, pctSecure);

}

void BQNetworkAccessManager::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("proxy"));
    QNetworkProxy proxy;
    if (settings.value(QLatin1String("enabled"), false).toBool()) {
        if (settings.value(QLatin1String("type"), 0).toInt() == 0)
            proxy = QNetworkProxy::Socks5Proxy;
        else
            proxy = QNetworkProxy::HttpProxy;
        proxy.setHostName(settings.value(QLatin1String("hostName")).toString());
        proxy.setPort(settings.value(QLatin1String("port"), 1080).toInt());
        proxy.setUser(settings.value(QLatin1String("userName")).toString());
        proxy.setPassword(settings.value(QLatin1String("password")).toString());
    }
    setProxy(proxy);
}

void BQNetworkAccessManager::authenticationRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.iconLabel->setText(QString());
    passwordDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Enter username and password for \"%1\" at %2</qt>");
    introMessage = introMessage.arg(Qt::escape(reply->url().toString())).arg(Qt::escape(reply->url().toString()));
    passwordDialog.introLabel->setText(introMessage);
    passwordDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.userNameLineEdit->text());
        auth->setPassword(passwordDialog.passwordLineEdit->text());
    }
}

void BQNetworkAccessManager::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth)
{
    BrowserMainWindow *mainWindow = BrowserApplication::instance()->mainWindow();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::ProxyDialog proxyDialog;
    proxyDialog.setupUi(&dialog);

    proxyDialog.iconLabel->setText(QString());
    proxyDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Connect to proxy \"%1\" using:</qt>");
    introMessage = introMessage.arg(Qt::escape(proxy.hostName()));
    proxyDialog.introLabel->setText(introMessage);
    proxyDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(proxyDialog.userNameLineEdit->text());
        auth->setPassword(proxyDialog.passwordLineEdit->text());
    }
}
*/

#ifndef QT_NO_OPENSSL
void BQNetworkAccessManager::sslErrors(QNetworkReply *reply, const QList<QSslError> &error) {

	// check if SSL certificate has been trusted already
	QString replyHost = reply->url().host() + ":" + reply->url().port();
	if(! sslTrustedHostList.contains(replyHost)) {
		QStringList errorStrings;
		for (int i = 0; i < error.count(); ++i)
			errorStrings += error.at(i).errorString();
		QString errors = errorStrings.join(QLatin1String("\n"));
		int ret = QMessageBox::warning(0, QCoreApplication::applicationName(),
				tr("SSL Errors:\n\n%1\n\n%2\n\n"
						"Do you want to ignore these errors for this host?").arg(reply->url().toString()).arg(errors),
						QMessageBox::Yes | QMessageBox::No,
						QMessageBox::No);
		if (ret == QMessageBox::Yes) {
			reply->ignoreSslErrors();
			sslTrustedHostList.append(replyHost);
		}
	}
}
#endif

