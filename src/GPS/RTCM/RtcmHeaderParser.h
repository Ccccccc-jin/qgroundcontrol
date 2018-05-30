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

    RtcmField<uint8_t,   8> preamble;
    RtcmField<uint8_t,   6> reserved;
    RtcmField<uint16_t, 10> length;

    RtcmPreamble(BitStream& bstream);
};


struct RtcmHeader
{
    RtcmField<uint16_t, 12> refStationId;
    RtcmField<uint32_t, 30> epochTime;
    RtcmField<uint8_t,   1> syncGnssFlag;
    RtcmField<uint8_t,   5> sattCount;
    RtcmField<uint8_t,   1> smoothIndicator;
    RtcmField<uint8_t,   3> smoothInterval;

    RtcmHeader(BitStream& bstream, uint msgid);
};


struct MSMHeader {
    RtcmField<uint16_t, 12> refStationId;
    RtcmField<uint32_t, 30> epochTime;
    RtcmField<uint8_t,   1> multMsgBit;

    RtcmField<uint8_t,   3> iods;
    RtcmField<uint8_t,   7> reserved;
    RtcmField<uint8_t,   2> clkInd;
    RtcmField<uint8_t,   2> extClkInd;

    RtcmField<uint8_t,   1> smoothIndicator;
    RtcmField<uint8_t,   3> smoothInterval;
    RtcmField<uint64_t, 64> satteliteMask;
    RtcmField<uint32_t, 32> signalMask;
    RtcmField<uint64_t, 64> cellMask;

    MSMHeader(BitStream& bstream);
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
        /*qDebug() << "Field bit size: " << offset;
        qDebug() << "Current bit pos: " << _bitPos;
        qDebug() << "Buffer size: " << _buffer.length();*/
        Q_ASSERT((offset + _bitPos) / 8 <= _buffer.length());

        field->data = static_cast<T>(_getbitu(_buffer, _bitPos, offset));
        _bitPos += offset;
        return *this;
    }

    BitStream& fillArray(QByteArray* array, std::size_t bytesCount)
    {
        /*qDebug() << "Field byte size: " << bytesCount;
        qDebug() << "Current bit pos: " << _bitPos;
        qDebug() << "Buffer size: " << _buffer.length();*/
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
    static uint _getbitu(QByteArray const& buff, uint pos, uint len)
    {
        uint bits = 0;
        uint i;

        for (i = pos; i < pos + len; i++)
            bits = (uint) ((bits << 1) + ((buff[i/8] >> (int) (7 - i%8)) & 1u));

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
    void sattsCountChanged(uint sattsCount);

private:
    void _sattsCountChanged(void);
    void _parsePayload(QByteArray payload);

    struct {
        uint gpsSatts;
        uint glonassSatts;
        uint sbasSatts;
        uint beidouSatts;
        uint qzssSatts;
        uint galileoSatts;
    } _sattelitesCount;
};

#endif
