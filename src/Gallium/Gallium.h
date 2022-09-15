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

#ifndef COCOA_GALLIUM_GALLIUM_H
#define COCOA_GALLIUM_GALLIUM_H

#include <memory>

#include "Core/Project.h"

#define CHECKED(E)  E.ToLocalChecked()

#define GALLIUM_NS_BEGIN   namespace cocoa::gallium {
#define GALLIUM_NS_END     }

#define GALLIUM_BINDER_NS_BEGIN     namespace cocoa::gallium::binder {
#define GALLIUM_BINDER_NS_END       }

#define GALLIUM_BINDINGS_NS_BEGIN   namespace cocoa::gallium::bindings {
#define GALLIUM_BINDINGS_NS_END     }

#define GALLIUM_JS_TYPEOF_STRING    "string"
#define GALLIUM_JS_TYPEOF_OBJECT    "object"
#define GALLIUM_JS_TYPEOF_FUNCTION  "function"
#define GALLIUM_JS_TYPEOF_NUMBER    "number"
#define GALLIUM_JS_TYPEOF_BOOLEAN   "boolean"
#define GALLIUM_JS_TYPEOF_UNDEFINED "undefined"

#endif //COCOA_GALLIUM_GALLIUM_H
