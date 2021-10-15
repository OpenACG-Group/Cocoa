#include <iostream>
#include <optional>
#include <sstream>
#include <random>

#include "include/v8.h"

#include "Core/CpuInfo.h"
#include "Core/Properties.h"
#include "Core/EventLoop.h"
#include "Core/EventSource.h"
#include "Core/Journal.h"
#include "Core/Filesystem.h"
#include "Core/CrpkgImage.h"

#include "Koi/binder/Class.h"
#include "Koi/binder/Module.h"
#include "Koi/binder/CallV8.h"
#include "Koi/lang/Base.h"
#include "Koi/lang/CoreBinding.h"
KOI_LANG_NS_BEGIN

#define FDLR_MAP_INITIAL_SIZE   128

#define THIS_FILE_MODULE COCOA_MODULE_NAME(Koi.lang)

/////////////////////////////////////////////////////////
// Utilities
//
std::optional<int> parse_property_array_subscript(const std::string_view& spec,
                                                  bool& thrown,
                                                  v8::Isolate *isolate)
{
    thrown = false;
    if (!spec.starts_with('#'))
        return {};

    std::string_view subscript(spec);
    subscript.remove_prefix(1);

    std::string subscript_dump(subscript);

    char *end_ptr = nullptr;
    long subscript_val = std::strtol(subscript_dump.c_str(), &end_ptr, 10);
    if (end_ptr - subscript_dump.c_str() != subscript_dump.size())
    {
        binder::throw_(isolate, "Array subscript should be an integer");
        thrown = true;
        return {};
    }

    if (subscript_val < 0)
    {
        binder::throw_(isolate, "Array subscript should be a positive integer");
        thrown = true;
        return {};
    }

    if (subscript_val > INT_MAX)
    {
        binder::throw_(isolate, "Array subscript is too large");
        thrown = true;
        return {};
    }

    return std::make_optional(static_cast<int>(subscript_val));
}

std::shared_ptr<PropertyNode> parse_property_spec(const std::string& spec,
                                                  bool& thrown,
                                                  v8::Isolate *isolate)
{
    thrown = false;

    std::vector<std::string_view> selectors;
    size_t p = 0;
    int64_t last_p = -1;
    while ((p = spec.find('.', p + 1)) != std::string::npos)
    {
        std::string_view view(spec);
        view.remove_prefix(last_p + 1);
        view.remove_suffix(spec.size() - p);
        selectors.emplace_back(view);
        last_p = static_cast<int64_t>(p);
    }
    std::string_view view(spec);
    view.remove_prefix(last_p + 1);
    selectors.emplace_back(view);

    std::shared_ptr<PropertyNode> currentNode = prop::Get();
    for (auto const& sel : selectors)
    {
        if (currentNode->kind() == PropertyNode::Kind::kData)
            return nullptr;

        auto maybe_array = parse_property_array_subscript(sel, thrown, isolate);
        if (thrown)
            return nullptr;

        if (maybe_array)
        {
            if (currentNode->kind() != PropertyNode::Kind::kArray)
            {
                binder::throw_(isolate, "Illegal usage of array subscript");
                thrown = true;
                return nullptr;
            }

            int subscript = maybe_array.value();
            auto array_node = prop::Cast<PropertyArrayNode>(currentNode);
            if (subscript >= array_node->size())
                return nullptr;

            currentNode = array_node->at(subscript);
        }
        else
        {
            if (currentNode->kind() != PropertyNode::Kind::kObject)
            {
                binder::throw_(isolate, "Illegal usage of member selector");
                thrown = true;
                return nullptr;
            }

            auto object_node = prop::Cast<PropertyObjectNode>(currentNode);
            if (!object_node->hasMember(std::string(sel)))
                return nullptr;

            currentNode = object_node->getMember(std::string(sel));
        }
    }

    return currentNode;
}

/////////////////////////////////////////////////////////
// Cocoa.core.print()
//

void jni_core_print(const std::string& str)
{
    if (str.empty())
        return;
    std::fwrite(str.c_str(), str.size(), 1, stdout);
}

/////////////////////////////////////////////////////////
// Cocoa.core.delay()
//

class jni_core_DelayTimer : public TimerSource
{
public:
    jni_core_DelayTimer(uint64_t timeout,
                        v8::Isolate *isolate,
                        v8::Local<v8::Promise::Resolver> resolver)
        : TimerSource(EventLoop::Instance()),
          fIsolate(isolate),
          fResolve(fIsolate, resolver)
    {
        startTimer(timeout);
    }
    ~jni_core_DelayTimer() override
    {
        fResolve.Reset();
    }

private:
    KeepInLoop timerDispatch() override
    {
        v8::Local<v8::Promise::Resolver> resolve = fResolve.Get(fIsolate);
        resolve->Resolve(fIsolate->GetCurrentContext(), v8::Null(fIsolate))
            .Check();
        delete this;
        return KeepInLoop::kDeleted;
    }

