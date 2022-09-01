/**
 * This file is part of Cocoa.
 *
 * Cocoa is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * Cocoa is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cocoa. If not, see <https://www.gnu.org/licenses/>.
 */

#include "Gallium/Gallium.h"
#include "Gallium/binder/Convert.h"
GALLIUM_BINDER_NS_BEGIN

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

GALLIUM_BINDER_NS_END
