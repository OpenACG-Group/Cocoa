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

#ifndef COCOA_CORE_TRACEEVENT_H
#define COCOA_CORE_TRACEEVENT_H

#include "perfetto.h"

PERFETTO_DEFINE_CATEGORIES(
        perfetto::Category("rendering")
                .SetDescription("Events from the rendering subsystem"),
        perfetto::Category("multimedia")
                .SetDescription("Events from the multimedia subsystem"),
        perfetto::Category("main")
                .SetDescription("Events related to JavaScript execution"),
        perfetto::Category("v8")
                .SetDescription("Trace JavaScript engine V8"),
        perfetto::Category("skia")
                .SetDescription("Trace rendering engine Skia"));

#endif //COCOA_CORE_TRACEEVENT_H
