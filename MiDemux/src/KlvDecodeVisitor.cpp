#include "KlvDecodeVisitor.h"

#include "KlvSecuritySetParserImpl.h"
#include "KlvSecuritySetVisitor.h"

#include <klvp/decode.h>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <gsl/gsl>
#include <regex>

#ifdef WIN32
#include <WinSock2.h>
#define sprintf_s sprintf
#else
#include <netinet/in.h>
#endif

// klvlex generated on Mon Aug 05 14:54:11 2013

using namespace std;

namespace
{
    LDSEntry fetchKlvDefinition(LDSDatabase& db, uint8_t key)
    {
        LDSEntry entry;
        entry.key = key;

        if (db.is_open())
        {
            db.fetch(&entry);
        }
        return entry;
    }


    string printRaw(const lcss::KLVElementImpl& klv)
    {
        uint16_t len = (uint16_t)klv.length() + 2;
        uint8_t* sodb = new uint8_t[len]{};
        std::stringstream ret;

        klv.serialize(sodb, len);

        for (uint16_t i = 0; i < len; i++)
        {
            if (ret.str().length() > 0)
                ret << " ";
            ret << setw(2) << setfill('0') << hex << (int)sodb[i];
        }

        delete[] sodb;
        return ret.str();
    }

    std::string space2underscore(const std::string value)
    {
        const auto target = std::regex{ " " };
        const auto replacement = std::string{ "_" };

        return std::regex_replace(value, target, replacement);
    }


    pt::ptree::value_type printString(LDSDatabase& db, const lcss::KLVElementImpl& klv)
    {
        int len = klv.length();
        uint8_t* buf = new uint8_t[len]{};
        klv.value(buf);
        pt::ptree obj;

        std::stringstream val;

        for (int i = 0; i < len; i++)
        {
            if (isprint(buf[i]))
                val << (char)buf[i];
            else
                val << hex << buf[i] << dec;
        }

        LDSEntry lds = fetchKlvDefinition(db, klv.key());
        /*obj["name"] = space2underscore(lds.name);
        obj["encoded"] = printRaw(klv);
        obj["units"] = lds.units;
        obj["format"] = lds.format;
        obj["description"] = lds.description;
        obj["key"] = klv.key();
        obj["length"] = klv.length();
        obj["value"] = val.str();*/

        obj.put<int>("key", klv.key());
        obj.put("value", val.str());

        pt::ptree::value_type vt(space2underscore(lds.name), obj);

        delete[] buf;

        return vt;
    }

    pt::ptree::value_type printReal(LDSDatabase& db, const lcss::KLVElementImpl& klv, double val)
    {
        pt::ptree obj;

        LDSEntry lds = fetchKlvDefinition(db, klv.key());

        /*obj["name"] = lds.name;
        obj["encoded"] = printRaw(klv);
        obj["units"] = lds.units;
        obj["format"] = lds.format;
        obj["description"] = lds.description;
        obj["key"] = klv.key();
        obj["length"] = klv.length();
        obj["value"] = val;*/
        obj.put<int>("key", klv.key());
        obj.put<float>("value", val);
        pt::ptree::value_type vt(space2underscore(lds.name), obj);

        return vt;
    }

    pt::ptree::value_type printInt(LDSDatabase& db, const lcss::KLVElementImpl& klv, int val)
    {
        pt::ptree obj;
        LDSEntry lds = fetchKlvDefinition(db, klv.key());

        /*obj["name"] = lds.name;
        obj["encoded"] = printRaw(klv);
        obj["units"] = lds.units;
        obj["format"] = lds.format;
        obj["description"] = lds.description;
        obj["key"] = klv.key();
        obj["length"] = klv.length();
        obj["value"] = val;*/
        obj.put<int>("key", klv.key());
        obj.put<int>("value", val);
        pt::ptree::value_type vt(space2underscore(lds.name), obj);

        return vt;
    }

