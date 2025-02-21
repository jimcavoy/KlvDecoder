#include "Mpeg2TsDecoder.h"

#include <iterator>


Mpeg2TsDecoder::Mpeg2TsDecoder(float frequency)
    : _interval((uint64_t)(90000 / frequency))
{
    
}

void Mpeg2TsDecoder::onPacket(lcss::TransportPacket& pckt)
{
    updateSystemClock(pckt);

    if (pckt.payloadUnitStart())
    {
        processStartPayload(pckt);
    }
    else
    {
        processPayload(pckt);
    }
}

int Mpeg2TsDecoder::reads() const
{
    return _reads;
}


void Mpeg2TsDecoder::processStartPayload(const lcss::TransportPacket& pckt)
{
    const BYTE* data = pckt.getData();
    if (pckt.PID() == 0 && _pat.size() == 0) // Program Association Table
    {
        _pat.parse(data);
    }
    else if (_pat.find(pckt.PID()) != _pat.end())
    {
        auto it = _pat.find(pckt.PID());
        if (it->second != 0) // program 0 is assigned to Network Information Table
        {
            _pmt = lcss::ProgramMapTable(data, pckt.data_byte());
            // True if the PMT fits in one TS packet
            if (_pmt.parse())
            {
                _pmtHelper.update(_pmt);
            }
        }
    }
    else
    {
        lcss::PESPacket pes;
        UINT16 bytesParsed = pes.parse(data);
        if (bytesParsed > 0)
        {
            if (_pmtHelper.packetType(pckt.PID()) == PmtProxy::STREAM_TYPE::KLVA)
            {
                if (_klvSample.length() > 0)
                {
                    processKlv(pes);
                }
                _klvSample.clear();
                _klvSample.insert((char*)data + bytesParsed, pckt.data_byte() - bytesParsed);
            }
        }
    }
}

void Mpeg2TsDecoder::processPayload(const lcss::TransportPacket& pckt)
{
    const BYTE* data = pckt.getData();
    // PMT spans across two or more TS packets
    auto it = _pat.find(pckt.PID());
    if (it != _pat.end() && it->second != 0)
    {
        _pmt.add(data, pckt.data_byte());
        if (_pmt.parse())
        {
            _pmtHelper.update(_pmt);
        }
    }

    if (_pmtHelper.packetType(pckt.PID()) == PmtProxy::STREAM_TYPE::KLVA)
    {
        _klvSample.insert((char*)data, pckt.data_byte());
    }
}

void Mpeg2TsDecoder::processKlv(const lcss::PESPacket& pes)
{
    _reads++;
    UINT64 timeNow = _systemClock.time();
    UINT64 diff = timeNow - _time;
    BYTE stream_id = pes.stream_id();

    //if (diff > _interval)
    /*{
        _time = timeNow;
        _klvParser.parse({ (uint8_t*)_klvSample.data(), _klvSample.length() });
        outputXmlSet();
    }*/

    if (stream_id == 0xFC)
    {
        lcss::MetadataAUWrapper wrapper;
        size_t count = wrapper.parse((BYTE*)_klvSample.data(), _klvSample.length());

        if (count > 0)
        {
            lcss::MetadataAUWrapper::const_iterator it;
            for (it = wrapper.begin(); it != wrapper.end(); ++it)
            {
                _klvParser.parse({ (BYTE*)it->AU_cell_data_bytes(), (UINT32)it->AU_cell_data_length() });
                outputSet();
            }
            return;
        }
        else // missed labeled metadata stream
        {
            stream_id = 0xBD;
        }

        if (stream_id == 0xBD)
        {
            _klvParser.parse({ (BYTE*)_klvSample.data(), (UINT32)_klvSample.length() });
            outputSet();
        }
    }
}

void Mpeg2TsDecoder::outputSet()
{

    //std::string xmlSet = _klvParser.xmlSet();
    //_sender.send(xmlSet.c_str(), (int)xmlSet.size());
    //_klvParser.reset();
}

void Mpeg2TsDecoder::updateSystemClock(const lcss::TransportPacket& pckt)
{
    char afe = pckt.adaptationFieldExist();
    if (afe == 0x02 || afe == 0x03)
    {
        UINT64 time;
        const lcss::AdaptationField* adf = pckt.getAdaptationField();
        if (adf->length() > 0 && adf->PCR_flag())
        {
            BYTE pcr[6]{};
            adf->getPCR(pcr);
            _pcrClock.setTime(pcr);
            time = _pcrClock.baseTime();  //Use the 90 kHz clock
            _systemClock.setTime(time);
        }
    }
}