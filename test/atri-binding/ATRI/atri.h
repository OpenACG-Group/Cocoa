#include "Koi/bindings/Base.h"

KOI_BINDINGS_NS_BEGIN

class AtriBinding : public BindingBase
{
public:
	AtriBinding() : BindingBase("ATRI", "わたしわ、高性能ですから！") {}
	~AtriBinding() override {}
	const char *onGetUniqueId() override;
	void onGetModule(binder::Module& self) override;
	const char **onGetExports() override;
};

void ATRIWakeup();

KOI_BINDINGS_NS_END

