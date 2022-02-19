#include <iostream>
#include "Koi/bindings/Base.h"
#include "atri.h"
KOI_BINDINGS_NS_BEGIN

void ATRIWakeup()
{
	std::cout << "「ありがとう、私の好きな人\n"
                 "もう二度と帰らない愛しい日々...」\n";
}

KOI_BINDING_LOADER_HOOK
{
	KOI_HOOK_RET(new AtriBinding());
}

KOI_BINDINGS_NS_END
