#pragma once

#include <memory>

class MiDemux
{
public:
    MiDemux(int reads, float frequency);
    ~MiDemux();

    void read(const uint8_t* stream, size_t len);

private:
    class Impl;
    std::unique_ptr<Impl> _pimpl;
};

