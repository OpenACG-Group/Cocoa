#include <iostream>
#include "Koi/bindings/Base.h"
#include "atri.h"
KOI_BINDINGS_NS_BEGIN

int32_t atriRoboko(int32_t a, int32_t b, int32_t c)
{
	std::cout << "atriRoboko() in native\n";
	return (a + b) * c;
}

extern "C" AtriBinding *__g_cocoa_hook()
{
	return new AtriBinding();
}

KOI_BINDINGS_NS_END

