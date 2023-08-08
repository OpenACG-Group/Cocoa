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

#ifndef COCOA_GALLIUM_BINDINGS_RESOURCES_EXPORTS_H
#define COCOA_GALLIUM_BINDINGS_RESOURCES_EXPORTS_H

#include "include/v8.h"
#include "modules/skresources/include/SkResources.h"

#include "Core/Project.h"
#include "Gallium/bindings/ExportableObjectBase.h"
#include "CRPKG/VirtualDisk.h"

#define GALLIUM_BINDINGS_RESOURCES_NS_BEGIN    namespace cocoa::gallium::bindings::resources_wrap {
#define GALLIUM_BINDINGS_RESOURCES_NS_END      }


GALLIUM_BINDINGS_RESOURCES_NS_BEGIN

void SetInstanceProperties(v8::Local<v8::Object> instance);

//! TSDecl: class ResourceProvider
class ResourceProviderWrap : public ExportableObjectBase
{
public:
    explicit ResourceProviderWrap(sk_sp<skresources::ResourceProvider> rp)
            : rp_(std::move(rp)) {}
    ~ResourceProviderWrap() = default;

    //! TSDecl: interface IResourceProvider {
    //!   load(path: string, name: string): Uint8Array;
    //!   loadImageAsset(path: string, name: string, id: string): ImageAsset;
    //!   loadAudioAsset(path: string, name: string, id: string): ExternalTrackAsset;
    //!   loadTypeface(name: string, url: string): GL.CkTypeface;
    //! }

    //! TSDecl: function MakeImpl(impl: IResourceProvider): ResourceProvider
    static v8::Local<v8::Value> MakeImpl(v8::Local<v8::Value> impl);

    //! TSDecl: function MakeFile(baseDir: string, predecode: boolean): ResourceProvider
    static v8::Local<v8::Value> MakeFile(const std::string& base_dir, bool predecode);

    //! TSDecl: interface IResourceProviderProxy {
    //!   load?(path: string, name: string): Uint8Array;
    //!   loadImageAsset?(path: string, name: string, id: string): ImageAsset;
    //!   loadAudioAsset?(path: string, name: string, id: string): ExternalTrackAsset;
    //!   loadTypeface?(name: string, url: string): GL.CkTypeface;
    //! }

    //! TSDecl: function MakeProxyImpl(impl: IResourceProviderProxy): ResourceProvider
    static v8::Local<v8::Value> MakeProxyImpl(v8::Local<v8::Value> impl);

    //! TSDecl: function MakeCachingProxy(rp: ResourceProvider): ResourceProvider
    static v8::Local<v8::Value> MakeCachingProxy(v8::Local<v8::Value> rp);

    //! TSDecl: function MakeDataURIProxy(rp: ResourceProvider, predecode: boolean): ResourceProvider
    static v8::Local<v8::Value> MakeDataURIProxy(v8::Local<v8::Value> rp, bool predecode);

    g_nodiscard g_inline sk_sp<skresources::ResourceProvider> Get() const {
        return rp_;
    }

private:
    sk_sp<skresources::ResourceProvider> rp_;
};

//! TSDecl: class ImageAsset
class ImageAssetWrap : public ExportableObjectBase
{
public:
    explicit ImageAssetWrap(sk_sp<skresources::ImageAsset> asset)
            : asset_(std::move(asset)) {}
    ~ImageAssetWrap() = default;

    //! TSDecl: interface ImageAssetFrame {
    //!   image: GL.CkImage;
    //!   sampling: Enum<GL.Sampling>;
    //!   matrix: GL.CkMatrix;
    //!   scaling: Enum<ImageAssetSizeFit>;
    //! }
    //!
    //! TSDecl: interface IImageAsset {
    //!   isMultiFrame(): boolean;
    //!   getFrameData(t: number): ImageAssetFrame;
    //! }

    //! TSDecl: function MakeMultiFrame(data: Uint8Array, predecode: boolean): ImageAsset
    static v8::Local<v8::Value> MakeMultiFrame(v8::Local<v8::Value> data, bool predecode);

