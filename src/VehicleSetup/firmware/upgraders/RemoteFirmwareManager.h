#ifndef REMOTEFIRMWAREMANAGER_H
#define REMOTEFIRMWAREMANAGER_H

#include <QtCore>
#include <QtNetwork>

#include "QGCXzDecompressor.h"
#include "QGCDownload.h"

class QGCFileDownload;

class RemoteFirmwareManager : public QObject {
    Q_OBJECT
public:
    RemoteFirmwareManager(QString const& sourceUrl = "",
                          QString const& destDir = "",
                          QObject* parent = nullptr);

    QString const& sourceUrl(void) const { return _sourcePath; }
    QString destDirPath(void) const { return QFileInfo(_destFilePath).dir().path(); }

    bool cached(void) const {
        return _cache.downloadingSuccess && QFileInfo(_cache.downloadedArchivePath).exists()
            && _cache.extractingSuccess && QFileInfo(_cache.extractedArchivePath).exists();
    }

    void setSourceUrl(QString const& sourceUrl) {
        resetCache();
        _sourcePath = sourceUrl;
        _destFilePath = QFileInfo(_destFilePath).dir().path() + "/" + QFileInfo(sourceUrl).fileName();
    }

    void setDestDirPath(QString const& destDirPath) {
        resetCache();
        _destFilePath = destDirPath + "/"
                + QFileInfo(_sourcePath).fileName();
    }


    bool asyncRun(void);

public slots:
    void cancel(void) { emit _cancel(); }
    void resetCache(void) { _cache.reset(); }

signals:
    void firmwareDownloaded(QString path);
    void firmwareExtracted (QString path);

    void archiveCached(void);
    void firmwareCached(void);
    void savingError(void);

    void networkError    (QNetworkReply::NetworkError error);
    void decompressError (QGCXzDecompressor::ErrorType error);
    void progressChanged (qint64 curr, qint64 total);
    void cancelled       (void);
    void _cancel         (void);

private:
    bool _extractAsync(QString archivePath);
    void _attachDownloadWatcher(QGCDownloadWatcher* watcher);

    struct Cache {
        Cache(void)
            : downloadingSuccess(false),
              extractingSuccess(false),
              extractedArchivePath(""),
              downloadedArchivePath("")
        {}

        void reset(void) {
            downloadingSuccess = extractingSuccess = false;
            extractedArchivePath = downloadedArchivePath = "";
        }

        bool downloadingSuccess;
        bool extractingSuccess;
        QString extractedArchivePath;
        QString downloadedArchivePath;
    } _cache;

    QString                             _destFilePath;
    QString                             _sourcePath;
    QGCXzDecompressor*                  _decompressor;

    std::unique_ptr<QGCDownloadWatcher> _downloadWatcher;
    std::shared_ptr<QIODevice>          _iodevice;
};

#endif // REMOTEFIRMWAREMANAGER_H