    pt::ptree::value_type printToBeImplemented(LDSDatabase& db, const lcss::KLVElementImpl& klv)
    {
        pt::ptree obj;

        LDSEntry lds = fetchKlvDefinition(db, klv.key());

        /*obj["name"] = space2underscore(lds.name);
        obj["encoded"] = printRaw(klv);
        obj["units"] = lds.units;
        obj["format"] = lds.format;
        obj["description"] = lds.description;
        obj["key"] = klv.key();
        obj["length"] = klv.length();
        obj["value"] = "To Be Implemented";*/
        obj.put<int>("key", klv.key());
        obj.put("value", "To be Implemented");
        pt::ptree::value_type vt(space2underscore(lds.name), obj);

        return vt;
    }

    pt::ptree::value_type printTimestamp(LDSDatabase& db, const lcss::KLVElementImpl& klv, int64_t tmValue)
    {
#ifdef WIN32
        struct tm st;
#else
        struct tm* st;
#endif
        char timebuf[512]{};
        char szTime[256]{};
        pt::ptree obj;

        int usec = tmValue % 1000000; // microseconds
        time_t time = (time_t)tmValue / 1e6; // seconds

#ifdef WIN32
        gmtime_s(&st, &time);
        strftime(szTime, 256, "%A, %d-%b-%y %T", &st);
        _snprintf_s(timebuf, 512, "%s.%06ld UTC", szTime, usec);
#else
        st = gmtime(&time);
        strftime(szTime, 256, "%A, %d-%b-%y %T", st);
        sprintf(timebuf, "%s.%06d UTC", szTime, usec);
#endif

        LDSEntry lds = fetchKlvDefinition(db, klv.key());
        /*obj["name"] = space2underscore(lds.name);
        obj["encoded"] = printRaw(klv);
        obj["units"] = lds.units;
        obj["format"] = lds.format;
        obj["description"] = lds.description;
        obj["key"] = klv.key();
        obj["length"] = klv.length();
        obj["value"] = timebuf;*/
        obj.put<int>("key", klv.key());
        obj.put("value", std::string(timebuf));

        pt::ptree::value_type vt(space2underscore(lds.name), obj);

        return vt;
    }

    int encodeBERLength(uint8_t* buffer, int len)
    {
        int numOfBytesRead = 0;
        if (len <= 127)
        {
            numOfBytesRead = 1;
            uint8_t c = (uint8_t)len;
            *buffer = len;
        }
        else if (len > 127 && len < 256)
        {
            numOfBytesRead = 2;
            *buffer++ = 0x81;
            uint8_t sz = (uint8_t)len;
            *buffer = sz;
        }
        else if (len >= 256 && len < 65536)
        {
            numOfBytesRead = 3;
            *buffer = 0x82;
            uint16_t sz = htons(len);
            memcpy(buffer + 1, (void*)&sz, 2);
        }
        return numOfBytesRead;
    }
}

KlvDecodeVisitor::KlvDecodeVisitor(pt::ptree& klvSet, const char* databaseUri)
    : _klvSet(klvSet)
{
    _ldsDb.connect(databaseUri);
}

void KlvDecodeVisitor::Visit(lcss::KLVUnknown& klv)
{
    const int len = klv.length();
    uint16_t key = (uint16_t)klv.key();
    uint8_t* val = new uint8_t[len]{};
    klv.value(val);
    std::stringstream strval;
    pt::ptree obj;

    for (int i = 0; i < len; i++)
    {
        strval << "0x" << setw(2) << setfill('0') << hex << (int)val[i] << " ";
    }

   /* obj["name"] = "UNKNOWN";
    obj["encoded"] = printRaw(klv);
    obj["key"] = klv.key();
    obj["length"] = klv.length();
    obj["value"] = strval.str();*/

    obj.put<int>("key", klv.key());
    obj.put("value", strval.str());
    pt::ptree::value_type vt("UNKNOWN", obj);

    _klvSet.push_back(vt);
    delete[] val;
}

