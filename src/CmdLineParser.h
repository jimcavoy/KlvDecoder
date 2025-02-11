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
        TEXT,
        UNKNOWN
    };

public:
    CmdLineParser();

    int parse(int argc, char** argv);

    int reads() const;
    float frequency() const;
    FORMAT format() const;
    std::string source() const;

private:
    class Impl;
    std::shared_ptr<Impl> _pimpl;
};

