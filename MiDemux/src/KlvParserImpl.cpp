#include "KlvParserImpl.h"

#include "KlvDecodeVisitor.h"

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

    switch (_type)
    {
    case lcss::TYPE::LOCAL_SET:
        _klvSet.put("localSet", len);
        break;
    case lcss::TYPE::UNIVERSAL_ELEMENT:
    case lcss::TYPE::SECURITY_UNIVERSAL_SET:
    case lcss::TYPE::UNIVERSAL_SET:
        _klvSet.put("universalSet", len);
        break;
    }
}

void KlvParserImpl::onElement(lcss::KLVElement& klv)
{
    lcss::KLVParser::onElement(klv);
    _count++;
    KLVDecodeVisitor vis(_klvSet, "file:klv.s3db");
    klv.Accept(vis);
}

void KlvParserImpl::onEndSet()
{
}

void KlvParserImpl::onError(const char* errmsg, int pos)
{
}

int KlvParserImpl::count() const
{
    return _count;
}

const pt::ptree& KlvParserImpl::klvSet() const
{
    return _klvSet;
}
