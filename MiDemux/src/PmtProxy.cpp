#include "PmtProxy.h"

#include <string.h>

#ifdef WIN32
#define strncpy_s strncpy
#endif

namespace
{
    char TAG_HDMV[] = { (char)0x48, (char)0x44, (char)0x4D, (char)0x56, (char)0xFF, (char)0x1B, (char)0x44, (char)0x3F, 0 };
    char TAG_HDPR[] = { (char)0x48, (char)0x44, (char)0x50, (char)0x52, (char)0xFF, (char)0x1B, (char)0x67, (char)0x3F, 0 };
}

void PmtProxy::update(const lcss::ProgramMapTable& pmt)
{
    if (_version == pmt.version_number())
    {
        return;
    }

    _version = pmt.version_number();
    for (const auto& pe : pmt)
    {
        switch (pe.stream_type())
        {
        case 0x06:
        case 0x15:
        {
            char value[255]{};
            char format_identifier[5]{};

            for (auto& desc : pe)
            {
                // registration_descriptor
                if (desc.tag() == 0x05)
                {
                    desc.value((uint8_t*)value);
                    strncpy(format_identifier, value, 4);
                    break;
                }
                // metadata_descriptor
                else if (desc.tag() == 0x26)
                {
                    desc.value((uint8_t*)value);
                    strncpy(format_identifier, value + 3, 4);
                    break;
                }
            }

            if (strcmp(format_identifier, "KLVA") == 0)
            {
                _pid2type.insert({ pe.pid(), STREAM_TYPE::KLVA });
            }
            else if (strcmp(format_identifier, "$EXI") == 0)
            {
                _pid2type.insert({ pe.pid(), STREAM_TYPE::$EXI });
            }
            else if (strcmp(format_identifier, "$XML") == 0)
            {
                _pid2type.insert({ pe.pid(), STREAM_TYPE::$XML });
            }
        }
        break;
        case 0x1B:
        {
            char value[255]{};
            for (const auto& desc : pe)
            {
                // registration_descriptor
                if (desc.tag() == 0x05)
                {
                    desc.value((uint8_t*)value);
                    break;
                }
            }

            if (strcmp(value, TAG_HDMV) == 0 || strcmp(value, TAG_HDPR) == 0)
            {
                _pid2type.insert({ pe.pid(), STREAM_TYPE::HDMV });
            }
            else
            {
                _pid2type.insert({ pe.pid(), STREAM_TYPE::H264 });
            }
        }
        break;
        case 0x02:
        {
            _pid2type.insert({ pe.pid(), STREAM_TYPE::VIDEO });
        }
        break;
        case 0x03:
        case 0x04:
        {
            _pid2type.insert({ pe.pid(), STREAM_TYPE::AUDIO });
        }
        break;
        case 0x24:
        {
            char value[255]{};
            char format_identifier[5]{};

            for (const auto& desc : pe)
            {
                // registration_descriptor
                if (desc.tag() == 0x05)
                {
                    desc.value((uint8_t*)value);
                    strncpy(format_identifier, value, 4);
                    break;
                }
            }

            if (strcmp(format_identifier, "HEVC") == 0)
            {
                _pid2type.insert({ pe.pid(), STREAM_TYPE::H265 });
            }
        }
        break;
        }
    }
}

PmtProxy::STREAM_TYPE PmtProxy::packetType(unsigned short pid)
{
    STREAM_TYPE type = STREAM_TYPE::UNKNOWN;

    map_type::iterator it = _pid2type.find(pid);
    if (it != _pid2type.end())
    {
        type = it->second;
    }

    return type;
}