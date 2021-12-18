#include "Koi/bindings/Base.h"

KOI_BINDINGS_NS_BEGIN

class AtriBinding : public BindingBase
{
public:
	AtriBinding() : BindingBase("Atri", "わたしわ、高性能ですから！") {}
	~AtriBinding() override {}
	const char *getUniqueId() override;
	void getModule(binder::Module& self) override;
	const char **getExports() override;
};

int32_t atriRoboko(int32_t a, int32_t b, int32_t c);

KOI_BINDINGS_NS_END

