#pragma once

#include <klvp/klvprsr.h>
#include <boost/property_tree/ptree.hpp>
#include <string>

namespace pt = boost::property_tree;

class KlvParserImpl :
    public lcss::KLVParser
{
public:
    KlvParserImpl();
    KlvParserImpl(std::string klvdbFilepath);
    virtual ~KlvParserImpl();

    void onBeginSet(int len, lcss::TYPE type) override;
    void onElement(lcss::KLVElement& klv) override;
    void onEndSet() override;
    void onError(const char* errmsg, int pos) override;

    int count() const;

    const pt::ptree& klvSet() const;
    void clear();

    void setKlvdbFilepath(const std::string& filepath);

private:
    int                 _count{ 0 };
    lcss::TYPE          _type{ lcss::TYPE::LOCAL_SET };
    pt::ptree           _klvSet;
    pt::ptree           _elements;
    std::string         _klvdbFilepath;
};