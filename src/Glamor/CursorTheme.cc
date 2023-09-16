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

#include "Core/Errors.h"
#include "Glamor/Glamor.h"
#include "Glamor/CursorTheme.h"
#include "Glamor/Cursor.h"
GLAMOR_NAMESPACE_BEGIN

GLAMOR_TRAMPOLINE_IMPL(CursorTheme, Dispose)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(0);
    info.GetThis()->As<CursorTheme>()->Dispose();
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(CursorTheme, LoadCursorFromName)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(1);
    auto theme = info.GetThis()->As<CursorTheme>()->LoadCursorFromName(info.Get<std::string>(0));
    info.SetReturnStatus(theme ? PresentRemoteCall::Status::kOpSuccess
                               : PresentRemoteCall::Status::kOpFailed);
    info.SetReturnValue(theme);
}

CursorTheme::CursorTheme()
    : PresentRemoteHandle(RealType::kCursorTheme)
    , disposed_(false)
{
    SetMethodTrampoline(GLOP_CURSORTHEME_DISPOSE,
                        CursorTheme_Dispose_Trampoline);
    SetMethodTrampoline(GLOP_CURSORTHEME_LOAD_CURSOR_FROM_NAME,
                        CursorTheme_LoadCursorFromName_Trampoline);
}

CursorTheme::~CursorTheme()
{
    CHECK(disposed_ && "CursorTheme should be disposed before destructing");
}

void CursorTheme::Dispose()
{
    if (!disposed_)
    {
        // Cursor objects will remove themselves from the theme's cache
        // when `Cursor::Dispose` is called. That brings trouble to our
        // iteration, so we copy the cache and do the iteration on the
        // copied one.
        auto copy_cache = cached_cursors_;
        for (const auto& pair : copy_cache)
        {
            pair.second->Dispose();
        }

        // Implementation can release platform-specific resources now
        this->OnDispose();

        disposed_ = true;
    }
}

std::shared_ptr<Cursor>
CursorTheme::LoadCursorFromName(const std::string& name)
{
    if (cached_cursors_.count(name) > 0)
        return cached_cursors_[name];

    auto cursor = this->OnLoadCursorFromName(name);
    if (cursor)
        cached_cursors_[name] = cursor;
    return cursor;
}

void CursorTheme::RemoveCursorFromCache(const std::shared_ptr<Cursor>& cursor)
{
    auto itr = cached_cursors_.begin();
    for (; itr != cached_cursors_.end(); itr++)
    {
        if (itr->second == cursor)
            break;
    }
    if (itr == cached_cursors_.end())
        return;
    cached_cursors_.erase(itr);
}

GLAMOR_NAMESPACE_END
