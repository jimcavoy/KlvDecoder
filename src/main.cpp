#include <array>
#include <fstream>
#include <iostream>
#include <sstream>
#include <MiDemux/MiDemux.h>
#include <fcntl.h>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/url.hpp>

#include "CmdLineParser.h"
#include "KlvTextWriter.h"

#ifdef _WIN32
#include <io.h>
#include <Windows.h>
#define fprintf fprintf_s
#endif

bool gRun = true;

/// <summary>
/// A helper class to parse a string representing a URL
/// </summary>
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

namespace boost {
    namespace property_tree {
        namespace json_parser
        {

            template<>
            void write_json_helper(std::basic_ostream<typename pt::basic_ptree<std::string, std::string>::key_type::value_type>& stream,
                const pt::basic_ptree<std::string, std::string>& pt,
                int indent, bool pretty)
            {

                typedef typename pt::basic_ptree<std::string, std::string>::key_type::value_type Ch;
                typedef typename std::basic_string<Ch> Str;

                // Value or object or array
                if (indent > 0 && pt.empty())
                {
                    // Write value
                    Str data = create_escapes(pt.template get_value<Str>());
                    try
                    {
                        float num = boost::lexical_cast<float>(data);
                        stream << num;
                    }
                    catch ( const boost::bad_lexical_cast& e)
                    {
                        stream << Ch('"') << data << Ch('"');
                    }

                }
                else if (indent > 0 && pt.count(Str()) == pt.size())
                {
                    // Write array
                    stream << Ch('[');
                    if (pretty) stream << Ch('\n');
                    typename pt::basic_ptree<std::string, std::string>::const_iterator it = pt.begin();
                    for (; it != pt.end(); ++it)
                    {
                        if (pretty) stream << Str(4 * (indent + 1), Ch(' '));
                        write_json_helper(stream, it->second, indent + 1, pretty);
                        if (boost::next(it) != pt.end())
                            stream << Ch(',');
                        if (pretty) stream << Ch('\n');
                    }
                    if (pretty) stream << Str(4 * indent, Ch(' '));
                    stream << Ch(']');

                }
                else
                {
                    // Write object
                    stream << Ch('{');
                    if (pretty) stream << Ch('\n');
                    typename pt::basic_ptree<std::string, std::string>::const_iterator it = pt.begin();
                    for (; it != pt.end(); ++it)
                    {
                        if (pretty) stream << Str(4 * (indent + 1), Ch(' '));
                        stream << Ch('"') << create_escapes(it->first) << Ch('"') << Ch(':');
                        if (pretty) stream << Ch(' ');
                        write_json_helper(stream, it->second, indent + 1, pretty);
                        if (boost::next(it) != pt.end())
                            stream << Ch(',');
                        if (pretty) stream << Ch('\n');
                    }
                    if (pretty) stream << Str(4 * indent, Ch(' '));
                    stream << Ch('}');
                }

            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
// Forward Declarations
#ifdef _WIN32
BOOL CtrlHandler(DWORD fdwCtrlType);
#endif
void Banner();

/// <summary>
/// Main function
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
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

    try
    {
        MiDemux demux(args.frequency(), args.klvdbFilepath());
        UrlParser urlp;
        urlp.parse(args.outputUrl());
        KlvTextWriter writer(urlp.ipaddress.c_str(), urlp.port, urlp.ttl, urlp.ifaceaddress.c_str());

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
                            pt::write_json(output, klvset, false);
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
                            writer.send(output.str());
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
    }
    catch (const std::exception& ex)
    {
        std::cerr << "ERROR: " << ex.what() << std::endl;
        return -1;
    }
    catch (...)
    {
        std::cerr << "ERROR: Unknown exception caught." << std::endl;
        return -1;
    }

    return retCode;
}

void Banner()
{
    std::cerr << "KlvDecoder v1.2.1" << std::endl;
    std::cerr << "Copyright (c) 2026 ThetaStream Consulting, jimcavoy@thetastream.com" << std::endl;
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
