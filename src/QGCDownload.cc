#include "QGCDownload.h"
#include "QGCApplication.h"

// QGCDownload definition

std::unique_ptr<QGCDownloadWatcher>
    QGCDownload::download(const QString &remotePath,
                                 std::shared_ptr<QIODevice> iodevice)
{
    Q_ASSERT(iodevice);

    if (!iodevice->isWritable()) {
        qWarning() << "download: iodevice not writable";
        return {};

    } else return _download(remotePath,
        [iodevice] (QNetworkReply* reply, QGCDownloadWatcher* watcher) {
            Q_ASSERT(reply);
            Q_ASSERT(watcher);

            auto data = reply->readAll();
            if (iodevice->write(data) != data.size()) {
                watcher->_onSavingError();
            }
        }
    );
}


std::unique_ptr<QGCDownloadWatcher>
    QGCDownload::download(const QString &remotePath,
                                 std::shared_ptr<QByteArray> byteArray)
{
    Q_ASSERT(byteArray);

    return _download(remotePath,
        [byteArray] (QNetworkReply* reply, QGCDownloadWatcher* watcher) {
            Q_UNUSED(watcher);
            Q_ASSERT(reply);
            Q_ASSERT(watcher);

            byteArray->push_back(reply->readAll());
        }
    );
}


std::unique_ptr<QGCDownloadWatcher>
    QGCDownload::_download(
        QString const& remotePath,
        std::function<void(QNetworkReply*, QGCDownloadWatcher*)> onReadyRead)
{
    if (remotePath.isEmpty()) {
        qWarning() << "downloadFile empty";
        return {};
    }

    // Split out filename from path
    QString remoteFileName = QFileInfo(remotePath).fileName();
    if (remoteFileName.isEmpty()) {
        qWarning() << "Unabled to parse filename from downloadFile" << remotePath;
        return {};
    }

    // Strip out parameters from remote filename
    int parameterIndex = remoteFileName.indexOf("?");
    if (parameterIndex != -1) {
        remoteFileName  = remoteFileName.left(parameterIndex);
    }

    QUrl remoteUrl;
    if (remotePath.startsWith("http:") || remotePath.startsWith("https:")) {
        remoteUrl.setUrl(remotePath);
    } else {
        remoteUrl = QUrl::fromLocalFile(remotePath);
    }
    if (!remoteUrl.isValid()) {
        qWarning() << "Remote URL is invalid" << remotePath;
        return {};
    }

    QNetworkRequest networkRequest(remoteUrl);
    QNetworkProxy tProxy;
    tProxy.setType(QNetworkProxy::DefaultProxy);
    auto& netwkManager = qgcApp()->networkManager();
    netwkManager.setProxy(tProxy);

    auto netwkReplyRawPtr = netwkManager.get(networkRequest);
    auto netwkReply = std::unique_ptr<QNetworkReply>(netwkReplyRawPtr);

    if (!netwkReply) {
        qWarning() << "QNetworkAccessManager::get request failed";
        return {};
    }

    auto watcherRawPtr = new QGCDownloadWatcher(std::move(netwkReply));
    auto watcher = std::unique_ptr<QGCDownloadWatcher>(watcherRawPtr);

    QObject::connect(&netwkManager, &QNetworkAccessManager::networkAccessibleChanged,
                     watcherRawPtr,  &QGCDownloadWatcher::_onNetworkAccessibilityChanged);

    QObject::connect(netwkReplyRawPtr, &QNetworkReply::readyRead,
        [netwkReplyRawPtr, onReadyRead, watcherRawPtr] (void) {
            onReadyRead(netwkReplyRawPtr, watcherRawPtr);
        }
    );

    return watcher;
}


// QGCDownloadWatcher definition

QGCDownloadWatcher::QGCDownloadWatcher(QObject* parent)
    : QObject(parent),
      _isValid(false)
{ }


QGCDownloadWatcher::QGCDownloadWatcher(std::unique_ptr<QNetworkReply>&& reply,
                                       QObject* parent)
    : QObject(parent),
      _isValid(true),
      _netwkReply(std::move(reply))
{ _initConnections(); }


void QGCDownloadWatcher::cancel(void)
{
    Q_ASSERT(_netwkReply);

    if (_isValid) {
        _netwkReply->close();
        _isValid = false;
    } else {
        qWarning() << "Download Watcher not valid";
    }
}


void QGCDownloadWatcher::
    _onNetworkAccessibilityChanged(QNetworkAccessManager::NetworkAccessibility access)
{
    Q_ASSERT(_netwkReply);

    if (access == QNetworkAccessManager::NotAccessible && _isValid) {
        qWarning() << "Network not accessible";
        _netwkReply->disconnect();
        cancel();
        emit networkError(QNetworkReply::NetworkSessionFailedError);
    }
}


void QGCDownloadWatcher::_onSavingError(void)
{
    Q_ASSERT(_netwkReply);

    _netwkReply->disconnect();
    cancel();
    emit savingError();
}


void QGCDownloadWatcher::_onNetworkError(QNetworkReply::NetworkError error) {
    Q_ASSERT(_netwkReply);

    _netwkReply->disconnect();
    if (error == QNetworkReply::OperationCanceledError) {
        emit cancelled();
    } else {
        emit networkError(error);
    }
}


void QGCDownloadWatcher::_onFinished(void)
{
    Q_ASSERT(_netwkReply);

    _netwkReply->disconnect();
    if (_netwkReply->error() == QNetworkReply::NoError) {
        emit success();
    }
}


void QGCDownloadWatcher::_initConnections(void)
{
    Q_ASSERT(_netwkReply);

    connect(_netwkReply.get(), &QNetworkReply::downloadProgress,
            this,              &QGCDownloadWatcher::progressChanged);

    connect(_netwkReply.get(), &QNetworkReply::finished,
            this,              &QGCDownloadWatcher::_onFinished);

    connect(_netwkReply.get(), static_cast<void (QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
            this,              &QGCDownloadWatcher::_onNetworkError);
}
