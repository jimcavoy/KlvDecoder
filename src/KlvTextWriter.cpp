#include "KlvTextWriter.h"

#include <iostream>
#include <fcntl.h>
#include <string.h>

#ifdef _WIN32
#include <io.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#endif

#ifdef _WIN32
#define sprintf sprintf_s
#else
#define sprintf sprintf
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define PVOID void*
#define SD_SEND SHUT_WR
#define SOCKADDR sockaddr
#define IN_ADDR in_addr
#endif


using namespace std;
const int BUFLEN = 1000;

class KlvTextWriter::Impl
{
public:
    Impl() {}
    virtual ~Impl() {}

public:
    virtual void send(const char* data, size_t length) = 0;
};

class ConsoleImpl
    : public KlvTextWriter::Impl
{
public:
    ConsoleImpl() {}
    virtual ~ConsoleImpl() {}

public:
    void send(const char* data, size_t length) override;
};

class UdpImpl
    : public KlvTextWriter::Impl
{
public:
    UdpImpl(const char* ipaddr, uint16_t port, unsigned char ttl, const char* iface_addr)
        : _port(port)
    {
        initSocket(ipaddr, port, ttl, iface_addr);
    }

    virtual ~UdpImpl()
    {
        if (_socket != INVALID_SOCKET)
        {
            shutdown(_socket, SD_SEND);
#ifdef _WIN32
            closesocket(_socket);
#else
            close(_pimpl->_socket);
#endif
        }
    }

public:
    void send(const char* data, size_t length) override;
    void initSocket(const char* ipaddr, unsigned short port, unsigned char ttl, const char* iface_addr);

public:
    SOCKET _socket{ INVALID_SOCKET };
    bool _run{ true };
    sockaddr_in _recvAddr{};
    uint32_t _address{};
    uint16_t _port{};
};

KlvTextWriter::KlvTextWriter(const char* ipaddr, uint16_t port, unsigned char ttl, const char* iface_addr)
{
    if (strcmp(ipaddr, "-") == 0) // write the data to console
    {
        _pimpl = std::make_unique<ConsoleImpl>();
#ifdef _WIN32
        _setmode(_fileno(stdout), _O_BINARY);
#endif
    }
    else // send the data over UDP
    {
        _pimpl = std::make_unique<UdpImpl>(ipaddr, port, ttl, iface_addr);
    }
}

KlvTextWriter::~KlvTextWriter(void)
{

}

void UdpImpl::initSocket(const char* ipaddr, uint16_t port, unsigned char ttl, const char* iface_addr)
{
    char szErr[BUFSIZ]{};
    IN_ADDR inaddr;
#ifdef _WIN32
    WORD winsock_version, err;
    WSADATA winsock_data;
    winsock_version = MAKEWORD(2, 2);

    err = WSAStartup(winsock_version, &winsock_data);
    if (err != 0)
    {
        std::exception exp("Failed to initialize WinSock");
        throw exp;
    }
#endif

    //----------------------
    // Create a SOCKET for connecting to server
    _socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (_socket == INVALID_SOCKET) {
#ifdef _WIN32
        WSACleanup();
        sprintf(szErr, "Error at socket(): %d", WSAGetLastError());
#else
        sprintf(szErr, "Error at socket(): %d", errno);
#endif
        std::runtime_error exp(szErr);
        throw exp;
    }

    // Set time to live
    unsigned char optVal = ttl;
    int optLen = sizeof(optVal);

    if (setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&optVal, optLen) == SOCKET_ERROR)
    {
#ifdef _WIN32
        WSACleanup();
        sprintf(szErr, "Error setting socket option IP_MULTICAST_TTL: %d", WSAGetLastError());
#else
        sprintf(szErr, "Error setting socket option IP_MULICAST_TTL: %d", errno);
#endif
        std::runtime_error exp(szErr);
        throw exp;
    }

    if (strlen(iface_addr) > 0)
    {
        inet_pton(AF_INET, iface_addr, (PVOID)&inaddr);
        if (setsockopt(_socket, IPPROTO_IP, IP_MULTICAST_IF, (char*)&inaddr.s_addr, sizeof(inaddr.s_addr)) == SOCKET_ERROR)
        {
#ifdef _WIN32
            WSACleanup();
            sprintf(szErr, "KlvTextWriter Error setting socket option IP_MULTICAST_IF: %d", WSAGetLastError());
#else
            sprintf(szErr, "KlvTextWriter Error setting socket option IP_MULTICAST_IF: %d", errno);
#endif
            std::runtime_error exp(szErr);
            throw exp;
        }
    }

    if (inet_pton(AF_INET, ipaddr, (PVOID)&inaddr) != 1)
    {
#ifdef _WIN32
        WSACleanup();
        sprintf_s(szErr, "Error when calling inet_pton: %d", WSAGetLastError());
#else
        sprintf(szErr, "Error when calling inet_pton: %d", errno);
#endif
        std::runtime_error exp(szErr);
        throw exp;
    }
    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being send to
    _recvAddr.sin_family = AF_INET;
    _recvAddr.sin_addr.s_addr = inaddr.s_addr;
    _recvAddr.sin_port = htons(port);

    _address = _recvAddr.sin_addr.s_addr;
}

void KlvTextWriter::send(const std::string& data)
{
    _pimpl->send(data.data(), data.length());
}

void UdpImpl::send(const char* data, size_t length)
{
    // Write the data to the standard console out
    if (_socket == INVALID_SOCKET)
    {
        return;
    }

    int bufsiz = BUFLEN;
    int nLeft = length;
    size_t i = 0;
    while (i < length)
    {
        if (nLeft < bufsiz)
        {
            bufsiz = nLeft;
        }

        const int status = sendto(_socket,
            data + i,
            bufsiz,
            0,
            (SOCKADDR*)&_recvAddr,
            sizeof(_recvAddr));

        if (status == SOCKET_ERROR)
        {
#ifdef _WIN32
            const int errCode = WSAGetLastError();
#else
            const int errCode = errno;
#endif
        }
        i += BUFLEN;
        nLeft -= BUFLEN;
    }
}

void ConsoleImpl::send(const char* data, size_t length)
{
    size_t w = 0;
    for (size_t nleft = length; nleft > 0;)
    {
        w = fwrite(data, 1, nleft, stdout);
        if (w == 0)
        {
            std::cerr << "error: unable to write " << nleft << " bytes to stdout" << std::endl;
        }
        nleft -= w;
        fflush(stdout);
    }
}
