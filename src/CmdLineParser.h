#pragma once
#include <memory>
#include <string>


class CmdLineParser
{
public:
    enum class FORMAT
    {
        JSON,
        XML,
        INFO,
        UNKNOWN
    };

public:
    CmdLineParser();

    int parse(int argc, char** argv);

    int reads() const;
    float frequency() const;
    FORMAT format() const;
    std::string source() const;
    std::string outputUrl() const;

private:
    class Impl;
    std::shared_ptr<Impl> _pimpl;
};

