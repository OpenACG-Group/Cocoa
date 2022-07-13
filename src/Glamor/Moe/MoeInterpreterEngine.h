#ifndef COCOA_GLAMOR_MOEINTERPRETERENGINE_H
#define COCOA_GLAMOR_MOEINTERPRETERENGINE_H

#include "Glamor/Glamor.h"
#include "Glamor/Moe/MoeByteStreamReader.h"
#include "Glamor/Moe/MoeHeap.h"
#include "Glamor/Moe/MoeExternalBreakpointHandler.h"
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

    void AttachExternalBreakpointHandler(Unique<MoeExternalBreakpointHandler>&& handler);

    g_nodiscard g_inline const auto& GetExternalBreakpointHandler() const {
        return external_breakpoint_handler_;
    }

    sk_sp<SkPicture> PerformInterpret();
    void GetLastHeapProfile(MoeHeap::Profile& out);

private:
    Unique<MoeByteStreamReader>             stream_reader_;
    MoeHeap                                 heap_;
    Unique<MoeExternalBreakpointHandler>    external_breakpoint_handler_;
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
