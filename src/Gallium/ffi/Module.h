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

#ifndef COCOA_GALLIUM_FFI_MODULE_H
#define COCOA_GALLIUM_FFI_MODULE_H

#include <unordered_map>
#include <map>

#include "Gallium/Gallium.h"
#include "Gallium/ffi/DataTypes.h"
GALLIUM_FFI_NS_BEGIN

class NativeModule;

using LocalExports = std::unordered_map<std::string, Local<Value>>;
using GlobalExports = std::unordered_map<std::string, v8::Global<Value>>;

// A table that connects `ffi::Module` with `v8::Module`.
// Per-isolate data.
class ModuleRegistry
{
public:
    ModuleRegistry();

    struct ModuleInfo
    {
        GlobalExports   exports;
        NativeModule   *native_module;
    };

    static ModuleRegistry *FromIsolate(Isolate *isolate);

    NativeModule *SearchModule(const std::string_view& name);

    void StoreInstantiated(NativeModule *native_module, Local<v8::Module> v8_module,
                           const LocalExports& exports);
    ModuleInfo *FetchModuleInfo(Local<v8::Module> v8_module);

    ModuleInfo *FetchOrInstantiateModuleInfo(v8::Isolate *isolate, const std::string& name);

private:
    std::unordered_map<std::string, std::unique_ptr<NativeModule>> module_map_;
    std::map<int, ModuleInfo>  instantiated_map_;
};

class NativeModule
{
public:
    NativeModule(std::string name, std::string description, std::vector<std::string> deps = {});
    virtual ~NativeModule() = default;

    const std::string& GetName() const {
        return name_;
    }

    const std::string& GetDescription() const {
        return description_;
    }

    const std::vector<std::string>& GetDeps() const {
        return deps_;
    }

    Local<v8::Module> Instantiate(Isolate *isolate);

protected:
    virtual LocalExports OnBuildExports(Isolate *isolate) = 0;

    // Helper functions for `OnBuildExports`

    /**
     * Load and evaluate a JavaScript located at `url`. The result of evaluation
     * is expected to be a function whose return value is a dictionary object that
     * represents the exports.
     * Then those exports will be inserted into `target`. Do nothing if any JS
     * exception is thrown or the evaluation result is incompatible (a JS exception
     * will be thrown).
     *
     * For example, given JavaScript:
     * @code
     * (function (currentExports) {
     *   function Say(str) { ... }
     *   function Greet(name) { ... }
     *   return { say: Say, greet: Greet };
     * });
     * @endcode
     *
     * Two JavaScript objects `Say` and `Greet` will be inserted into `target`
     * with name `say` and `greet`, respectively.
     * The parameter `currentExports` receives a dictionary object, which represents
     * a copy of values that have been exported so far (before the invocation of the function).
     */
    static void InsertScriptExports(Isolate *isolate, const std::string& url, LocalExports& target);

private:
    std::string                 name_;
    std::string                 description_;
    std::vector<std::string>    deps_;
    v8::Global<v8::Module>      module_;
};

GALLIUM_FFI_NS_END
#endif //COCOA_GALLIUM_FFI_MODULE_H
