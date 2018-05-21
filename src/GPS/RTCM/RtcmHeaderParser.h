#ifndef RTCMHEADERPARSER_H
#define RTCMHEADERPARSER_H

#include <QtCore>

template<typename T, std::size_t N>
struct RtcmField {
    T data;
    enum : char { BitSize = N };
};

class BitStream;

struct RtcmPreamble
{
    static constexpr uint32_t preambleByteSize() { return 3; }

    RtcmField<char,      8> preamble;
    RtcmField<char,      6> reserved;
    RtcmField<uint16_t, 10> length;

    RtcmPreamble(BitStream& bstream);
};


struct RtcmHeader
{
    RtcmField<ushort, 12> msgid;
    RtcmField<ushort, 12> refStationId;
    RtcmField<uint,   30> epochTime;
    RtcmField<bool,    1> syncGnssFlag;
    RtcmField<uint8_t, 5> sattCount;
    RtcmField<bool,    1> smoothIndicator;
    RtcmField<char,    3> smoothInterval;

    RtcmHeader(BitStream& bstream);
};


class BitStream {
public:
    BitStream(QByteArray array)
        : _buffer(std::move(array)),
          _bitPos(0)
    { }

    template<typename T, std::size_t N>
    BitStream& fillField(RtcmField<T, N>* field, std::size_t bitCount = 0)
    {
        auto offset = bitCount == 0 ?
                    static_cast<std::size_t>(field->BitSize) : bitCount;
        auto bufSize = static_cast<std::size_t>(_buffer.length());

        Q_ASSERT((offset + _bitPos) / 8 <= bufSize);

        field->data = static_cast<T>(_getbitu(_buffer, _bitPos, offset));
        _bitPos += offset;
        return *this;
    }

    BitStream& fillArray(QByteArray* array, std::size_t bytesCount)
    {
        auto bytePos = _bitPos / 8;
        Q_ASSERT(bytePos + bytesCount<= static_cast<std::size_t>(_buffer.size()));

        for (auto i = bytePos; i < bytePos + bytesCount; i++) {
            char c = _buffer[(uint)i];
            array->append(c);
        }

        _bitPos += bytesCount * 8;

        return *this;
    }

private:
    static uint64_t _getbitu(QByteArray const& buff, uint64_t pos, uint len)
    {
        uint64_t bits = 0;
        uint64_t i;

        for (i = pos; i < pos + len; i++)
            bits = (uint64_t) ((bits << 1) + ((buff[(uint)i/8] >> (int) (7 - i%8)) & 1u));

        return bits;
    }

    QByteArray _buffer;
    std::size_t _bitPos;
};


class RtcmHeaderParser : public QObject
{
    Q_OBJECT
public:
    RtcmHeaderParser(QObject* parent = nullptr);

public slots:
    void onRtcmMessageReceived(QByteArray buffer);

signals:
    void satsCountChanged(uint sattsCount);

private:
    void _satsCountChanged(void);
    void _parsePayload(QByteArray payload);

    struct {
        uint gpsSatts;
        uint glonassSatts;
    } _sattelitesCount;
};

#endif
