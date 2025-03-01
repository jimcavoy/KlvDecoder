#pragma once
#include <klvp/klvprsr.h>

#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

class KlvSecuritySetVisitor;

class KlvSecuritySetParserImpl :
    public lcss::KLVSecuritySetParser
{
public:
    KlvSecuritySetParserImpl(KlvSecuritySetVisitor& visitor);
    virtual ~KlvSecuritySetParserImpl(void);

    virtual void onBeginSet(int len, lcss::TYPE type);
    virtual void onElement(lcss::KLVElement& klv);
    virtual void onEndSet();
    virtual void onError(const char* errmsg, int pos);

    const pt::ptree& securitySet() const { return _securitySet; }

private:
    pt::ptree _securitySet;
    pt::ptree _elements;
    KlvSecuritySetVisitor& _visitor;
};

