#include "KlvSecuritySetVisitor.h"

#include <iostream>
#include <iomanip>
#include <string.h>
#include <sstream>
#include <regex>

#ifdef WIN32
#include <WinSock2.h>
#define sprintf_s sprintf
#else
#include <netinet/in.h>
#endif


// klvsslex generated on Tue Aug 06 10:36:49 2013
using namespace std;

namespace
{
    LDSEntry fetchKlvDefinition(LDSDatabase& db, uint8_t key)
    {
        LDSEntry entry;
        entry.key = key;

        if (db.is_open())
        {
            db.fetch_security(&entry);
        }
        return entry;
    }

    string printRaw(const lcss::KLVElementImpl& klv)
    {
        const uint16_t len = (uint16_t)klv.length() + 2;
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
        const int len = klv.length();
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
        /*obj["name"] = lds.name;
        obj["encoded"] = printRaw(klv);
        obj["units"] = lds.units;
        obj["format"] = lds.format;
        obj["key"] = klv.key();
        obj["length"] = len;
        obj["value"] = val.str();*/
        obj.put("key", klv.key());
        obj.put("value", val.str());

        delete[] buf;

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
        obj["key"] = klv.key();
        obj["length"] = klv.length();
        obj["value"] = val;*/
        obj.put("key", klv.key());
        obj.put("value", val);

        pt::ptree::value_type vt(space2underscore(lds.name), obj);

        return vt;
    }
}

KlvSecuritySetVisitor::KlvSecuritySetVisitor(pt::ptree& klvset, LDSDatabase& ldsDb)
    : _ldsDb(ldsDb)
    , _set(klvset)
{

}

void KlvSecuritySetVisitor::Visit(lcss::KLVUnknown& klv)
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
    obj.put("key", klv.key());
    obj.put("value", strval.str());

    pt::ptree::value_type vt("UNKNOWN", obj);
    _set.push_back(vt);
    delete[] val;
}

void KlvSecuritySetVisitor::Visit(lcss::KLVParseError& klv)
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
    obj.put("key", klv.key());
    obj.put("value", strval.str());
    obj.put("error_msg", klv.what_);

    pt::ptree::value_type vt("PARSER_ERRRO", obj);

    _set.push_back(vt);
    delete[] val;
}

void KlvSecuritySetVisitor::Visit(lcss::KLVObjectCountryCodingMethodVersionDate& klv)
{
    _set.push_back(printString(_ldsDb, klv));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVSecurityClassification& klv)
{
    uint8_t value;
    klv.value(&value);
    _set.push_back(printInt(_ldsDb, klv, value));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVClassifyingCountryandReleasingInstructionsCountryCodingMethod& klv)
{
    uint8_t value;
    klv.value(&value);
    _set.push_back(printInt(_ldsDb, klv, value));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVClassifyingCountry& klv)
{
    _set.push_back(printString(_ldsDb, klv));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVSecuritySCISHIinformation& klv)
{
    _set.push_back(printString(_ldsDb, klv));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVCaveats& klv)
{
    _set.push_back(printString(_ldsDb, klv));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVReleasingInstructions& klv)
{
    _set.push_back(printString(_ldsDb, klv));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVClassifiedBy& klv)
{
    _set.push_back(printString(_ldsDb, klv));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVDerivedFrom& klv)
{
    _set.push_back(printString(_ldsDb, klv));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVClassificationReason& klv)
{
    _set.push_back(printString(_ldsDb, klv));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVDeclassificationDate& klv)
{
    _set.push_back(printString(_ldsDb, klv));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVClassificationandMarkingSystem& klv)
{
    _set.push_back(printString(_ldsDb, klv));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVObjectCountryCodingMethod& klv)
{
    uint8_t value;
    klv.value(&value);
    _set.push_back(printInt(_ldsDb, klv, value));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVObjectCountryCodes& klv)
{
    const int len = klv.length();
    uint8_t* buf = new uint8_t[len]{};
    klv.value(buf);
    pt::ptree obj;
    std::stringstream val;
    for (int i = 0; i < len; i++)
    {
        if (isprint(buf[i]))
            val << (char)buf[i];
    }

    LDSEntry lds = fetchKlvDefinition(_ldsDb, klv.key());
    /*obj["name"] = lds.name;
    obj["encoded"] = printRaw(klv);
    obj["units"] = lds.units;
    obj["format"] = lds.format;
    obj["key"] = klv.key();
    obj["length"] = klv.length();
    obj["value"] = val.str();*/
    obj.put("key", klv.key());
    obj.put("value", val.str());

    pt::ptree::value_type vt(space2underscore(lds.name), obj);

    _set.push_back(vt);
    delete[] buf;
}

