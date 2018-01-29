#include <lzma.h>
#include <cstdio>
#include <memory>
#include <QtCore>
#include <QtConcurrent>

#include "QGCXzDecompressor.h"


static bool _metatypesWasRegistered = false;

namespace lzma {
    using LzmaStreamDeleter_t = std::function<void(lzma_stream*)>;

    lzma_ret                     initializeAsDecoder(lzma_stream* stream);
    std::unique_ptr<lzma_stream, LzmaStreamDeleter_t> makeLzmaStream(void);
    QGCXzDecompressor::ErrorType mapLzmaReturnValue(lzma_ret lzmaReturn);
}


lzma_ret lzma::initializeAsDecoder(lzma_stream* stream)
{
    return ::lzma_stream_decoder(stream, UINT64_MAX, LZMA_CONCATENATED);
}


std::unique_ptr<lzma_stream, lzma::LzmaStreamDeleter_t>
    lzma::makeLzmaStream(void)
{
    auto lzmaStreamDeleter =
            [] (lzma_stream* stream) { ::lzma_end(stream); delete stream; };

    return std::unique_ptr<lzma_stream, decltype(lzmaStreamDeleter)>
            (new lzma_stream(LZMA_STREAM_INIT), lzmaStreamDeleter);

}


QGCXzDecompressor::ErrorType lzma::mapLzmaReturnValue(lzma_ret lzmaReturn)
{
    if (lzmaReturn == LZMA_STREAM_END || lzmaReturn == LZMA_OK)  {
        return QGCXzDecompressor::ErrorType::NoErrors;
    }

    switch (lzmaReturn) {
        case LZMA_FORMAT_ERROR:
            return QGCXzDecompressor::MagicNumberError;

        case LZMA_DATA_ERROR:
            return QGCXzDecompressor::CorruptData;

        default:
            qWarning() << "Unspeciefed error";
            return QGCXzDecompressor::NoErrors;
    }
}


uint QGCXzDecompressor::defaultBufferSize(void)
{
    return 65536;
}

static auto const xzSuffix = QStringLiteral(".xz");

QString QGCXzDecompressor::eraseXzSuffix(QString const& xzArchiveName)
{
    if (xzArchiveName.endsWith(xzSuffix)) {
        auto nameCopy = xzArchiveName;
        nameCopy.truncate(nameCopy.length() - xzSuffix.length());
        return nameCopy;

    } else {
        return {};
    }
}


QGCXzDecompressor::QGCXzDecompressor(QObject* parent)
    : QObject(parent),
      _bufferSize(defaultBufferSize())
{
    if (!_metatypesWasRegistered) {
        registerMetatypes();
        _metatypesWasRegistered = true;
    }

}


QGCXzDecompressor::QGCXzDecompressor(uint bufferSize, QObject* parent)
    : QGCXzDecompressor(parent)
{
    Q_ASSERT(bufferSize != 0);
    _bufferSize = bufferSize;
}


bool QGCXzDecompressor::decompressAsync(QString const& archive, QString const& dest)
{
    return QtConcurrent::run(
        [this, archive, dest] () { _decompress(archive, dest); }
    ).isRunning();
}


void QGCXzDecompressor::_decompress(QString const& archivePath, QString const& destFilePath)
{
    _needToCancel = false;

    struct {
        QFile archive;
        QFile dest;
    } files { {archivePath}, {destFilePath} };

    struct {
        lzma_action current_action;
        std::unique_ptr<lzma_stream, lzma::LzmaStreamDeleter_t> stream;
        lzma_ret retval;
    } lzma { LZMA_RUN, lzma::makeLzmaStream(), LZMA_OK };

    using Buffer_t = std::unique_ptr<char[]>;
    struct {
        Buffer_t compressed, decompressed;
    } bufs = {
        Buffer_t(new char[_bufferSize]),
        Buffer_t(new char[_bufferSize])
    };

    if (!files.archive.open(QIODevice::ReadOnly)) {
        emit error(ErrorType::OpenArchiveFailed);
        return;
    }

    if (!files.dest.open(QIODevice::WriteOnly)) {
        emit error(ErrorType::OpenDestFailed);
        return;
    }

    lzma.retval = lzma::initializeAsDecoder(lzma.stream.get());
    auto mappedError = lzma::mapLzmaReturnValue(lzma.retval);

    if (mappedError != ErrorType::NoErrors) {
        emit error(mappedError);
        return;
    }

    lzma.stream->next_in   = nullptr;
    lzma.stream->avail_in  = 0;
    lzma.stream->next_out  = reinterpret_cast<uint8_t*>(bufs.decompressed.get());
    lzma.stream->avail_out = _bufferSize;

    auto lastProgress = 0u;

    while (true) {
        // if all compressed data was decomressed and file not at end - read new data
        if (lzma.stream->avail_in == 0 && !files.archive.atEnd()) {
            lzma.stream->next_in  = reinterpret_cast<uint8_t*>(bufs.compressed.get());
            lzma.stream->avail_in = files.archive.read(bufs.compressed.get(), _bufferSize);

            if (files.archive.error() != QFileDevice::NoError) {
                emit error(ErrorType::ReadArchiveFailed);
                return;
            }

            if (files.archive.atEnd()) {
                lzma.current_action = LZMA_FINISH;
            }
        }

        lzma.retval = lzma_code(lzma.stream.get(), lzma.current_action);

        if (lzma.stream->avail_out == 0 || lzma.retval == LZMA_STREAM_END) {
            qint64 writeSize = _bufferSize - lzma.stream->avail_out;

            if (files.dest.write(bufs.decompressed.get(), writeSize) != writeSize) {
                emit error(ErrorType::WriteDestFailed);
                return;
            }

            lzma.stream->next_out  = reinterpret_cast<uint8_t*>(bufs.decompressed.get());
            lzma.stream->avail_out = _bufferSize;
        }

        auto currentProgress = (lzma.stream->total_in * 100) / files.archive.size();

        if (currentProgress != lastProgress) {
            lastProgress = currentProgress;
            emit progressChanged(lzma.stream->total_in, files.archive.size());
        }

        if (_needToCancel) {
            _needToCancel = false;
            emit cancelled();
            return;
        }

        if (lzma.retval != LZMA_OK) {
            if (lzma.retval == LZMA_STREAM_END) {
                emit success(destFilePath);
            } else {
                emit error(lzma::mapLzmaReturnValue(lzma.retval));
            }
            return;
        }
    }
}
