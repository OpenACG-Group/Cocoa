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

#ifndef COCOA_GLAMOR_MOEINTERPRETERENGINE_H
#define COCOA_GLAMOR_MOEINTERPRETERENGINE_H

#include "Glamor/Glamor.h"
#include "Glamor/Moe/MoeByteStreamReader.h"
#include "Glamor/Moe/MoeHeap.h"
GLAMOR_NAMESPACE_BEGIN

class MoeInterpreterEngine
{
public:
    explicit MoeInterpreterEngine(Unique<MoeByteStreamReader> reader);
    ~MoeInterpreterEngine();

    template<typename T>
    void LoadObjectToHeap(uint32_t key, T&& rvalue);

    template<typename T>
    void LoadObjectToHeap(uint32_t key, const T& lvalue);

    sk_sp<SkPicture> PerformInterpret();
    void GetLastHeapProfile(MoeHeap::Profile& out);

private:
    Unique<MoeByteStreamReader>             stream_reader_;
    MoeHeap                                 heap_;
};

template<typename T>
void MoeInterpreterEngine::LoadObjectToHeap(uint32_t key, T&& rvalue)
{
    heap_.Allocate<T>(key, std::forward<T>(rvalue));
}

template<typename T>
void MoeInterpreterEngine::LoadObjectToHeap(uint32_t key, const T& lvalue)
{
    heap_.Allocate<T>(key, lvalue);
}

GLAMOR_NAMESPACE_END
#endif //COCOA_GLAMOR_MOEINTERPRETERENGINE_H