    v8::Isolate *fIsolate;
    v8::Global<v8::Promise::Resolver>   fResolve;
};

v8::Local<v8::Value> jni_core_delay(uint64_t timeout)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    auto resolver = v8::Promise::Resolver::New(isolate->GetCurrentContext()).ToLocalChecked();

    /* The object will be deleted in jni_core_DelayTimer::timerDispatch(). */
    new jni_core_DelayTimer(timeout, isolate, resolver);
    return resolver->GetPromise();
}

/////////////////////////////////////////////////////////
// Cocoa.core.getProperty(), Cocoa.core.hasProperty()
//

v8::Local<v8::Value> jni_core_getProperty(const std::string& spec)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    bool thrown = false;
    std::shared_ptr<PropertyNode> maybeNode = parse_property_spec(spec, thrown, isolate);
    if (thrown)
        return {};

    if (!maybeNode || maybeNode->protection() == PropertyNode::Protection::kPrivate)
    {
        binder::throw_(isolate, "No such property or property is inaccessible to JavaScript");
        return {};
    }

    if (maybeNode->kind() != PropertyNode::Kind::kData)
    {
        binder::throw_(isolate, "Property is an array or object");
        return {};
    }

    auto node = prop::Cast<PropertyDataNode>(maybeNode);
#define T_(t)  (node->type() == typeid(t))
    if (T_(int8_t) || T_(int16_t) || T_(int32_t))
        return v8::Integer::New(isolate, node->extract<int32_t>());
    else if (T_(uint8_t) || T_(uint16_t) || T_(uint32_t))
        return v8::Integer::NewFromUnsigned(isolate, node->extract<uint32_t>());
    else if (T_(int64_t))
        return v8::BigInt::New(isolate, node->extract<int64_t>());
    else if (T_(uint64_t))
        return v8::BigInt::NewFromUnsigned(isolate, node->extract<uint64_t>());
    else if (T_(bool))
        return v8::Boolean::New(isolate, node->extract<bool>());
    else if (T_(float))
        return v8::Number::New(isolate, node->extract<float>());
    else if (T_(double))
        return v8::Number::New(isolate, node->extract<double>());
    else if (T_(const char*))
        return v8::String::NewFromUtf8(isolate, node->extract<const char*>()).ToLocalChecked();
    else if (T_(std::string))
        return v8::String::NewFromUtf8(isolate, node->extract<std::string>().c_str()).ToLocalChecked();
#undef T_

    binder::throw_(isolate, "Property is not of primitive type");
    return {};
}

bool jni_core_hasProperty(const std::string& spec)
{
    bool thrown = false;
    std::shared_ptr<PropertyNode> node = parse_property_spec(spec, thrown, v8::Isolate::GetCurrent());
    if (thrown)
        return {};

    return (node && node->protection() != PropertyNode::Protection::kPrivate);
}

/////////////////////////////////////////////////////////
// jni_core_Timer (Cocoa.core.Timer)
//

class jni_core_Timer : public TimerSource
{
public:
    jni_core_Timer()
        : TimerSource(EventLoop::Instance()) {}
    ~jni_core_Timer() override = default;

    static binder::Class<jni_core_Timer> GetClass() {
        return binder::Class<jni_core_Timer>(v8::Isolate::GetCurrent())
                .constructor<>()
                .set("setInterval", &jni_core_Timer::setInterval)
                .set("setTimeout", &jni_core_Timer::setTimeout)
                .set("stop", &jni_core_Timer::stop);
    }

    void setInterval(uint64_t timeout, int64_t repeat, v8::Local<v8::Value> cbValue);
    void setTimeout(uint64_t timeout, v8::Local<v8::Value> cbValue);
    void stop();

private:
    KeepInLoop timerDispatch() override;

    v8::Global<v8::Function> fCallback;
};

void jni_core_Timer::setInterval(uint64_t timeout, int64_t repeat, v8::Local<v8::Value> cbValue)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    if (!cbValue->IsFunction())
    {
        binder::throw_(isolate, "Callback should be a function");
        return;
    }
    fCallback = v8::Global<v8::Function>(isolate, v8::Local<v8::Function>::Cast(cbValue));
    TimerSource::startTimer(timeout, repeat);
}

