#ifndef COCOA_KOIBASE_H
#define COCOA_KOIBASE_H

#include <memory>

#define KOI_NS_BEGIN   namespace cocoa::koi {
#define KOI_NS_END     }

#define KOI_BINDER_NS_BEGIN     namespace cocoa::koi::binder {
#define KOI_BINDER_NS_END       }

#define KOI_BINDINGS_NS_BEGIN   namespace cocoa::koi::bindings {
#define KOI_BINDINGS_NS_END     }

#define koi_nodiscard       [[nodiscard]]
#define koi_maybe_unsed     [[maybe_unused]]
#define koi_noreturn        [[noreturn]]

template<typename T>
using KoSp = std::shared_ptr<T>;

#endif //COCOA_KOIBASE_H
