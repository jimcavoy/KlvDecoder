#include "CmdLineParser.h"

#include <iostream>
#include <string.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

namespace
{
    std::string urlEncode(const char* url)
    {
        std::string ret;
        size_t len = strlen(url);

        for (int i = 0; i < len; i++)
        {
            if (isspace(url[i]) != 0)
                ret += "%20";
            else
                ret.push_back(url[i]);
        }
        return ret;
    }
}

class CmdLineParser::Impl
{
public:
    Impl() {}
    ~Impl() {}

    int _reads{ 0 };
    float _frequency{ 1 };
    std::string _format;
};

CmdLineParser::CmdLineParser()
{
    _pimpl = std::make_shared<CmdLineParser::Impl>();
}


int CmdLineParser::parse(int argc, char** argv)
{
    using namespace std;
    using namespace boost;
    namespace po = boost::program_options;

    int ret = 0;

    try
    {
        po::options_description desc("Allowed options.");

        desc.add_options()
            ("help,?", "Produce help message.")
            ("reads,r", po::value<int>(&_pimpl->_reads), "Number of KLV reads. Zero means continuous reads.")
            ("freqs,f", po::value<float>(&_pimpl->_frequency), "Frequency (Hz) to output the text representation.")
            ("format,F", po::value<std::string>(&_pimpl->_format), "Output text format. (default: json).")
            ;

        po::command_line_parser parser{ argc, argv };
        parser.options(desc);
        po::parsed_options poptions = parser.run();

        po::variables_map vm;
        po::store(poptions, vm);
        po::notify(vm);

        if (vm.count("help"))
        {
            cerr << desc << endl;
            return -1;
        }
    }
    catch (std::exception& ex)
    {
        cerr << ex.what() << endl;
        return -1;
    }

    return ret;
}


int CmdLineParser::reads() const
{
    return _pimpl->_reads;
}

float CmdLineParser::frequency() const
{
    return _pimpl->_frequency;
}

CmdLineParser::FORMAT CmdLineParser::format() const
{
    std::string format = boost::algorithm::to_lower_copy(_pimpl->_format);
    CmdLineParser::FORMAT ret{ CmdLineParser::FORMAT::UNKNOWN };

    if (format == "json")
    {
        ret = CmdLineParser::FORMAT::JSON;
    }
    else if (format == "xml")
    {
        ret = CmdLineParser::FORMAT::XML;
    }
    else if (format == "text")
    {
        ret = CmdLineParser::FORMAT::TEXT;
    }

    return ret;
}
