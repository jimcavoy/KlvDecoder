
#include "MiDemux.h"

#include "Mpeg2TsDecoder.h"

class MiDemux::Impl
{
public:
    Impl(float frequency)
        :_decoder(std::make_unique<Mpeg2TsDecoder>(frequency))
    {

    }

    std::unique_ptr<Mpeg2TsDecoder> _decoder;
};


MiDemux::MiDemux(float frequency)
    :_pimpl(std::make_unique<MiDemux::Impl>(frequency))
{
}

MiDemux::~MiDemux()
{
}

void MiDemux::read(const uint8_t* stream, size_t len)
{
    _pimpl->_decoder->parse(stream, (uint32_t) len);
}

int MiDemux::reads() const
{
    return _pimpl->_decoder->reads();
}

void MiDemux::setKlvSetCallback(MiDemux::OnKlvSet cb)
{
    _pimpl->_decoder->setCallback(cb);
}
