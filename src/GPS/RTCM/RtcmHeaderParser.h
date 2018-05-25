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


struct Rtcm1002
{
    static constexpr ushort messageId() { return 1002; }
    static QString system() { return "GPS"; }

    RtcmField<char,      6> sattId;
    RtcmField<bool,      1> codeIndicator;
    RtcmField<uint32_t, 24> pseudorange;
    RtcmField<int32_t,  20> phaserange;
    RtcmField<uint8_t,   7> lockTimeIndicator;
    RtcmField<uint8_t,   8> ambiguity;
    RtcmField<uint8_t,   8> cnr;

    Rtcm1002(BitStream& bstream);
};


struct Rtcm1010
{
    static constexpr ushort messageId() { return 1010; }
    static QString system() { return "GLONASS"; }

    RtcmField<char,      6> sattId;
    RtcmField<bool,      1> codeIndicator;
    RtcmField<uint8_t,   5> sattFreqChannelNumber;
    RtcmField<uint32_t, 25> pseudorange;
    RtcmField<int32_t,  20> phaserange;
    RtcmField<uint8_t,   7> lockTimeIndicator;
    RtcmField<uint8_t,   8> ambiguity;
    RtcmField<uint8_t,   8> cnr;

    Rtcm1010(BitStream& bstream);
};


class BitStream {
public:
    BitStream(QByteArray array)
        : _buffer(std::move(array)),
          _pos(0)
    { }

    template<typename T, std::size_t N>
    BitStream& fillField(RtcmField<T, N>* field, std::size_t bitCount = 0)
    {
        auto offset = bitCount == 0 ?
                    static_cast<std::size_t>(field->BitSize) : bitCount;

        field->data = static_cast<T>(_getbitu(_buffer, _pos, offset));
        _pos += offset;
        return *this;
    }

    BitStream& fillArray(QByteArray* array, std::size_t bytesCount)
    {
        auto bytePos = _pos / 8;
        Q_ASSERT(bytePos + bytesCount<= static_cast<std::size_t>(_buffer.size()));

        for (auto i = bytePos; i < bytePos + bytesCount; i++) {
            char c = _buffer[(uint)i];
            array->append(c);
        }

        _pos += bytesCount * 8;

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
    std::size_t _pos;
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
    } _sattelitesCount;
};

#endif
