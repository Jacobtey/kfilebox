#include "dropboxclient.h"

DropboxClient::DropboxClient(QObject *parent) :
    QObject(parent),
    m_sharedFolders(new QMap<QString,QString>()),
    m_ps(new QProcess(this)),
    m_timer(new QTimer(this)),
    dc(new SynchronousDropboxConnection(this))
{
    prev_status = DropboxUnkown;
    m_message = "";
    m_authUrl = "";

    m_socketPath = QDir::toNativeSeparators(QDir::homePath().append("/.dropbox/command_socket"));
    connect(m_ps, SIGNAL(readyReadStandardOutput()), this, SLOT(readDaemonOutput()));
    connect(m_timer, SIGNAL(timeout()), this, SLOT(getDropboxStatus()));
    m_timer->start(500);
}

DropboxClient::~DropboxClient()
{
    if(m_ps->isOpen())
        m_ps->close();
    delete m_sharedFolders;
}

void DropboxClient::start()
{
    if(!isRunning()) {
        m_ps->start(QDir::toNativeSeparators(QDir::homePath().append("/.dropbox-dist/dropboxd")));
    }
}

void DropboxClient::stop()
{
    sendCommand("tray_action_hard_exit");
    m_ps->waitForFinished();
}

/** Loop. I don't know what is worth(every m_timer->interval() ):
  * - get dropbox pid from file, try to read file in /proc/PID/some_file
  * - if m_socket isn't open - try to connect, got error
  */
void DropboxClient::getDropboxStatus()
{
    sendCommand("get_dropbox_status");
}

void DropboxClient::sendCommand(const QString &command)
{
    QString message = dc->sendCommand(command);
    if(command=="tray_action_hard_exit") {
        message = "Dropbox daemon isn't running";
    }

    if(message.isEmpty()) return;
    DropboxStatus m_status = DropboxUnkown;

    //! @todo coment first if{} block(or modify) if you want disable tray icon blinking on startup in green and blue color(default icons scheme)
    if (message.contains("Initializing")||message.contains("Starting")) {
        m_status=DropboxBussy;
    }
    else if (message == "Idle" || message.contains("Connecting")) {
        m_status=DropboxIdle;
    }
    else if (message.contains("Up")) {
        m_status=DropboxUploading;
    }
    else if (message.contains("Downloading")) {
        m_status=DropboxDownloading;
    }
    else if (message.contains("Saving")) {
        m_status=DropboxSaving;
    }
    else if (message.contains("Indexing")) {
        m_status=DropboxIndexing;
    }
    else if(message.contains("isn't")) {
        m_status=DropboxStopped;
    }
    else if(message.contains("couldn't")||message.contains("Syncing paused")||message.contains("Waiting to be linked")){
        m_status=DropboxDisconnected;
    }
    else if(message.contains("dopped")){
        m_status=DropboxError;
    }

    if((prev_status != m_status) || (m_message != message)) {
        prev_status = m_status;
        m_message = message;
        emit updateStatus(m_status, message);
    }

    if((m_status == DropboxIdle )) { //&& (m_sharedFolders->isEmpty())
        updateSharedFolders("/home/nib/Dropbox/"); //! hard coded yeat
        //        qDebug() << m_sharedFolders->count();
    }
}

void DropboxClient::readDaemonOutput()
{
    QString swap = m_ps->readAllStandardOutput();
    if (swap.contains("https://www.dropbox.com/cli_link?host_id=")) {
        QString prevAuthUrl = m_authUrl;
        m_authUrl = swap.remove("Please visit ").remove(" to link this machine.");
        if(prevAuthUrl.isEmpty() || prevAuthUrl!=m_authUrl) Notification().send(tr("Please visit <a href=\"%1\">url</a> to link this machine.").arg(m_authUrl));
    }
}

bool DropboxClient::isRunning()
{
    QFile file(QDir::toNativeSeparators(QDir::homePath().append("/.dropbox/dropbox.pid")));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    int pid = 0;
    QTextStream in(&file);
    in >> pid;
    file.close();
    return QFile::exists(QString("/proc/%1/cmdline").arg(QString::number(pid)));
}

bool DropboxClient::isInstalled()
{
    return QFile(QDir::toNativeSeparators(QDir::homePath().append("/.dropbox-dist/dropbox"))).exists();
}

void DropboxClient::hideGtkUi(bool v)
{
    if(v) {
        if(QFile(QDir::homePath().append("/.dropbox-dist/wx._controls_.so")).exists())
            QDir().rename(QDir::homePath().append("/.dropbox-dist/wx._controls_.so"), QDir::homePath().append("/.dropbox-dist/wx._controls_orig.so"));
    } else {
        if(QFile(QDir::homePath().append("/.dropbox-dist/wx._controls_orig.so")).exists())
            QDir().rename(QDir::homePath().append("/.dropbox-dist/wx._controls_orig.so"), QDir::homePath().append("/.dropbox-dist/wx._controls_.so"));
    }

}

QString DropboxClient::getVersion()
{
    QFile file(QDir::toNativeSeparators(QDir::homePath().append("/.dropbox-dist/VERSION")));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();

    QString contents = "";
    QTextStream in(&file);
    in >> contents;

    return contents;
}

/**
  * I assume that inside shared folder you can't share subfolder
  */
void DropboxClient::updateSharedFolders(const QString& to)
{
    //! (shared, dropbox, public, photos, "")
    QString reply;

    foreach (QString filename, QDir(to).entryList(QDir::Dirs|QDir::NoDotAndDotDot)) {
        QString tmpPath = to+QDir::separator()+filename+QDir::separator();
        reply = dc->sendCommand(QString("get_folder_tag\npath\t%1").arg(tmpPath));
        reply = reply.remove("tag\t");

        if(reply.isEmpty())
            updateSharedFolders(to+QDir::separator()+filename);
        else
            m_sharedFolders->insert(tmpPath, reply);
    }
}

QStringList DropboxClient::getSharedFolders()
{
    QStringList retVal;
    QMap<QString,QString>::const_iterator i = m_sharedFolders->constBegin();
    while (i != m_sharedFolders->constEnd()) {
        if(i.value() != "dropbox")
            retVal.push_back(i.key());
        ++i;
    }
    return retVal;
}
