#include "Koi/bindings/Base.h"

KOI_BINDINGS_NS_BEGIN

class AtriBinding : public BindingBase
{
public:
	AtriBinding() : BindingBase("ATRI", "わたしわ、高性能ですから！") {}
	~AtriBinding() override {}
	const char *getUniqueId() override;
	void getModule(binder::Module& self) override;
	const char **getExports() override;
};

void ATRIWakeup();

KOI_BINDINGS_NS_END

