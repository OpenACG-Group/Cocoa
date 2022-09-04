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

#include "Glamor/Glamor.h"

#ifndef COCOA_GLAMOR_GSHADER_GSASSEMBLY_H
#define COCOA_GLAMOR_GSHADER_GSASSEMBLY_H
GLAMOR_NAMESPACE_BEGIN

#define TRIPLE_VECTOR(pred)     pred##2, pred##3, pred##4

enum class GSOpcode : uint32_t
{
    // Instruction prefixes:
    // -p: pointer (memory reference)
    // -b: byte or boolean value
    // -i: 32bits integer (signed and unsigned)
    // -si: 32bits signed integer
    // -ui: 32bits unsigned integer
    // -f: 32bits float number
    // -i<n>: vector with `n` components of 32bits integer (signed and unsigned)
    // -s<n>: vector with `n` components of 32bits signed integer
    // -u<n>: vector with `n` components of 32bits unsigned integer
    // -f<n>: vector with `n` components of 32bits float number

    // Do nothing, just a placeholder
    nop,

    // Pop out the element at the stack top
    drop,

    // Load and store elements of the operands stack
    loadp,
    loadi,
    loadf,
    TRIPLE_VECTOR(loadi),
    TRIPLE_VECTOR(loadf),
    storep,
    storei,
    storef,
    TRIPLE_VECTOR(storei),
    TRIPLE_VECTOR(storef),

    // Vector-only operating instructions
    TRIPLE_VECTOR(vpacki),
    TRIPLE_VECTOR(vpackf),
    TRIPLE_VECTOR(vunpacki),
    TRIPLE_VECTOR(vunpackf),
    TRIPLE_VECTOR(swizzlei),
    TRIPLE_VECTOR(swizzlef),

    // Scalar arithmetic instructions
    addi,
    addf,
    subi,
    subf,
    muli,
    mulf,
    negi,
    negf,
    divsi,
    divui,
    divf,
    remsi,
    remui,
    ceilf,
    floorf,
    truncf,
    sqrtf,
    nearestf,

    // Vector arithmetic instructions
    TRIPLE_VECTOR(addi),
    TRIPLE_VECTOR(addf),
    TRIPLE_VECTOR(subi),
    TRIPLE_VECTOR(subf),
    TRIPLE_VECTOR(muli),
    TRIPLE_VECTOR(mulf),
    TRIPLE_VECTOR(negi),
    TRIPLE_VECTOR(negf),
    TRIPLE_VECTOR(divsi),
    TRIPLE_VECTOR(divui),
    TRIPLE_VECTOR(divf),
    TRIPLE_VECTOR(remsi),
    TRIPLE_VECTOR(remui),
    TRIPLE_VECTOR(ceilf),
    TRIPLE_VECTOR(floorf),
    TRIPLE_VECTOR(truncf),
    TRIPLE_VECTOR(sqrtf),
    TRIPLE_VECTOR(nearestf),

    // Vector-only arithmetic instructions (linear algebra)
    TRIPLE_VECTOR(vdoti),
    TRIPLE_VECTOR(vdotf),
    TRIPLE_VECTOR(vcrossi),
    TRIPLE_VECTOR(vcrossf),
    TRIPLE_VECTOR(vnormf),
    TRIPLE_VECTOR(vlengthf)
};

enum class GSVMCallOpcode : uint32_t
{
    kKeywordImport,
    kPositionalImport
};

constexpr struct
{
    uint8_t x = 0;
    uint8_t y = 1;
    uint8_t z = 2;
    uint8_t w = 3;
} kGSSwizzleComp;

// NOLINTNEXTLINE
#define __SWC(P)    (cocoa::gl::kGSSwizzleComp.P)

#define GS_SWIZZLE2_ID(x, y)            (__SWC(x) | (__SWC(y) << 2))
#define GS_SWIZZLE3_ID(x, y, z)         (__SWC(X) | (__SWC(y) << 2) | (__SWC(z) << 4))
#define GS_SWIZZLE4_ID(x, y, z, w)      (__SWC(x) | (__SWC(y) << 2) | (__SWC(z) << 4) | (__SWC(w) << 6))

#undef __SWC

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_GSHADER_GSASSEMBLY_H
