#ifndef QGCXZDECOMPRESSOR_H
#define QGCXZDECOMPRESSOR_H

#include <QObject>
#include <QFile>
#include <memory>


class QGCXzDecompressor : public QObject
{
    Q_OBJECT
public:
    enum ErrorType {
        NoErrors = 0,
        OpenArchiveFailed,
        OpenDestFailed,
        ReadArchiveFailed,
        WriteDestFailed,
        MagicNumberError,
        CorruptData
    };

    static uint defaultBufferSize(void);

    static QString eraseXzSuffix(QString const& xzArchive);

    static void registerMetatypes(void) {
        qRegisterMetaType<ErrorType>("QGCXzDecompressor::ErrorType");
    }

    explicit QGCXzDecompressor(QObject *parent = nullptr);
    explicit QGCXzDecompressor(uint bufferSize, QObject *parent = nullptr);

    bool decompressAsync(QString const& archive, QString const& dest);

public slots:
    void cancel(void) { _needToCancel = true; }

signals:
    void progressChanged(qint64 curr, qint64 total);
    void error(QGCXzDecompressor::ErrorType);
    void success(QString destPath);
    void cancelled(void);

private:
    void _decompress(QString const& archive, QString const& dest);

    uint          _bufferSize;
    volatile bool _needToCancel;
};

#endif // QGCXZDECOMPRESSOR_H
