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

#include "Gallium/bindings/reactor/Exports.h"
#include "Gallium/binder/Class.h"
GALLIUM_BINDINGS_REACTOR_NS_BEGIN

ReactorBinding::ReactorBinding()
    : BindingBase("reactor", "LLVM Based Machine Code Generator")
{
    using P = reactor::Options::CodeOptPass;

    Bitfield<P> passes;
    passes |= P::kCFGSimplification;
    passes |= P::kLICM;
    passes |= P::kAggressiveDCE;
    passes |= P::kGVN;
    passes |= P::kInstructionCombining;
    passes |= P::kReassociate;
    passes |= P::kDeadStoreElimination;
    passes |= P::kSCCP;
    passes |= P::kSROA;
    passes |= P::kEarlyCSE;

    reactor::InitializePlatform({
        reactor::Options::CodeOptLevels::kDefault,
        passes
    });
}

ReactorBinding::~ReactorBinding()
{
    reactor::DisposePlatform();
}

void ReactorBinding::onRegisterClasses(v8::Isolate *isolate)
{
    gshader_builder_wrap_ = NewClassExport<GShaderBuilderWrap>(isolate);
    (*gshader_builder_wrap_)
        .constructor<const std::string&>()
        .set("mainTestCodeGen", &GShaderBuilderWrap::mainTestCodeGen)
        .set("userMainEntrypointBasicBlock", &GShaderBuilderWrap::userMainEntrypointBasicBlock)
        .set("insertJSFunctionSymbol", &GShaderBuilderWrap::insertJSFunctionSymbol);

    gshader_module_wrap_ = NewClassExport<GShaderModuleWrap>(isolate);
    (*gshader_module_wrap_)
        .set_static_func("Compile", GShaderModuleWrap::Compile)
        .set("executeMain", &GShaderModuleWrap::executeMain);

    value_wrap_ = NewClassExport<ValueWrap>(isolate);

    basic_block_wrap_ = NewClassExport<BasicBlockWrap>(isolate);
    (*basic_block_wrap_)
        .set("newByte", &BasicBlockWrap::newByte)
        .set("newByte2", &BasicBlockWrap::newByte2)
        .set("newByte4", &BasicBlockWrap::newByte4)
        .set("newSByte", &BasicBlockWrap::newSByte)
        .set("newSByte2", &BasicBlockWrap::newSByte2)
        .set("newSByte4", &BasicBlockWrap::newSByte4)
        .set("newShort", &BasicBlockWrap::newShort)
        .set("newShort2", &BasicBlockWrap::newShort2)
        .set("newShort4", &BasicBlockWrap::newShort4)
        .set("newUShort", &BasicBlockWrap::newUShort)
        .set("newUShort2", &BasicBlockWrap::newUShort2)
        .set("newUShort4", &BasicBlockWrap::newUShort4)
        .set("newInt", &BasicBlockWrap::newInt)
        .set("newInt2", &BasicBlockWrap::newInt2)
        .set("newInt4", &BasicBlockWrap::newInt4)
        .set("newUInt", &BasicBlockWrap::newUInt)
        .set("newUInt2", &BasicBlockWrap::newUInt2)
        .set("newUInt4", &BasicBlockWrap::newUInt4)
        .set("newLong", &BasicBlockWrap::newLong)
        .set("newLong2", &BasicBlockWrap::newLong2)
        .set("newLong4", &BasicBlockWrap::newLong4)
        .set("newULong", &BasicBlockWrap::newULong)
        .set("newULong2", &BasicBlockWrap::newULong2)
        .set("newULong4", &BasicBlockWrap::newULong4)
        .set("newFloat", &BasicBlockWrap::newFloat)
        .set("newFloat2", &BasicBlockWrap::newFloat2)
        .set("newFloat4", &BasicBlockWrap::newFloat4)
        .set("createReturn", &BasicBlockWrap::createReturn)
        .set("createJSFunctionCall", &BasicBlockWrap::createJSFunctionCall);
}

GALLIUM_BINDINGS_REACTOR_NS_END
