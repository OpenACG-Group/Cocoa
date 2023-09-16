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

#include <utility>

#include "Core/Errors.h"
#include "Glamor/Cursor.h"
#include "Glamor/CursorTheme.h"
GLAMOR_NAMESPACE_BEGIN

GLAMOR_TRAMPOLINE_IMPL(Cursor, Dispose)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(0);
    info.GetThis()->As<Cursor>()->Dispose();
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
}

GLAMOR_TRAMPOLINE_IMPL(Cursor, GetHotspotVector)
{
    GLAMOR_TRAMPOLINE_CHECK_ARGS_NUMBER(0);
    auto v = info.GetThis()->As<Cursor>()->GetHotspotVector();
    info.SetReturnStatus(PresentRemoteCall::Status::kOpSuccess);
    info.SetReturnValue(v);
}

Cursor::Cursor(std::weak_ptr<CursorTheme> theme)
    : PresentRemoteHandle(RealType::kCursor)
    , disposed_(false)
    , theme_(std::move(theme))
{
    SetMethodTrampoline(GLOP_CURSOR_DISPOSE, Cursor_Dispose_Trampoline);
    SetMethodTrampoline(GLOP_CURSOR_GET_HOTSPOT_VECTOR, Cursor_GetHotspotVector_Trampoline);
}

Cursor::~Cursor()
{
    CHECK(disposed_ && "Cursor should be disposed before destructing");
}

void Cursor::Dispose()
{
    if (!disposed_)
    {
        this->OnDispose();
        disposed_ = true;

        if (!theme_.expired())
            theme_.lock()->RemoveCursorFromCache(Self()->As<Cursor>());
    }
}

SkIVector Cursor::GetHotspotVector()
{
    return this->OnGetHotspotVector();
}

bool Cursor::HasAnimation()
{
    return this->OnHasAnimation();
}

void Cursor::TryAbortAnimation()
{
    this->OnTryAbortAnimation();
}

void Cursor::TryStartAnimation()
{
    this->OnTryStartAnimation();
}

GLAMOR_NAMESPACE_END
