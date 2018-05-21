#ifndef RTCMHEADERPARSER_H
#define RTCMHEADERPARSER_H

#include <QtCore>

template<typename T, std::size_t N>
struct RtcmField {
    T data;
    enum { BitSize = N };
};

struct RtcmPreamble
{
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
    RtcmField<char,    5> sattCount;
    RtcmField<bool,    1> smoothIndicator;
    RtcmField<char,    3> smoothInterval;

    RtcmHeader(BitStream& bstream);
};


struct Rtcm1002
{
    static constexpr ushort messageId() { return 1002; }
    static constexpr QString system() { return "GPS"; }

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
    static constexpr QString system() { return "GLONASS"; }

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
    BitStream& fill(RtcmField<T, N>* field, std::size_t bitCount = 0)
    {
        auto offset = bitCount == 0 ?
                    field->BitSize : bitCount;

        field->data = static_cast<T>(_getbitu(_buffer, _pos, offset));
        pos += offset;
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
    qint64 _pos;
};

#endif
