#include "KlvParserImpl.h"

KlvParserImpl::KlvParserImpl()
{
    validateChecksum(true);
}

KlvParserImpl::~KlvParserImpl()
{
}

void KlvParserImpl::onBeginSet(int len, lcss::TYPE type)
{
    lcss::KLVParser::onBeginSet(len, type);
    _type = type;
}

void KlvParserImpl::onElement(lcss::KLVElement& klv)
{
}

void KlvParserImpl::onEndSet()
{
}

void KlvParserImpl::onError(const char* errmsg, int pos)
{
}

int KlvParserImpl::count() const
{
    return 0;
}