void jni_core_Timer::setTimeout(uint64_t timeout, v8::Local<v8::Value> cbValue)
{
    setInterval(timeout, 0, cbValue);
}

void jni_core_Timer::stop()
{
    TimerSource::stopTimer();
    fCallback.Reset();
}

KeepInLoop jni_core_Timer::timerDispatch()
{
    if (fCallback.IsEmpty())
    {
        LOGW(LOG_WARNING, "A timer was dispatched but the callback function is empty")
        return KeepInLoop::kNo;
    }
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    v8::Local<v8::Value> result = binder::call_v8(isolate,
                                                  fCallback.Get(isolate),
                                                  isolate->GetCurrentContext()->Global());

    if (result->IsBoolean())
    {
        bool ret = result->ToBoolean(isolate)->Value();
        return ret ? KeepInLoop::kYes : KeepInLoop::kNo;
    }
    return KeepInLoop::kYes;
}

/////////////////////////////////////////////////////////
// Cocoa.core.open, Cocoa.core.close, Cocoa.core.read, etc.
//

/**
 * FDLR (File Descriptor Layout Randomization):
 * JavaScript can NOT access system's file descriptor (real file descriptor)
 * for safety.
 */

struct FDLRTable
{
    enum OwnerType
    {
        kSystem,
        kUser,
        kUnknown
    };

    struct Target
    {
        int32_t fd;
        OwnerType owner;
        void(*closer)(int32_t);
        bool used : 1;
    } *pMap;
    size_t allocatedCount;
} gFDLRTable;

#define FDLR_MAP_SIZE   sizeof(FDLRTable::Target)

int32_t FDLRNewRandomizedDescriptor(int32_t realFd, FDLRTable::OwnerType owner, void (*closer)(int32_t))
{
    std::vector<std::array<size_t, 2>> unusedRegions;
    for (size_t i = 0; i < gFDLRTable.allocatedCount; i++)
    {
        if (gFDLRTable.pMap[i].used)
            continue;
        size_t l = i, r = i;
        while (!gFDLRTable.pMap[r].used && r < gFDLRTable.allocatedCount)
            r++;
        unusedRegions.push_back({l, r - 1});
        i = r - 1;
    }

    if (unusedRegions.empty())
    {
        gFDLRTable.allocatedCount *= 2;
        gFDLRTable.pMap = static_cast<FDLRTable::Target*>(std::realloc(gFDLRTable.pMap,
                                                                       gFDLRTable.allocatedCount * FDLR_MAP_SIZE));
        assert(gFDLRTable.pMap);
        for (size_t i = gFDLRTable.allocatedCount / 2; i < gFDLRTable.allocatedCount; i++)
        {
            gFDLRTable.pMap[i].used = false;
            gFDLRTable.pMap[i].closer = nullptr;
        }
        LOGF(LOG_DEBUG, "Expanding FDLR allocation map memory to {} bytes", gFDLRTable.allocatedCount)
        unusedRegions.push_back({gFDLRTable.allocatedCount / 2, gFDLRTable.allocatedCount - 1});
    }

    unsigned long long seed;
    if (CpuInfo::Ref().getX86Info().features.rdrnd)
    {
        __builtin_ia32_rdrand64_step(&seed);
    }
    else
    {
        seed = 2233;
    }
    std::mt19937 twisterEngine(seed);
    std::uniform_int_distribution<size_t> generator(0, unusedRegions.size() - 1);
    std::array<size_t, 2> targetRange{};
    {
        size_t idx = generator(twisterEngine);
        targetRange = unusedRegions[idx];
    }

    generator = std::uniform_int_distribution<size_t>(targetRange[0], targetRange[1]);
    auto finalFd = static_cast<int32_t>(generator(twisterEngine));
    gFDLRTable.pMap[finalFd].used = true;
    gFDLRTable.pMap[finalFd].fd = realFd;
    gFDLRTable.pMap[finalFd].owner = owner;
    gFDLRTable.pMap[finalFd].closer = closer;
    return finalFd;
}

void FDLRMarkUnused(int32_t rfd)
{
    if (rfd >= gFDLRTable.allocatedCount)
        return;
    gFDLRTable.pMap[rfd].used = false;
    gFDLRTable.pMap[rfd].closer = nullptr;
}

FDLRTable::Target *FDLRGetUnderlyingDescriptor(int32_t rfd)
{
    if (rfd >= gFDLRTable.allocatedCount || !gFDLRTable.pMap[rfd].used)
    {
        return nullptr;
    }
    return &gFDLRTable.pMap[rfd];
}

