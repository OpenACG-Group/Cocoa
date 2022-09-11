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

#ifndef COCOA_CORE_CMDPARSER_H
#define COCOA_CORE_CMDPARSER_H

#include <optional>
#include <string>
#include <vector>
#include <cstdint>

#include "Core/Project.h"
namespace cocoa::cmd {

enum class ValueType
{
    kString,
    kInteger,
    kFloat,
    kBoolean
};

struct Template
{
    enum class RequireValue
    {
        kEmpty,
        kNecessary,
        kOptional
    };

    const char                 *long_name = nullptr;
    std::optional<char>         short_name;
    RequireValue                has_value = RequireValue::kEmpty;
    std::optional<ValueType>    value_type;
    const char                 *desc = nullptr;
};

enum class ParseState
{
    kExit,
    kSuccess,
    kError,
    kJustInitialize
};

struct ParseResult
{
    struct Option
    {
        struct Value
        {
            std::string     v_str;
            int32_t         v_int;
            float           v_float;
            bool            v_bool;
        };
        const Template          *matched_template = nullptr;
        std::string              origin;
        std::optional<Value>     value;
    };

    std::vector<const char*> orphans;
    std::vector<Option>      options;
};

ParseState Parse(int argc, const char **argv, ParseResult& result);
void PrintHelp(const char *program);

} // namespace cocoa::cmd
#endif //COCOA_CORE_CMDPARSER_H
