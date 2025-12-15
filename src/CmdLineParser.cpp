#include "CmdLineParser.h"

#include <iostream>
#include <string.h>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

class CmdLineParser::Impl
{
public:
    Impl() {}
    ~Impl() {}

    int _reads{ 0 };
    float _frequency{ 1 };
    std::string _format{ "json" };
    std::string _source{ "-" };
    std::string _outputUrl{ "-" };
#ifdef _WIN32
    std::string _klvdbFilepath{ "file:///C:/PROGRA~1/klvp/share/klv.s3db" };
#else
    std::string _klvdbFilepath{ "file:///usr/local/share/klv.s3db" };
#endif
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
        po::positional_options_description pos_desc;
        pos_desc.add("source", 1);

        desc.add_options()
            ("help,?", "Produce help message.")
            ("source", po::value<string>(&_pimpl->_source), "Source Motion Imagery stream or file. (default: - )")
            ("reads,r", po::value<int>(&_pimpl->_reads), "Number of KLV reads. Zero means continuous reads. (default: 0")
            ("freqs,f", po::value<float>(&_pimpl->_frequency), "Frequency (Hz) to output the text representation. (default: 1)")
            ("format,F", po::value<std::string>(&_pimpl->_format), "Output text format [info|json|xml]. (default: json).")
            ("outputUrl,o", po::value<std::string>(&_pimpl->_outputUrl), "Stream the text representation over UDP. If the option is missing, output to console. (default: - )")
            ("klvdbFilepath,k", po::value<std::string>(&_pimpl->_klvdbFilepath), "The filepath to klv.s3db file.")
            ;

        po::command_line_parser parser{ argc, argv };
        parser.options(desc).positional(pos_desc);
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
    else if (format == "info")
    {
        ret = CmdLineParser::FORMAT::INFO;
    }

    return ret;
}

std::string CmdLineParser::source() const
{
    return _pimpl->_source;
}

std::string CmdLineParser::outputUrl() const
{
    return _pimpl->_outputUrl;
}

std::string CmdLineParser::klvdbFilepath() const
{
    return _pimpl->_klvdbFilepath;
}
