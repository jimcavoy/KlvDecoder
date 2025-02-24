#pragma once

#include <memory>

#include <boost/property_tree/ptree.hpp>

namespace pt = boost::property_tree;

class MiDemux
{
public:
    typedef std::function<void(const pt::ptree& klvset)> OnKlvSet;

public:
    MiDemux(float frequency);
    ~MiDemux();

    void read(const uint8_t* stream, size_t len);

    int reads() const;

    void setKlvSetCallback(OnKlvSet cb);

private:
    class Impl;
    std::unique_ptr<Impl> _pimpl;
};