void KlvDecodeVisitor::Visit(lcss::KLVParseError& klv)
{
    const int len = klv.length();
    uint16_t key = (uint16_t)klv.key();
    uint8_t* val = new uint8_t[len]{};
    klv.value(val);
    pt::ptree obj;
    std::stringstream strval;

    for (int i = 0; i < len; i++)
    {
        strval << "0x" << setw(2) << setfill('0') << hex << (int)val[i] << " ";
    }

    /*obj["name"] = "PARSE ERROR";
    obj["encoded"] = printRaw(klv);
    obj["key"] = klv.key();
    obj["length"] = klv.length();
    obj["what"] = klv.what_;
    obj["value"] = strval.str();*/

    obj.put<int>("key", klv.key());
    obj.put("value", strval.str());
    pt::ptree::value_type vt("PARSE ERROR", obj);

    _klvSet.push_back(vt);
    delete[] val;
}

void KlvDecodeVisitor::Visit(lcss::KLVChecksum& klv)
{
    uint8_t val[2];
    memset(val, 0, 2);
    klv.value(val);
    char crc[16];
    sprintf(crc, "%#4.2x %#4.2x", val[0], val[1]);
    pt::ptree obj;

    LDSEntry lds = fetchKlvDefinition(_ldsDb, klv.key());
    /*obj["name"] = space2underscore(lds.name);
    obj["encoded"] = printRaw(klv);
    obj["units"] = lds.units;
    obj["format"] = lds.format;
    obj["description"] = lds.description;
    obj["key"] = klv.key();
    obj["length"] = klv.length();
    obj["value"] = crc;*/
    obj.put<int>("key", klv.key());
    obj.put("value", std::string(crc));
    pt::ptree::value_type vt(space2underscore(lds.name), obj);

    _klvSet.push_back(vt);
}

