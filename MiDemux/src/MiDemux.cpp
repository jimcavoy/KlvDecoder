
#include "MiDemux.h"

#include "Mpeg2TsDecoder.h"

class MiDemux::Impl
{
public:
    Impl(int reads, float frequency)
        :_decoder(std::make_unique<Mpeg2TsDecoder>(reads, frequency))
    {

    }

    std::unique_ptr<Mpeg2TsDecoder> _decoder;
};


MiDemux::MiDemux(int reads, float frequency)
    :_pimpl(std::make_unique<MiDemux::Impl>(reads, frequency))
{
}

MiDemux::~MiDemux()
{
}

void MiDemux::read(const uint8_t* stream, size_t len)
{
    _pimpl->_decoder->parse(stream, (uint32_t) len);
}