    //! TSDecl: function MakeImpl(impl: IImageAsset): ImageAsset
    static v8::Local<v8::Value> MakeImpl(v8::Local<v8::Value> impl);

    g_nodiscard g_inline sk_sp<skresources::ImageAsset> Get() const {
        return asset_;
    }

private:
    sk_sp<skresources::ImageAsset> asset_;
};

//! TSDecl: class ExternalTrackAsset
class ExternalTrackAssetWrap : public ExportableObjectBase
{
public:
    explicit ExternalTrackAssetWrap(sk_sp<skresources::ExternalTrackAsset> asset)
            : asset_(std::move(asset)) {}
    ~ExternalTrackAssetWrap() = default;

    //! TSDecl: interface IExternalTrackAsset {
    //!   seek(t: number): void;
    //! }

    //! TSDecl: function MakeImpl(impl: IExternalTrackAsset): ExternalTrackAsset
    static v8::Local<v8::Value> MakeImpl(v8::Local<v8::Value> impl);

    g_nodiscard g_inline sk_sp<skresources::ExternalTrackAsset> Get() const {
        return asset_;
    }

private:
    sk_sp<skresources::ExternalTrackAsset> asset_;
};

//! TSDecl: class CRPKGStorage
class CRPKGStorageWrap : public ExportableObjectBase
{
public:
    CRPKGStorageWrap(std::shared_ptr<crpkg::VirtualDisk> disk,
                     const crpkg::VirtualDisk::Storage& storage)
        : disk_(std::move(disk)), storage_(storage) {}
    ~CRPKGStorageWrap() = default;

    //! TSDecl: readonly byteLength: number
    g_nodiscard size_t byteLength() const;

    //! TSDecl: function read(srcOffset: number, dst: Uint8Array,
    //!                       dstOffset: number, size: number): Promise<number>
    g_nodiscard
    v8::Local<v8::Value> read(size_t src_offset, v8::Local<v8::Value> dst,
                              size_t dst_offset, size_t size) const;

    //! TSDecl: function readSync(srcOffset: number, dst: Uint8Array,
    //!                           dstOffset: number, size: number): number
    g_nodiscard
    v8::Local<v8::Value> readSync(size_t src_offset, v8::Local<v8::Value> dst,
                                  size_t dst_offset, size_t size) const;

    //! TSDecl: function unref(): void
    void unref();

    g_nodiscard const crpkg::VirtualDisk::Storage& GetStorage() const {
        return storage_;
    }

    g_nodiscard std::shared_ptr<crpkg::VirtualDisk> GetDisk() const {
        return disk_;
    }

private:
    std::shared_ptr<crpkg::VirtualDisk> disk_;
    crpkg::VirtualDisk::Storage storage_;
};

enum class CRPKGSourceType
{
    kUint8Array,
    kCRPKGStorage,
    kFilePath,

    kLastEnum = kFilePath
};

//! TSDecl: class CRPKGVirtualDisk
class CRPKGVirtualDiskWrap : public ExportableObjectBase
{
public:
    explicit CRPKGVirtualDiskWrap(std::shared_ptr<crpkg::VirtualDisk> disk)
        : disk_(std::move(disk)) {}
    ~CRPKGVirtualDiskWrap() = default;

    //! TSDecl: interface CRPKGSource {
    //!   type: Enum<CRPKGSourceType>;
    //!   source: Uint8Array | CRPKGStorage | string;
    //! }

    //! TSDecl: function MakeLayers(layers: Array<CRPKGSource>): CRPKGVirtualDisk
    static v8::Local<v8::Value> MakeLayers(v8::Local<v8::Value> layers);

    //! TSDecl: function resolve(path: string): CRPKGStorage | null
    v8::Local<v8::Value> resolve(const v8::Local<v8::String>& path);

    //! TSDecl: function unref(): void
    void unref();

private:
    std::shared_ptr<crpkg::VirtualDisk> disk_;
};

GALLIUM_BINDINGS_RESOURCES_NS_END
#endif //COCOA_GALLIUM_BINDINGS_RESOURCES_EXPORTS_H
