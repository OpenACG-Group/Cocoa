#include <iostream>
#include "Komorebi/Namespace.h"
#include "Komorebi/KRSLParserDriver.h"
#include "Komorebi/KRSLScanner.h"
KOMOREBI_NS_BEGIN

KRSLParserDriver::KRSLParserDriver()
    : fParser(fScanner, *this),
      fScanner(*this)
{
}

int KRSLParserDriver::parse(std::istream& in, std::ostream& out)
{
    fScanner.switch_streams(in, out);
    return fParser.parse();
}

KOMOREBI_NS_END
