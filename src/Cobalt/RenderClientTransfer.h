#ifndef COCOA_RENDERCLIENTTRANSFER_H
#define COCOA_RENDERCLIENTTRANSFER_H

#include "Cobalt/Cobalt.h"
COBALT_NAMESPACE_BEGIN

class RenderClientTransfer
{
public:
    enum class Type
    {
        kInvocationResponse,
        kSignalEmit
    };

    explicit RenderClientTransfer(Type type) : type_(type) {}
    virtual ~RenderClientTransfer() = default;

    g_nodiscard g_inline bool IsInvocationResponse() const {
        return (type_ == Type::kInvocationResponse);
    }

    g_nodiscard g_inline bool IsSignalEmit() const {
        return (type_ == Type::kSignalEmit);
    }

private:
    Type        type_;
};

COBALT_NAMESPACE_END
#endif //COCOA_RENDERCLIENTTRANSFER_H
