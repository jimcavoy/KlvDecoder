#include <array>
#include <fstream>
#include <iostream>
#include <MiDemux/MiDemux.h>
#include <fcntl.h>

#include "CmdLineParser.h"

#ifdef _WIN32
#include <io.h>
#include <Windows.h>
#define fprintf fprintf_s
#endif

bool gRun = true;

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
        }
        else
        {
            gRun = false;
        }

        if (demux.reads() > args.reads() && args.reads() > 0)
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
