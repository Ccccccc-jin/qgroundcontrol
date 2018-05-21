#include "RtcmHeaderParser.h"


namespace impl {
    constexpr char rtcmPreamble(void) { return 0xD3; }

    static uint getbitu(QByteArray const& buff, uint pos, uint len)
    {
        uint bits = 0;
        uint i;

        for (i = pos; i < pos + len; i++)
            bits = (uint) ((bits << 1) + ((buff[i/8] >> (int) (7 - i%8)) & 1u));

        return bits;
    }
}


using Sz = RtcmPreamble::BitSize;

RtcmPreamble::RtcmPreamble(BitStream& bstream)
{
    bstream.fill(&preamble)
           .fill(&reserved)
           .fill(&length);
}


RtcmHeader::RtcmHeader(BitStream& bstream)
{
    bstream.fill(&msgid)
           .fill(&refStationId)
           .fill(&epochTime, msgid <= 1012 || msgid >= 1009 ? 27 : 0)
           .fill(&syncGnssFlag)
           .fill(&sattCount)
           .fill(&smoothIndicator)
           .fill(&smoothInterval);
}


Rtcm1002::Rtcm1002(BitStream& bstream)
{
    bstream.fill(&sattId)
           .fill(&codeIndicator)
           .fill(&pseudorange)
           .fill(&phaserange)
           .fill(&lockTimeIndicator)
           .fill(&ambiguity)
           .fill(&cnr);
}


Rtcm1010::Rtcm1010(BitStream& bstream)
{
    bstream.fill(&sattId)
           .fill(&codeIndicator)
           .fill(&sattFreqChannelNumber)
           .fill(&pseudorange)
           .fill(&phaserange)
           .fill(&lockTimeIndicator)
           .fill(&ambiguity)
           .fill(&cnr);
}


void RtcmHeaderParser::parseRtcm(QByteArray rtcmData)
{

}
