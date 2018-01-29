#include "RemoteFirmwareManager.h"
#include "QGCFileDownload.h"
#include "QGCXzDecompressor.h"
#include "QGCDownload.h"

RemoteFirmwareManager::RemoteFirmwareManager(QString const& sourceUrl, QString const& destDir, QObject* parent)
    : QObject(parent),
      _destFilePath(destDir + QFileInfo(sourceUrl).fileName()),
      _sourcePath(sourceUrl),
      _decompressor(new QGCXzDecompressor(this))
{
    QObject::connect(this,          &RemoteFirmwareManager::_cancel,
                    _decompressor, &QGCXzDecompressor::cancel);

    QObject::connect(_decompressor, &QGCXzDecompressor::error,
                     this,          &RemoteFirmwareManager::decompressError);

    QObject::connect(_decompressor, &QGCXzDecompressor::success,
                     this,          &RemoteFirmwareManager::firmwareExtracted);

    QObject::connect(_decompressor, &QGCXzDecompressor::success,
        [this] (QString extractedArchivePath) {
            _cache.extractingSuccess = true;
            _cache.extractedArchivePath = extractedArchivePath;
        }
    );

    QObject::connect(_decompressor, &QGCXzDecompressor::cancelled,
                     this,          &RemoteFirmwareManager::cancelled);

    QObject::connect(_decompressor, &QGCXzDecompressor::progressChanged,
                     this,          &RemoteFirmwareManager::progressChanged);
}


bool RemoteFirmwareManager::asyncRun(void)
{
    if (!_cache.downloadingSuccess || !QFileInfo(_cache.downloadedArchivePath).exists()) {
        _iodevice.reset(new QFile(_destFilePath));

        if (!_iodevice->open(QIODevice::WriteOnly)) {
            qWarning() << QString("Can not open device with %1 name").arg(_destFilePath);
            return false;
        }

        _downloadWatcher = QGCDownload::download(_sourcePath, _iodevice);
        if (_downloadWatcher) {
            _attachDownloadWatcher(_downloadWatcher.get());
        } else {
            return false;
        }
        return true;

    } else if (!_cache.extractingSuccess || !QFileInfo(_cache.extractedArchivePath).exists()) {
        emit archiveCached();
        return _extractAsync(_cache.downloadedArchivePath);

    } else {
        emit archiveCached();
        emit firmwareCached();
        return true;
    }
}


bool RemoteFirmwareManager::_extractAsync(QString archivePath)
{
    Q_ASSERT(!archivePath.isEmpty());

    auto destFilePath = QGCXzDecompressor::eraseXzSuffix(archivePath);

    return _decompressor->decompressAsync(archivePath, destFilePath);
}


void RemoteFirmwareManager::_attachDownloadWatcher(QGCDownloadWatcher *watcher)
{
    connect(watcher, &QGCDownloadWatcher::networkError,
            this,    &RemoteFirmwareManager::networkError);

    QObject::connect(watcher, &QGCDownloadWatcher::cancelled,
        [this] (void) {
            _iodevice->close();
            emit cancelled();
        }
    );

    connect(this,    &RemoteFirmwareManager::_cancel,
            watcher, &QGCDownloadWatcher::cancel);

    connect(watcher, &QGCDownloadWatcher::savingError,
            this,    &RemoteFirmwareManager::savingError);

    QObject::connect(watcher, &QGCDownloadWatcher::progressChanged,
                     this,     &RemoteFirmwareManager::progressChanged);

    QObject::connect(watcher, &QGCDownloadWatcher::success,
        [this] (void) {
            Q_ASSERT(_iodevice);
            _iodevice->close();

            _cache.downloadingSuccess = true;
            _cache.downloadedArchivePath = _destFilePath;

            emit firmwareDownloaded(_destFilePath);
            _extractAsync(_cache.downloadedArchivePath);
        }
    );
}
