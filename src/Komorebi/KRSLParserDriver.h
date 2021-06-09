#ifndef COCOA_KRSLPARSERDRIVER_H
#define COCOA_KRSLPARSERDRIVER_H

#include <iostream>
#include "Komorebi/Namespace.h"
#include "Komorebi/KRSLParser.hh"
#include "Komorebi/KRSLScanner.h"
KOMOREBI_NS_BEGIN

class KRSLParserDriver
{
public:
    KRSLParserDriver();
    ~KRSLParserDriver() = default;

    int parse(std::istream& in, std::ostream& out);

private:
    Parser          fParser;
    KRSLScanner     fScanner;
};

KOMOREBI_NS_END
#endif //COCOA_KRSLPARSERDRIVER_H
