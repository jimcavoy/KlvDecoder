#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <MiDemux/MiDemux.h>
#include <fcntl.h>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/url.hpp>

#include "CmdLineParser.h"
#include "UdpSender.h"

#ifdef _WIN32
#include <io.h>
#include <Windows.h>
#define fprintf fprintf_s
#endif

bool gRun = true;

class UrlParser
{
public:
    UrlParser() {}

    void parse(std::string strurl)
    {
        using namespace boost;
        namespace url = boost::urls; 

        if (strurl.empty())
            return;

        if (strurl == "-")
            return;

        system::result<url::url_view> res = url::parse_uri(strurl);

        url::url_view urlView = res.value();
        
        if (!res)
            return;

        schema = urlView.scheme();
        ipaddress = urlView.host();

        if (urlView.has_port())
        {
            port = urlView.port_number();
        }

        if (urlView.has_query())
        {
            url::params_encoded_view params_ref = urlView.encoded_params();

            for (const auto& v : params_ref)
            {
                url::decode_view dk(v.key);
                url::decode_view dv(v.value);

                if (dk == "ttl")
                {
                    std::string strTTL = std::string(dv.begin(), dv.end());
                    ttl = (uint8_t)std::stoi(strTTL);
                }
                else if (dk == "localaddr")
                {
                    std::string strVal(dv.begin(), dv.end());
                    ifaceaddress = strVal;
                }
            }
        }
    }

public:
    std::string schema;
    std::string ipaddress{ "-" };
    uint16_t port{ 0 };
    uint8_t ttl{ 255 };
    std::string ifaceaddress;
};

/////////////////////////////////////////////////////////////////////////////
// Forward Declarations
#ifdef _WIN32
BOOL CtrlHandler(DWORD fdwCtrlType);
#endif
void Banner();

int main(int argc, char* argv[])
{
    int retCode = 0;
    CmdLineParser args;
    std::shared_ptr<std::istream> ifile;
    std::array<uint8_t, 9212> buffer;

    Banner();

    if ((retCode = args.parse(argc, argv)) != 0)
    {
        return retCode;
    }

    MiDemux demux(args.frequency());
    UrlParser urlp;
    urlp.parse(args.outputUrl());
    UdpSender writer(urlp.ipaddress.c_str(), urlp.port, urlp.ttl, urlp.ifaceaddress.c_str());

    if (args.source() == "-")
    {
#ifdef _WIN32
        _setmode(_fileno(stdin), _O_BINARY);
#endif
        ifile.reset(&std::cin, [](...) {});
    }
    else
    {
        std::ifstream* tsfile = new std::ifstream(args.source(), std::ios::binary);
        if (!tsfile->is_open())
        {
            std::cerr << "ERROR: Failed to open file, " << args.source() << std::endl;
            return -1;
        }
        ifile.reset(tsfile);
    }

    while (gRun)
    {
        if (ifile->good())
        {
            ifile->read((char*)buffer.data(), buffer.size());
            const std::streamsize len = ifile->gcount();

            demux.read(buffer.data(), len);

            demux.setKlvSetCallback([&](const pt::ptree& klvset)
                {
                    std::stringstream output;
                    switch (args.format())
                    {
                    case CmdLineParser::FORMAT::JSON:
                        pt::write_json(output, klvset);
                        break;
                    case CmdLineParser::FORMAT::XML:
                        pt::write_xml(output, klvset, pt::xml_parser::trim_whitespace);
                        break;
                    case CmdLineParser::FORMAT::INFO:
                        pt::write_info(output, klvset);
                        break;
                    default:
                        std::cerr << "Unknown Format" << std::endl;
                    }

                    if (!output.str().empty())
                    {
                        writer.send(output.str().c_str(), output.str().length());
                    }
                });
        }
        else
        {
            gRun = false;
        }

        if (demux.reads() >= args.reads() && args.reads() > 0)
        {
            gRun = false;
        }
    }

    return retCode;
}

void Banner()
{
    std::cerr << "KlvDecoder v1.0.0" << std::endl;
    std::cerr << "Copyright (c) 2025 ThetaStream Consulting, jimcavoy@thetastream.com" << std::endl;
}

#ifdef _WIN32
/// <summary>
/// Controls the handler.
/// </summary>
/// <param name="fdwCtrlType">Type of the FDW control.</param>
/// <returns></returns>
BOOL CtrlHandler(DWORD fdwCtrlType)
{
    switch (fdwCtrlType)
    {
        // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    case CTRL_LOGOFF_EVENT:
        gRun = false;
        std::cerr << "Closing down, please wait" << std::endl;
        Sleep(1000);
        return TRUE;
    default:
        return FALSE;
    }
}
#endif
