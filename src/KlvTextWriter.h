#pragma once

#include <memory>
#include <string>

class KlvTextWriter
{
public:
    KlvTextWriter(const char* ipaddr, unsigned short port, unsigned char ttl, const char* iface_addr);
    ~KlvTextWriter();

    void send(const std::string& data);

public:
    class Impl;

private:
    std::unique_ptr<Impl> _pimpl;
};

