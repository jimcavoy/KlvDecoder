#include "KlvParserImpl.h"

#include "KlvDecodeVisitor.h"

#ifdef WIN32
const char* dbUrl = "file:///C:/PROGRA~1/klvp/share/klv.s3db";
#else
const char* dbUrl = "file:///usr/local/share/klv.s3db";
#endif

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
    lcss::KLVParser::onElement(klv);
    _count++;
    KlvDecodeVisitor vis(_elements, dbUrl);
    klv.Accept(vis);
}

void KlvParserImpl::onEndSet()
{
    lcss::KLVParser::onEndSet();

    switch (_type)
    {
    case lcss::TYPE::LOCAL_SET:
        _klvSet.put_child("metadata_set", _elements);
        break;
    case lcss::TYPE::UNIVERSAL_ELEMENT:
    case lcss::TYPE::SECURITY_UNIVERSAL_SET:
    case lcss::TYPE::UNIVERSAL_SET:
        break;
    }
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

void KlvParserImpl::clear()
{
    _klvSet.clear();
    _elements.clear();
}
