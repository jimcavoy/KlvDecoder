#pragma once

#include <memory>

class MiDemux
{
public:
    MiDemux(float frequency);
    ~MiDemux();

    void read(const uint8_t* stream, size_t len);

    int reads() const;

private:
    class Impl;
    std::unique_ptr<Impl> _pimpl;
};