void KlvSecuritySetVisitor::Visit(lcss::KLVClassificationComments& klv)
{
    _set.push_back(printString(_ldsDb, klv));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVUMIDVideo& klv)
{
    /*boost::json::object obj;
    obj["name"] = "UMID";
    obj["encoded"] = printRaw(klv);
    obj["key"] = klv.key();
    obj["length"] = klv.length();
    obj["value"] = "Not Implemented";

    _set.push_back(obj);*/
}

void KlvSecuritySetVisitor::Visit(lcss::KLVUMIDAudio& klv)
{
    /*boost::json::object obj;
    obj["name"] = "UMID Audio";
    obj["encoded"] = printRaw(klv);
    obj["key"] = klv.key();
    obj["length"] = klv.length();
    obj["value"] = "Not Implemented";

    _set.push_back(obj);*/
}

void KlvSecuritySetVisitor::Visit(lcss::KLVUMIDData& klv)
{
    /*boost::json::object obj;
    obj["name"] = "UMID Data";
    obj["encoded"] = printRaw(klv);
    obj["key"] = klv.key();
    obj["length"] = klv.length();
    obj["value"] = "Not Implemented";

    _set.push_back(obj);*/
}

void KlvSecuritySetVisitor::Visit(lcss::KLVUMIDSystem& klv)
{
    /*boost::json::object obj;
    obj["name"] = "UMID System";
    obj["encoded"] = printRaw(klv);
    obj["key"] = klv.key();
    obj["length"] = klv.length();
    obj["value"] = "Not Implemented";

    _set.push_back(obj);*/
}

void KlvSecuritySetVisitor::Visit(lcss::KLVStreamID& klv)
{
    uint8_t value;
    klv.value(&value);
    _set.push_back(printInt(_ldsDb, klv, value));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVTransportStreamID& klv)
{
    uint8_t value[2];
    klv.value(value);
    uint16_t val;
    memcpy(&val, value, 2);
    uint16_t nValue = ntohs(val);
    _set.push_back(printInt(_ldsDb, klv, nValue));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVItemDesignator& klv)
{
    uint8_t value[16];
    klv.value(value);
    pt::ptree obj;
    std::stringstream val;

    for (int i = 0; i < klv.length(); i++)
    {
        if (i != 0)
            val << " ";
        char hexc[8];
        if (value[i] != 0)
            sprintf(hexc, "%#4.2x", value[i]);
        else
            sprintf(hexc, "0x00");
        val << hexc;
    }

    LDSEntry lds = fetchKlvDefinition(_ldsDb, klv.key());
    /*obj["name"] = "Item Designator ID";
    obj["encoded"] = printRaw(klv);
    obj["units"] = lds.units;
    obj["format"] = lds.format;
    obj["key"] = klv.key();
    obj["length"] = klv.length();
    obj["value"] = val.str();*/
    obj.put("key", klv.key());
    obj.put("value", val.str());

    pt::ptree::value_type vt("Item_Designator_ID", obj);

    _set.push_back(vt);
}

void KlvSecuritySetVisitor::Visit(lcss::KLVVersion& klv)
{
    uint8_t value[2];
    klv.value(value);
    uint16_t val;
    memcpy(&val, value, 2);
    uint16_t nValue = ntohs(val);
    _set.push_back(printInt(_ldsDb, klv, nValue));
}

void KlvSecuritySetVisitor::Visit(lcss::KLVClassifyingCountryandReleasingInstructionsCountryCodingMethodVersionDate& klv)
{
    uint8_t* buf = new uint8_t[klv.length()]{};
    klv.value(buf);
    _set.push_back(printString(_ldsDb, klv));
    delete[] buf;
}

