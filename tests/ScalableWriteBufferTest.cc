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

#include "ScalableWriteBufferTest.h"

#include "fmt/format.h"
#include "Core/ScalableWriteBuffer.h"
#include "Core/Data.h"

namespace cocoa {

bool ScalableWriteBufferTest()
{
    // Test 1. Used to concatenate strings
    ScalableWriteBuffer buf(8);

    std::string str1("A brown dog jumps over the lazy fox.");
    std::string str2("You should see this string concatenated after another string.");
    std::string str3("This is the last string.");

    buf.writeBytes(reinterpret_cast<const uint8_t*>(str1.c_str()), str1.length());
    buf.writeBytes(reinterpret_cast<const uint8_t*>(str2.c_str()), str2.length());
    buf.writeBytes(reinterpret_cast<const uint8_t*>(str3.c_str()), str3.length() + 1);

    std::shared_ptr<Data> result = buf.finalize();

    fmt::print("Result: [{}]\n", reinterpret_cast<const char*>(result->getAccessibleBuffer()));

    return true;
}

} // namespace cocoa
