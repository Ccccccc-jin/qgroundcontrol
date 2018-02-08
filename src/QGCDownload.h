#ifndef QGCDOWNLOAD_H
#define QGCDOWNLOAD_H

#include <QtCore>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <functional>
#include <memory>

class QGCDownloadWatcher;


class QGCDownload {
    friend class QGCDownloadWatcher;
public:
    /**
     * @brief async download to object which inherits form QIODevice
     * @param remotePath url of remote file
     * @param iodevice iodevice which should be opened for writing
     * @return QGCDownloadWatcher object which represents the process of downloading.
     *         If downloading is failed, download() returns invalid pointer (nullptr)
     */
    static std::unique_ptr<QGCDownloadWatcher>
        download(QString const& remotePath, std::shared_ptr<QIODevice> iodevice);

    /**
     * @brief async in memory download
     * @param remotePath url of remote file
     * @param byteArray storage for future data
     * @return QGCDownloadWatcher object which represents the process of downloading.
     *         If downloading is failed, download() returns invalid pointer (nullptr)
     */
    static std::unique_ptr<QGCDownloadWatcher>
        download(QString const& remotePath, std::shared_ptr<QByteArray> byteArray);

protected:
    QGCDownload() {}

private:
    using _onReadyReadCallback_t = std::function<void(QNetworkReply*, QGCDownloadWatcher*)>;

    static std::unique_ptr<QGCDownloadWatcher>
        _download(QString const& remotePath, _onReadyReadCallback_t onReadyRead);
};


class QGCDownloadWatcher : public QObject
{
    friend class QGCDownload;
    Q_OBJECT
public:
    bool isValid(void) const { return _isValid; }

signals:
    void success(void);
    void progressChanged(qint64 curr, qint64 total);
    void networkError(QNetworkReply::NetworkError error);
    void savingError(void);
    void cancelled(void);

public slots:
    void cancel(void);

private slots:
    void _onNetworkAccessibilityChanged(
            QNetworkAccessManager::NetworkAccessibility access);
    void _onNetworkError(QNetworkReply::NetworkError error);
    void _onSavingError(void);
    void _onFinished(void);

private:
    QGCDownloadWatcher(QObject* parent = nullptr);
    QGCDownloadWatcher(std::unique_ptr<QNetworkReply>&& reply,
                       QObject* parent = nullptr);

    void _initConnections(void);

    bool                           _isValid;
    std::unique_ptr<QNetworkReply> _netwkReply;
};


#endif // QGCDOWNLOAD_H
