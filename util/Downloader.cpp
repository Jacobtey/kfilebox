#include "Downloader.h"

namespace util {
Downloader::Downloader(QString url, QString file)
// don't delete the following line as it's needed to preserve source code of this autogenerated element
// section 127-0-0-2-50c13b42:1288dba840f:-8000:0000000000000E46 begin
{
    //this->url=new QUrl();
    this->url.setUrl(url);
    this->filename=file;
}
// section 127-0-0-2-50c13b42:1288dba840f:-8000:0000000000000E46 end
// don't delete the previous line as it's needed to preserve source code of this autogenerated element

void Downloader::download()
// don't delete the following line as it's needed to preserve source code of this autogenerated element
// section 127-0-0-2-50c13b42:1288dba840f:-8000:0000000000000E48 begin
{
    QNetworkAccessManager *manager = new QNetworkAccessManager(this);

    QString file_path=filename;
    if (downloadpath.length()>0)
        file_path=downloadpath+filename;
    file.setFileName(file_path);
    file.open(QIODevice::WriteOnly);
    QNetworkRequest *request= new QNetworkRequest(QUrl(url));
    request->setRawHeader("User-Agent", "Kfilebox");
    reply = manager->get(*request);
    connect(reply, SIGNAL(finished()), SLOT(downloadFinished()));
    connect(reply, SIGNAL(readyRead()), SLOT(downloadReadyRead()));
    connect(reply,SIGNAL(downloadProgress(qint64,qint64)),SLOT(downloadProgress(qint64,qint64)));
    connect(reply,SIGNAL(error(QNetworkReply::NetworkError)),SLOT(displayError(QNetworkReply::NetworkError)));

    //qt_message_output(QtWarningMsg,"Descargando archivo desde "+ url.toString().toLatin1());

}
// section 127-0-0-2-50c13b42:1288dba840f:-8000:0000000000000E48 end
// don't delete the previous line as it's needed to preserve source code of this autogenerated element

Downloader::Downloader()
// don't delete the following line as it's needed to preserve source code of this autogenerated element
// section 127-0-0-2--1d986f05:128916c472c:-8000:0000000000000E76 begin
{
}
// section 127-0-0-2--1d986f05:128916c472c:-8000:0000000000000E76 end
// don't delete the previous line as it's needed to preserve source code of this autogenerated element

void Downloader::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit (downloadProgress(100*bytesReceived/bytesTotal));

}

void Downloader::downloadFinished()
{
   file.close();
   QVariant possible_redirect=reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (!possible_redirect.toUrl().toString().isEmpty() && possible_redirect.toUrl()!=url){
       url=possible_redirect.toUrl().toString();
       download();
   }
   else
    {
       reply->close();
       delete reply;
       emit (fileDownloaded());
    }
}

void Downloader::downloadReadyRead()
{
    file.write(reply->readAll());
}

void Downloader::displayError(QNetworkReply::NetworkError err){
    qt_message_output(QtWarningMsg,tr("Error downloading file").toLatin1());
}

} /* End of namespace util */