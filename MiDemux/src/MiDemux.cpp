
#include "MiDemux.h"

#include "Mpeg2TsDecoder.h"

class MiDemux::Impl
{
public:
    Impl()
        :_args(args)
        , _decoder(std::make_unique<Mpeg2TsDecoder>())
    {

    }

    std::unique_ptr<Mpeg2TsDecoder> _decoder;

};


MiDemux::MiDemux()
    :_pimpl(std::make_unique<MiDemux::Impl>())
{
}

MiDemux::~MiDemux()
{
}

void MiDemux::read(const uint8_t* stream, size_t len)
{
    _pimpl->_decoder->parse(stream, (uint32_t) len);
}
