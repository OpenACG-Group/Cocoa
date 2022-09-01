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

#ifndef COCOA_GLAMOR_CURSORTHEME_H
#define COCOA_GLAMOR_CURSORTHEME_H

#include <unordered_map>

#include "Glamor/Glamor.h"
#include "Glamor/RenderClientObject.h"
GLAMOR_NAMESPACE_BEGIN

#define GLOP_CURSORTHEME_DISPOSE                    1
#define GLOP_CURSORTHEME_LOAD_CURSOR_FROM_NAME      2

class Cursor;

class CursorTheme : public RenderClientObject
{
public:
    ~CursorTheme() override;

    Shared<Cursor> LoadCursorFromName(const std::string& name);
    void Dispose();

    g_private_api void RemoveCursorFromCache(const Shared<Cursor>& cursor);

protected:
    CursorTheme();

    virtual void OnDispose() = 0;
    virtual Shared<Cursor> OnLoadCursorFromName(const std::string& name) = 0;

private:
    bool                        disposed_;
    std::unordered_map<std::string, Shared<Cursor>>
                                cached_cursors_;
};

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_CURSORTHEME_H