void FDLRInitialize()
{
    gFDLRTable.pMap = static_cast<FDLRTable::Target*>(std::malloc(FDLR_MAP_SIZE * FDLR_MAP_INITIAL_SIZE));
    gFDLRTable.allocatedCount = FDLR_MAP_INITIAL_SIZE;
    for (size_t i = 0; i < gFDLRTable.allocatedCount; i++)
    {
        gFDLRTable.pMap[i].used = false;
        gFDLRTable.pMap[i].closer = nullptr;
    }
}

void FDLRCollectAndSweep()
{
    for (size_t i = 0; i < gFDLRTable.allocatedCount; i++)
    {
        if (gFDLRTable.pMap[i].used && gFDLRTable.pMap[i].closer)
        {
            LOGF(LOG_WARNING, "Unclosed (randomized) file descriptor #{}", i)
            gFDLRTable.pMap[i].closer(gFDLRTable.pMap[i].fd);
        }
    }
    std::free(gFDLRTable.pMap);
}

void FDLRDumpMappingInfo()
{
    size_t usedCount = 0;
    for (size_t i = 0; i < gFDLRTable.allocatedCount; i++)
    {
        if (gFDLRTable.pMap[i].used)
            usedCount++;
    }
    LOGF(LOG_DEBUG, "%fg<hl>FDLR subsystem statistics: {} entries, {} used%reset", gFDLRTable.allocatedCount, usedCount)

    size_t p = 0;
    for (size_t i = 0; i < gFDLRTable.allocatedCount; i++)
    {
        if (gFDLRTable.pMap[i].used)
        {
            std::ostringstream os;
            if (gFDLRTable.pMap[i].owner == FDLRTable::kSystem)
                os << "%fg<re,hl>system%reset<>,";
            else if (gFDLRTable.pMap[i].closer)
                os << "%fg<gr,hl>closable%reset<>,";

            if (gFDLRTable.pMap[i].fd == STDIN_FILENO)
                os << "%fg<bl,hl>stdin%reset<>,";
            else if (gFDLRTable.pMap[i].fd == STDOUT_FILENO)
                os << "%fg<bl,hl>stdout%reset<>,";
            else if (gFDLRTable.pMap[i].fd == STDERR_FILENO)
                os << "%fg<bl,hl>stderr%reset<>,";

            std::string_view str = os.view();
            if (str.length() > 0)
                str.remove_suffix(1);
            LOGF(LOG_DEBUG, "  Entry#{} %fg<ma,hl>{:03d}%reset -> %fg<cy,hl>{:03d}%reset [{}]",
                 p++, i, gFDLRTable.pMap[i].fd, str)
        }
    }
}

/**
 * Argument #2, Open flags:
 *  r   read only
 *  w   write only
 *  rw  read-write
 *  +   create if not exists
 *  a   append mode
 *  t   truncate
 */
int32_t jni_core_open(const v8::FunctionCallbackInfo<v8::Value>& info)
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    CHECK_AND_JS_THROW_WITH_RET(info.Length() < 2, "Too few arguments", -1);
    CHECK_AND_JS_THROW_WITH_RET(info.Length() > 3, "Too much arguments", -1);

    CHECK_AND_JS_THROW_WITH_RET(!info[0]->IsString(), "File path should be a string", -1);
    std::string path = binder::from_v8<std::string>(isolate, info[0]);

    CHECK_AND_JS_THROW_WITH_RET(!info[1]->IsString(), "Open flags should be a string", -1);
    std::string strFlags = binder::from_v8<std::string>(isolate, info[1]);
    vfs::Bitfield<vfs::OpenFlags> flags;
    bool fR = false, fW= false;
    for (char p : strFlags)
    {
        switch (p)
        {
        case 'r': fR = true; break;
        case 'w': fW = true; break;
        case '+': flags |= vfs::OpenFlags::kCreate; break;
        case 'a': flags |= vfs::OpenFlags::kAppend; break;
        case 't': flags |= vfs::OpenFlags::kTrunc; break;
        default: CHECK_AND_JS_THROW_WITH_RET(true, "Bad open mode", -1);
        }
    }
    if (fR && fW)
        flags |= vfs::OpenFlags::kReadWrite;
    else if (fR)
        flags |= vfs::OpenFlags::kReadonly;
    else if (fW)
        flags |= vfs::OpenFlags::kWriteOnly;

    vfs::Bitfield<vfs::Mode> mode({vfs::Mode::kUsrR, vfs::Mode::kUsrW,
                                   vfs::Mode::kGrpR, vfs::Mode::kGrpW,
                                   vfs::Mode::kOthR});
    if (info.Length() == 3)
    {
        CHECK_AND_JS_THROW_WITH_RET(!info[2]->IsUint32(),
                                    "Creation mode should be an unsigned integer", -1);
        mode = vfs::Bitfield<vfs::Mode>(binder::from_v8<uint32_t>(isolate, info[2]));
    }
    int32_t fd = vfs::Open(path, flags, mode);
    return fd < 0 ? fd : FDLRNewRandomizedDescriptor(fd, FDLRTable::OwnerType::kUser, [](int32_t fd) {
        vfs::Close(fd);
    });
}

