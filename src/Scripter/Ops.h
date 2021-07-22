#ifndef COCOA_OPS_H
#define COCOA_OPS_H

#include <cstdint>
#include <cassert>
#include <vector>

#include "include/v8.h"
#include "Scripter/ScripterBase.h"
SCRIPTER_NS_BEGIN

#define OP_SUCCESS     0
#define OP_ETYPE       1
#define OP_EARGC       2
#define OP_EOPNUM      3
#define OP_EINTERNAL   4
#define OP_EINVARG     5
#define OP_ENOMEM      6
#define OP_EBADRID     7
#define OP_EBUSY       8
#define OP_EASYNC      9

#define OP_PRINT                    "op_print"
#define OP_DISPOSE                  "op_dispose"
#define OP_TIMER_CREATE             "op_timer_create"
#define OP_TIMER_CTL                "op_timer_ctl"
/* Cocoa/Vanilla engine Ops */
#define OP_VA_CTX_CREATE            "op_va_ctx_create"
#define OP_VA_CTX_CONNECT           "op_va_ctx_connect"

#define OP_VA_DIS_GEOMETRY          "op_va_dis_geometry"
#define OP_VA_DIS_NEW_WINDOW        "op_va_dis_new_window"
#define OP_VA_DIS_FLUSH             "op_va_dis_flush"
#define OP_VA_DIS_DISPOSE           "op_va_dis_dispose"
#define OP_VA_WIN_SET               "op_va_win_set"
#define OP_VA_WIN_GEOMETRY          "op_va_win_geometry"
#define OP_VA_WIN_SHOW              "op_va_win_show"
#define OP_VA_WIN_UPDATE            "op_va_win_update"
#define OP_VA_WIN_CLOSE             "op_va_win_close"

enum class OpTimerCtlVerb : uint8_t
{
    kStart          = 1,
    kStop           = 2,
    kSetInterval    = 3,
    kMaxEnum        = 4
};

enum class OpVaWindowSetVerb : uint8_t
{
    kTitle          = 1,
    kResizable      = 2,
    kIconFile       = 3,
    kEventListeners = 4,
    kMaxEnum        = 5
};

class Runtime;
class OpParameterInfo
{
public:
    enum class StorageType
    {
        kLocal,
        kPersistent
    };

    OpParameterInfo(v8::Isolate *isolate, v8::Local<v8::Object> object, StorageType type);
    OpParameterInfo(v8::Isolate *isolate, v8::Local<v8::Object> object,
                    v8::Local<v8::Promise::Resolver> promise, StorageType type);
    ~OpParameterInfo();

    inline v8::Isolate *isolate()
    { return fIsolate; }

    inline v8::Local<v8::Context> context()
    { return fIsolate->GetCurrentContext(); }

    inline StorageType storageType()
    { return fType; }

    v8::Global<v8::Promise::Resolver>& promise()
    { return fPromise; }

    inline Runtime *runtime()
    { return fRuntime; }

    v8::Local<v8::Object> get();
    v8::Local<v8::Value> operator[](const std::string& key);
    v8::Local<v8::Value> operator[](v8::Local<v8::String> key);

private:
    v8::Isolate                *fIsolate;
    Runtime                    *fRuntime;
    StorageType                 fType;
    v8::Local<v8::Object>       fLocal;
    v8::Persistent<v8::Object>  fPersistent;
    v8::Global<v8::Promise::Resolver>
                                fPromise;
};

using OpRet = int32_t;
using OpHandler = OpRet(*)(OpParameterInfo&);
struct OpEntry
{
    enum class ExecutionType
    {
        kAsynchronous,
        kSynchronous
    };

    std::string         name;
    uint64_t            nameHash;
    ExecutionType       executionType;
    OpHandler           pfn;

    uint64_t            callCount;
    OpEntry            *pLeft;
    OpEntry            *pRight;
};
void OpsTableHeapInitialize();
void OpsTableHeapDispose();
const OpEntry *OpsTableFind(const char *name);
void OpsTableInsertEntry(const char *name, OpEntry::ExecutionType type, OpHandler pfn);

#define OpHandlerImpl(N)          OpRet N(OpParameterInfo& param)

SCRIPTER_NS_END
#endif //COCOA_OPS_H
