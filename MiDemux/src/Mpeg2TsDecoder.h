#pragma once

#include <mp2tp/libmp2tp.h>

#include "PmtProxy.h"
#include "AccessUnit.h"
#include "Clock.h"
#include "KlvParserImpl.h"


class CmdLineParser;

class Mpeg2TsDecoder
    : public lcss::TSParser
{
public:
    Mpeg2TsDecoder(float frequency);

    void onPacket(lcss::TransportPacket& pckt);

    int reads() const;

private:
    void processStartPayload(const lcss::TransportPacket& pckt);
    void processPayload(const lcss::TransportPacket& pckt);
    void processKlv(const lcss::PESPacket& pes);
    void outputSet();
    void updateSystemClock(const lcss::TransportPacket& pckt);

private:
    lcss::ProgramAssociationTable _pat{};
    lcss::ProgramMapTable _pmt{};
    PmtProxy _pmtHelper;
    AccessUnit _klvSample;
    int _reads{ 0 };
    uint64_t _interval{};
    uint64_t _time{};
    PCRClock _pcrClock;
    SystemClock _systemClock;
    KlvParserImpl _klvParser;
};