void jni_core_close(int32_t fd)
{
    FDLRTable::Target *target = FDLRGetUnderlyingDescriptor(fd);
    CHECK_AND_JS_THROW(!target, "Corrupted file descriptor");
    CHECK_AND_JS_THROW(!target->closer, "File descriptor is not closable");
    target->closer(target->fd);
    FDLRMarkUnused(fd);
}

void jni_core_dump(const std::string& what)
{
    if (what == "fdlr-info")
    {
        FDLRDumpMappingInfo();
    }
    else
    {
        CHECK_AND_JS_THROW(false, "Unknown dump target");
    }
}

void jni_core_exit()
{
    v8::Isolate *isolate = v8::Isolate::GetCurrent();
    isolate->TerminateExecution();
    EventLoop::Instance()->dispose();
    LOGW(LOG_DEBUG, "Program will be terminated directly by JavaScript")
}

CoreBindingModule::CoreBindingModule()
        : BaseBindingModule("core",
                            "Basic language features for Cocoa JavaScript")
{
    FDLRInitialize();
}

CoreBindingModule::~CoreBindingModule()
{
    FDLRCollectAndSweep();
}

#define SET_CONST_ENUM(name, val)   self.set_const(name, static_cast<uint32_t>(val))

void CoreBindingModule::getModule(binder::Module& self)
{
    auto jni_core_Timer_class = jni_core_Timer::GetClass();

    int32_t vfdStdin = FDLRNewRandomizedDescriptor(STDIN_FILENO, FDLRTable::kSystem, nullptr);
    int32_t vfdStdout = FDLRNewRandomizedDescriptor(STDOUT_FILENO, FDLRTable::kSystem, nullptr);
    int32_t vfdStderr = FDLRNewRandomizedDescriptor(STDERR_FILENO, FDLRTable::kSystem, nullptr);

    self.set("VFD_STDIN", vfdStdin)
        .set("VFD_STDOUT", vfdStdout)
        .set("VFD_STDERR", vfdStderr);

    SET_CONST_ENUM("MODE_NONE", vfs::Mode::kNone);
    SET_CONST_ENUM("MODE_USR_W", vfs::Mode::kUsrW);
    SET_CONST_ENUM("MODE_USR_R", vfs::Mode::kUsrR);
    SET_CONST_ENUM("MODE_USR_X", vfs::Mode::kUsrX);
    SET_CONST_ENUM("MODE_OTH_W", vfs::Mode::kOthW);
    SET_CONST_ENUM("MODE_OTH_R", vfs::Mode::kOthR);
    SET_CONST_ENUM("MODE_OTH_X", vfs::Mode::kOthX);
    SET_CONST_ENUM("MODE_GRP_R", vfs::Mode::kGrpR);
    SET_CONST_ENUM("MODE_GRP_W", vfs::Mode::kGrpW);
    SET_CONST_ENUM("MODE_GRP_X", vfs::Mode::kGrpX);
    SET_CONST_ENUM("MODE_DIR", vfs::Mode::kDir);
    SET_CONST_ENUM("MODE_LINK", vfs::Mode::kLink);
    SET_CONST_ENUM("MODE_REGULAR", vfs::Mode::kRegular);
    SET_CONST_ENUM("MODE_CHAR", vfs::Mode::kChar);
    SET_CONST_ENUM("MODE_BLOCK", vfs::Mode::kBlock);
    SET_CONST_ENUM("MODE_FIFO", vfs::Mode::kFifo);
    SET_CONST_ENUM("MODE_SOCKET", vfs::Mode::kSocket);

    self.set("print", jni_core_print)
        .set("delay", jni_core_delay)
        .set("getProperty", jni_core_getProperty)
        .set("hasProperty", jni_core_hasProperty)
        .set("Timer", jni_core_Timer_class)
        .set("open", jni_core_open)
        .set("close", jni_core_close)
        .set("dump", jni_core_dump)
        .set("exit", jni_core_exit);
}

KOI_LANG_NS_END
