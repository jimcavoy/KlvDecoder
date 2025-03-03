#pragma once

#include <memory>

class UdpSender
{
public:
    UdpSender(const char* ipaddr, unsigned short port, unsigned char ttl, const char* iface_addr);
    ~UdpSender();

    void send(const char* buffer, size_t len);

private:
    class Impl;
    std::unique_ptr<Impl> _pimpl;
};

