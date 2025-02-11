#include <iostream>

#include <MiDemux/MiDemux.h>

#include "CmdLineParser.h"

#ifdef _WIN32
#include <Windows.h>
#define fprintf fprintf_s
#endif

/////////////////////////////////////////////////////////////////////////////
// Forward Declarations
#ifdef _WIN32
BOOL CtrlHandler(DWORD fdwCtrlType);
#endif
void Banner();

int main(int argc, char* argv[])
{
    CmdLineParser args;
    int retCode = 0;

    Banner();

    if ((retCode = args.parse(argc, argv)) != 0)
    {
        return retCode;
    }

    MiDemux demux(args.reads(), args.frequency());



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
        std::cerr << "Closing down, please wait" << std::endl;
        Sleep(1000);
        return TRUE;
    default:
        return FALSE;
    }
}
#endif
