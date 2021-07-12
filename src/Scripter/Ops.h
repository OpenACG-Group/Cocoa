#ifndef COCOA_OPS_H
#define COCOA_OPS_H

#include <cstdint>
#include <cassert>
#include <vector>

#include "include/v8.h"
#include "Scripter/ScripterBase.h"
#include "Scripter/JavaScriptCodeGen.h"
SCRIPTER_NS_BEGIN

__js_codegen_pragma(error_definitions#begin)
#define OP_SUCCESS     0
#define OP_ETYPE       1
#define OP_EARGC       2
#define OP_EOPNUM      3
#define OP_EINTERNAL   4
#define OP_EINVARG     5
#define OP_EASYNC      6
#define OP_ENOMEM      7
__js_codegen_pragma(error_definitions#end)

__js_codegen_pragma(vmcall_definitions#begin)
#define OP_PRINT   "op_print"
__js_codegen_pragma(vmcall_definitions#end)

__js_codegen_pragma(eternal_rds_definitions#begin)
__js_codegen_pragma(eternal_rds_definitions#end)

__js_codegen_pragma(rd_mode_definitions#begin)
#define OPEN_RDONLY       0x001
#define OPEN_WRONLY       0x002
#define OPEN_RDWR         0x004
#define OPEN_TRUNC        0x008
#define OPEN_CREAT        0x010
#define CREAT_IRUSR       0x001
#define CREAT_IWUSR       0x002
#define CREAT_IXUSR       0x004
#define CREAT_IRGRP       0x008
#define CREAT_IWGRP       0x010
#define CREAT_IXGRP       0x020
#define CREAT_IROTH       0x040
#define CREAT_IWOTH       0x080
#define CREAT_IXOTH       0x100
__js_codegen_pragma(rd_mode_definitions#end)

class OpParameterInfo
{
public:
    enum class StorageType
    {
        kLocal,
        kPersistent
    };

    OpParameterInfo(v8::Isolate *isolate, v8::Local<v8::Object> object, StorageType type);
    ~OpParameterInfo();

    inline v8::Isolate *isolate()
    { return fIsolate; }

    inline v8::Local<v8::Context> context()
    { return fIsolate->GetCurrentContext(); }

    inline StorageType storageType()
    { return fType; }

    v8::Local<v8::Object> get();
    v8::Local<v8::Value> operator[](const std::string& key);
    v8::Local<v8::Value> operator[](v8::Local<v8::String> key);

private:
    v8::Isolate                *fIsolate;
    StorageType                 fType;
    v8::Local<v8::Object>       fLocal;
    v8::Persistent<v8::Object>  fPersistent;
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
