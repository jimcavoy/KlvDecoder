#pragma once
#include <map>

#include <mp2tp/libmp2tp.h>

class PmtProxy
{
public:
    enum class STREAM_TYPE
    {
        UNKNOWN,
        H264,
        H265,
        HDMV,
        VIDEO,
        AUDIO,
        KLVA,
        $EXI,
        $XML
    };

private:
    typedef std::map<unsigned short, PmtProxy::STREAM_TYPE> map_type;

public:
    void update(const lcss::ProgramMapTable& pmt);
    STREAM_TYPE packetType(unsigned short pid);
    bool isEmpty() const noexcept
    {
        return _pid2type.empty();
    }

private:
    map_type _pid2type{};
    uint8_t _version{ 0xFF };
};

