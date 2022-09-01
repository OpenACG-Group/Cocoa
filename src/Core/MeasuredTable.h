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

#ifndef COCOA_CORE_MEASUREDTABLE_H
#define COCOA_CORE_MEASUREDTABLE_H

#include <functional>
#include <string>
#include <vector>
#include <sstream>

namespace cocoa
{

class MeasuredTable
{
public:
    explicit MeasuredTable(int32_t minSpaces = 4)
        : fIndent(0), fMinSpaces(minSpaces) { }

    template<typename T>
    void append(const std::string& hdr, const T& item)
    {
        std::ostringstream oss;
        oss << item;
        std::string str = oss.str();
        if (hdr.length() > fIndent)
            fIndent = hdr.length();
        fData.emplace_back(hdr, str);
    }

    inline void flush(const std::function<void(const std::string&)>& printer)
    {
        for (const auto& pair : fData)
        {
            std::ostringstream oss;
            oss << pair.first;
            for (int i = 0; i < fMinSpaces + fIndent - pair.first.length(); i++)
                oss << ' ';
            oss << pair.second;
            printer(oss.str());
        }
    }

private:
    using HdrDataPair = std::pair<std::string, std::string>;

    std::vector<HdrDataPair>    fData;
    int32_t                   fIndent;
    int32_t                   fMinSpaces;
};

}

#endif //COCOA_CORE_MEASUREDTABLE_H
