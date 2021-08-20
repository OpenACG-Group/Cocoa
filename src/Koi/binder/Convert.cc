#include "Koi/KoiBase.h"
#include "Koi/binder/Convert.h"
KOI_BINDER_NS_BEGIN

template struct convert<std::string>;
template struct convert<std::string_view>;
// template struct convert<char const*>;

template struct convert<std::u16string>;
template struct convert<std::u16string_view>;
// template struct convert<char16_t const*>;

// template struct convert<bool>;

template struct convert<char>;
template struct convert<signed char>;
template struct convert<unsigned char>;

template struct convert<short>;
template struct convert<unsigned short>;

template struct convert<int>;
template struct convert<unsigned int>;

template struct convert<long>;
template struct convert<unsigned long>;

template struct convert<long long>;
template struct convert<unsigned long long>;

template struct convert<float>;
template struct convert<double>;
template struct convert<long double>;

KOI_BINDER_NS_END