void KlvDecodeVisitor::Visit(lcss::KLVUNIXTimeStamp& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printTimestamp(_ldsDb, klv, _decoder.tmValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVMissionID& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformTailNumber& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformHeadingAngle& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformPitchAngle& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformRollAngle& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformTrueAirspeed& klv)
{
    uint8_t val;
    klv.value(&val);
    uint16_t nValue = (uint16_t)val;
    _klvSet.push_back(printInt(_ldsDb, klv, nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformIndicatedAirspeed& klv)
{
    uint8_t val;
    klv.value(&val);
    uint16_t nValue = (uint16_t)val;
    _klvSet.push_back(printInt(_ldsDb, klv, nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformDesignation& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVImageSourceSensor& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVImageCoordinateSystem& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorLatitude& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorLongitude& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorTrueAltitude& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorHorizontalFieldofView& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorVerticalFieldofView& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorRelativeAzimuthAngle& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorRelativeElevationAngle& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorRelativeRollAngle& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSlantRange& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVTargetWidth& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVFrameCenterLatitude& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVFrameCenterLongitude& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVFrameCenterElevation& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOffsetCornerLatitudePoint1& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOffsetCornerLongitudePoint1& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOffsetCornerLatitudePoint2& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOffsetCornerLongitudePoint2& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOffsetCornerLatitudePoint3& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOffsetCornerLongitudePoint3& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOffsetCornerLatitudePoint4& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOffsetCornerLongitudePoint4& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVIcingDetected& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printInt(_ldsDb, klv, _decoder.nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVWindDirection& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVWindSpeed& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVStaticPressure& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVDensityAltitude& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOutsideAirTemperature& klv)
{
    uint8_t val;
    klv.value(&val);
    int nValue = (int)val;
    _klvSet.push_back(printInt(_ldsDb, klv, nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVTargetLocationLatitude& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVTargetLocationLongitude& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVTargetLocationElevation& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVTargetTrackGateWidth& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVTargetTrackGateHeight& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVTargetErrorEstimateCE90& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVTargetErrorEstimateLE90& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVGenericFlagData01& klv)
{
    pt::ptree obj;
    /*obj["name"] = "Generic Flag Data 01";
    obj["key"] = klv.key();
    obj["length"] = klv.length();
    obj["value"] = *klv.value();*/
    obj.put<int>("key", klv.key());
    obj.put("value", *klv.value());
    pt::ptree::value_type vt("Generic_Flag_Data_01", obj);
    _klvSet.push_back(vt);
}

void KlvDecodeVisitor::Visit(lcss::KLVSecurityLocalMetadataSet& klv)
{
    uint8_t ss[0x0FFF]{};
    uint8_t value[0x0FFF]{};
    klv.value(value);
    ss[0] = 0x30;

    int numOfBytesEncoded = encodeBERLength(ss + 1, klv.length());
    numOfBytesEncoded++; // add one for the key
    memcpy(ss + numOfBytesEncoded, value, klv.length());

    pt::ptree array;
    KlvSecuritySetVisitor pv(array, _ldsDb);
    KlvSecuritySetParserImpl ssp(pv);
    const int len = klv.length() + numOfBytesEncoded;
    ssp.parse({ ss, gsl::narrow_cast<std::size_t>(len) });

    pt::ptree::value_type vt("Security_Set", array);

    _klvSet.push_back(vt);
}

void KlvDecodeVisitor::Visit(lcss::KLVDifferentialPressure& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformAngleofAttack& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformVerticalSpeed& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformSideslipAngle& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVAirfieldBarometicPressure& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVAirfieldElevation& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVRelativeHumidity& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformGroundSpeed& klv)
{
    uint8_t val;
    klv.value(&val);
    uint16_t nValue = (uint16_t)val;
    _klvSet.push_back(printInt(_ldsDb, klv, nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVGroundRange& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformFuelRemaining& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformCallSign& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVWeaponLoad& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printInt(_ldsDb, klv, _decoder.nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVWeaponFired& klv)
{
    _klvSet.push_back(printInt(_ldsDb, klv, *klv.value()));
}

void KlvDecodeVisitor::Visit(lcss::KLVLaserPRFCode& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printInt(_ldsDb, klv, _decoder.nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorFieldofViewName& klv)
{
    _klvSet.push_back(printInt(_ldsDb, klv, *klv.value()));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformMagneticHeading& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVUASLDSVersionNumber& klv)
{
    uint8_t val;
    klv.value(&val);
    int nValue = (int)val;
    _klvSet.push_back(printInt(_ldsDb, klv, nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVTargetLocationCovarianceMatrix& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVAlternatePlatformLatitude& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVAlternatePlatformLongitude& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVAlternatePlatformAltitude& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVAlternatePlatformName& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVAlternatePlatformHeading& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVEventStartTimeUTC& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printTimestamp(_ldsDb, klv, _decoder.tmValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVRVTLocalDataSet& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVVMTILocalDataSet& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorEllipsoidHeight& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVAlternatePlatformEllipsoidHeight& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOperationalMode& klv)
{
    _klvSet.push_back(printInt(_ldsDb, klv, *klv.value()));
}

void KlvDecodeVisitor::Visit(lcss::KLVFrameCenterHeightAboveEllipsoid& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorNorthVelocity& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorEastVelocity& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVImageHorizonPixelPack& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVCornerLatitudePoint1Full& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVCornerLongitudePoint1Full& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVCornerLatitudePoint2Full& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVCornerLongitudePoint2Full& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVCornerLatitudePoint3Full& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVCornerLongitudePoint3Full& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVCornerLatitudePoint4Full& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVCornerLongitudePoint4Full& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformPitchAngleFull& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformRollAngleFull& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}


void KlvDecodeVisitor::Visit(lcss::KLVPlatformAngleofAttackFull& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformSideslipAngleFull& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVMIISCoreIdentifier& klv)
{
    std::string strKey = std::to_string(klv.key());
    uint8_t value[18]{};
    klv.value(value);
    char uuid[BUFSIZ]{};

    sprintf(uuid, "%02X%02X:%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X",
        value[0], value[1], value[2], value[3], value[4], value[5], value[6], value[7],
        value[8], value[9], value[10], value[11], value[12], value[13], value[14],
        value[15], value[16], value[17]);

    pt::ptree obj;
    LDSEntry lds = fetchKlvDefinition(_ldsDb, klv.key());
     /*obj["name"] = space2underscore(lds.name);
    obj["encoded"] = printRaw(klv);
    obj["units"] = lds.units;
    obj["format"] = lds.format;
    obj["description"] = lds.description;
    obj["key"] = klv.key();
    obj["length"] = klv.length();
    obj["value"] = uuid;*/
    obj.put<int>("key", klv.key());
    obj.put("value", std::string(uuid));
    pt::ptree::value_type vt(space2underscore(lds.name), obj);

    _klvSet.push_back(vt);
}

void KlvDecodeVisitor::Visit(lcss::KLVSARMotionImageryMetadata& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVTargetWidthExtended& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVRangeImageLocalSet& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVGeoRegistrationLocalSet& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVCompositeImagingLocalSet& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVSegmentLocalSet& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVAmendLocalSet& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVSDCCFLP& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVDensityAltitudeExtended& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorEllipsoidHeightExtended& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVAlternatePlatformEllipsoidHeightExtended& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVStreamDesignator& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVOperationalBase& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVBroadcastSource& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVRangeToRecoveryLocation& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVTimeAirborne& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printInt(_ldsDb, klv, _decoder.nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPropulsionUnitSpeed& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printInt(_ldsDb, klv, _decoder.nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformCourseAngle& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVAltitudeAGL& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVRadarAltimeter& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVControlCommand& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVControlCommandVerificationList& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorAzimuthRate& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorElevationRate& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorRollRate& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOnboardMIStoragePercentFull& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVActiveWavelengthList& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVCountryCodes& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVNumberofNAVSATsinView& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printInt(_ldsDb, klv, _decoder.nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPositioningMethodSource& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printInt(_ldsDb, klv, _decoder.nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVPlatformStatus& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printInt(_ldsDb, klv, _decoder.nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorControlMode& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printInt(_ldsDb, klv, _decoder.nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVSensorFrameRatePack& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVWavelengthsList& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVTargetID& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVAirbaseLocations& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVTakeoffTime& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printTimestamp(_ldsDb, klv, _decoder.tmValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVTransmissionFrequency& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVOnboardMIStorageCapacity& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printInt(_ldsDb, klv, _decoder.nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVZoomPercentage& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printReal(_ldsDb, klv, _decoder.fValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVCommunicationsMethod& klv)
{
    _klvSet.push_back(printString(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVLeapSeconds& klv)
{
    klv.Accept(_decoder);
    _klvSet.push_back(printInt(_ldsDb, klv, _decoder.nValue));
}

void KlvDecodeVisitor::Visit(lcss::KLVCorrectionOffset& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVPayloadList& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVActivePayloads& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVWeaponsStores& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}

void KlvDecodeVisitor::Visit(lcss::KLVWaypointList& klv)
{
    _klvSet.push_back(printToBeImplemented(_ldsDb, klv));
}



void KlvDecodeVisitor::Visit(lcss::UniversalMetadataElement& klv)
{
    const int len = klv.length();
    uint8_t key[16]{};
    klv.key(key);
    char szKey[BUFSIZ]{};
    uint8_t* val = new uint8_t[len]{};
    klv.value(val);
    pt::ptree obj;
    std::stringstream strval;

    sprintf(szKey, "%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X.%02X",
        key[0], key[1], key[2], key[3], key[4], key[5], key[6], key[7],
        key[8], key[9], key[10], key[11], key[12], key[13], key[14], key[15]);
    /*obj["key"] = szKey;
    obj["length"] = len;*/

    for (int i = 0; i < len; i++)
    {
        strval << "0x" << setw(2) << setfill('0') << hex << (int)val[i] << " ";
    }
    obj.put("key", szKey);
    obj.put("value", strval.str());

    //obj["value"] = strval.str();

    pt::ptree::value_type vt("UniversalSetElement", obj);
    _klvSet.push_back(vt);
    delete[] val;
}



