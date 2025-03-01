#include "KlvSecuritySetParserImpl.h"

#include "KlvSecuritySetVisitor.h"

#include <klvp/klvelmt.h>
#include <klvp/klvelmtimpl.h>

using namespace std;

KlvSecuritySetParserImpl::KlvSecuritySetParserImpl(KlvSecuritySetVisitor& visitor)
    : _visitor(visitor)
{
}


KlvSecuritySetParserImpl::~KlvSecuritySetParserImpl(void)
{
}

void KlvSecuritySetParserImpl::onBeginSet(int len, lcss::TYPE type)
{
    lcss::KLVParser::onBeginSet(len, type);
}

void KlvSecuritySetParserImpl::onEndSet()
{
    lcss::KLVParser::onEndSet();
}

void KlvSecuritySetParserImpl::onElement(lcss::KLVElement& klv)
{
    lcss::KLVParser::onElement(klv);
    klv.Accept(_visitor);
}

void KlvSecuritySetParserImpl::onError(const char* errmsg, int pos)
{

}