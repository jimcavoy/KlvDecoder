#pragma once

#include <mp2tp/libmp2tp.h>

#include "PmtProxy.h"
#include "AccessUnit.h"
#include "Clock.h"


class CmdLineParser;

class Mpeg2TsDecoder
    : public lcss::TSParser
{
public:
    Mpeg2TsDecoder(int count, float frequency);

    void onPacket(lcss::TransportPacket& pckt);

private:
    void processStartPayload(const lcss::TransportPacket& pckt);
    void processPayload(const lcss::TransportPacket& pckt);
    void processKlv();
    void outputXmlSet();
    void updateSystemClock(const lcss::TransportPacket& pckt);

private:
    lcss::ProgramAssociationTable _pat{};
    lcss::ProgramMapTable _pmt{};
    PmtProxy _pmtHelper;
    AccessUnit _klvSample;
    int _reads{};
    UINT64 _interval;
    UINT64 _time;
    PCRClock _pcrClock;
    SystemClock _systemClock;
};

