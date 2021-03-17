#include <cstring>
#include <fstream>
#include <iostream>

#include "Core/Exception.h"
#include "crpkg/proto.h"
#include "crpkg/INodeTree.h"
#include "crpkg/Serializer.h"
CRPKG_BEGIN_NS

namespace {

void flatten_inode_tree(std::vector<FlatINode::Ptr>& out, INodeBase *inode)
{
    if (inode->kind() == INodeBase::Kind::kFile)
    {
        auto *fileINode = dynamic_cast<INodeFile*>(inode);

        auto flat = std::make_unique<FlatINode>();
        flat->kind = INodeBase::Kind::kFile;
        flat->name = inode->name();
        flat->fCompressor = std::move(fileINode->content().compressor);
        flat->perm = fileINode->content().perm;
        std::memcpy(flat->md5sum, fileINode->content().md5sum, sizeof(flat->md5sum));

        out.push_back(std::move(flat));
        return;
    }

    {
        auto flat = std::make_unique<FlatINode>();
        flat->kind = INodeBase::Kind::kDirectory;
        flat->name = inode->name();
        out.push_back(std::move(flat));
    }

    auto *dir = dynamic_cast<INodeDirectory*>(inode);
    int32_t flatIdx = out.size() - 1;
    for (INodeBase *ptr : *dir)
    {
        out[flatIdx]->childrenIdx.push_back(out.size());
        flatten_inode_tree(out, ptr);
    }
}

struct ProtoINodeWrap
{
    size_t size = 0;
    uint64_t offset_in_section = 0;
    int32_t sym_table_idx = -1;
    union {
        proto::INode *pb = nullptr;
        proto::FileINode *pf;
        proto::DirectoryINode *pd;
    } ptrs;
};

struct ProtoFileDataWrap
{
    ProtoFileDataWrap() = default;
    ProtoFileDataWrap(const ProtoFileDataWrap&) = delete;
    ProtoFileDataWrap(ProtoFileDataWrap&& rhs) noexcept
        : size(rhs.size),
          offset_in_section(rhs.offset_in_section),
          compressor(std::move(rhs.compressor)),
          ptr(rhs.ptr) { rhs.ptr = nullptr; }

    size_t size = 0;
    uint64_t offset_in_section = 0;
    CompressContext::PromiseResult compressor;
    proto::FileData *ptr = nullptr;
};

struct ProtoSymbolWrap
{
    size_t size = 0;
    uint64_t offset_in_section = 0;
    proto::Symbol *ptr = nullptr;
};

int32_t append_proto_sym_wrap(const std::string& sym, std::vector<ProtoSymbolWrap>& wraps)
{
    ProtoSymbolWrap w{};
    w.size = sizeof(proto::Symbol) + sym.size() + 1;
    if (wraps.empty())
        w.offset_in_section = 0;
    else
        w.offset_in_section = wraps.back().offset_in_section + wraps.back().size;
    w.ptr = static_cast<proto::Symbol*>(std::malloc(w.size));
    w.ptr->symbol_size = sym.size() + 1;
    std::strncpy(reinterpret_cast<char *>(w.ptr->symbol), sym.c_str(), sym.size());
    wraps.push_back(w);

    return wraps.size() - 1;
}

void fill_proto_file_inode_wrap(FlatINode::Ptr& flat, ProtoINodeWrap& wrap, std::vector<ProtoSymbolWrap>& symWraps)
{
    wrap.size = sizeof(proto::FileINode);
    wrap.ptrs.pf = static_cast<proto::FileINode*>(std::malloc(wrap.size));

    wrap.ptrs.pb->inode_type = CRPKG_INODE_FILE;
    wrap.ptrs.pf->permission = flat->perm;
    std::memcpy(wrap.ptrs.pf->uncompressed_md5sum,
                flat->md5sum, sizeof(wrap.ptrs.pf->uncompressed_md5sum));
    wrap.sym_table_idx = append_proto_sym_wrap(flat->name, symWraps);
}

void fill_proto_file_directory_inode_wrap(FlatINode::Ptr& flat, ProtoINodeWrap& wrap, std::vector<ProtoSymbolWrap>& symWraps)
{
    wrap.size = sizeof(proto::DirectoryINode) + sizeof(wrap.ptrs.pd->contains_inode_offsets[0]) * flat->childrenIdx.size();
    wrap.ptrs.pd = static_cast<proto::DirectoryINode*>(std::malloc(wrap.size));

    wrap.ptrs.pb->inode_type = CRPKG_INODE_DIRECTORY;
    wrap.ptrs.pd->contains_count = flat->childrenIdx.size();
    wrap.sym_table_idx = append_proto_sym_wrap(flat->name, symWraps);
}

void fill_proto_file_data_wrap(FlatINode::Ptr& flat, ProtoFileDataWrap& wrap)
{
    wrap.size = sizeof(proto::FileData) + flat->fCompressor.compressedSize();
    wrap.ptr = static_cast<proto::FileData*>(std::malloc(wrap.size));

    wrap.compressor = std::move(flat->fCompressor);
    wrap.ptr->magic = CRPKG_FILEDATA_MAGIC;
    wrap.ptr->compressed_size = wrap.compressor.compressedSize();
}

void fill_proto_inode_and_data_wrap(std::vector<FlatINode::Ptr>& flattened,
                                    std::vector<ProtoINodeWrap>& inodeWraps,
                                    std::vector<ProtoFileDataWrap>& dataWraps,
                                    std::vector<ProtoSymbolWrap>& symWraps)
{
    uint64_t inodeSectionSum = 0;
    uint64_t dataSectionSum = 0;

    for (FlatINode::Ptr& flat : flattened)
    {
        ProtoINodeWrap wrap;
        switch (flat->kind)
        {
        case INodeBase::Kind::kFile:
        {
            ProtoFileDataWrap dwrap;
            fill_proto_file_inode_wrap(flat, wrap, symWraps);
            fill_proto_file_data_wrap(flat, dwrap);

            dwrap.offset_in_section = dataSectionSum;
            dataSectionSum += dwrap.size;
            dataWraps.emplace_back(std::move(dwrap));
            break;
        }

        case INodeBase::Kind::kDirectory:
            fill_proto_file_directory_inode_wrap(flat, wrap, symWraps);
            break;
        }

        wrap.offset_in_section = inodeSectionSum;
        inodeSectionSum += wrap.size;
        inodeWraps.push_back(wrap);
    }
}

void relocate_wraps(std::vector<FlatINode::Ptr>& flattened,
                    proto::Header& hdr,
                    std::vector<ProtoSymbolWrap>& symWrap,
                    std::vector<ProtoINodeWrap>& inodeWrap,
                    std::vector<ProtoFileDataWrap>& dataWrap)
{
    hdr.symbol_table_offset = sizeof(proto::Header);
    hdr.symbol_count = symWrap.size();
    hdr.inode_table_offset = hdr.symbol_table_offset + symWrap.back().offset_in_section + symWrap.back().size;
    hdr.inode_count = inodeWrap.size();

    uint32_t dataTableOffset = hdr.inode_table_offset + inodeWrap.back().offset_in_section + inodeWrap.back().size;

    // uint64_t inodeSectionSize = inodeWrap.back().sizeSum + inodeWrap.back().size;

    int32_t dataWrapIdx = 0;
    for (int32_t i = 0; i < inodeWrap.size(); i++)
    {
        if (flattened[i]->kind == INodeBase::Kind::kDirectory)
        {
            auto& childrenIdx = flattened[i]->childrenIdx;
            for (int32_t k = 0; k < childrenIdx.size(); k++)
            {
                inodeWrap[i].ptrs.pd->contains_inode_offsets[k] =
                        inodeWrap[childrenIdx[k]].offset_in_section + hdr.inode_table_offset;
            }
        }
        else
        {
            inodeWrap[i].ptrs.pf->data_offset =
                    dataWrap[dataWrapIdx].offset_in_section + dataTableOffset;
            dataWrapIdx++;
        }

        inodeWrap[i].ptrs.pb->inode_symbol_offset =
                symWrap[inodeWrap[i].sym_table_idx].offset_in_section + hdr.symbol_table_offset;
    }
}

#define STR(p)  static_cast<const char*>(static_cast<const void*>(p))
void write_structure_to_file(const std::string& file,
                             proto::Header& header,
                             std::vector<ProtoSymbolWrap>& sym,
                             std::vector<ProtoINodeWrap>& inodes,
                             std::vector<ProtoFileDataWrap>& datas)
{
    std::ofstream fs(file, std::ios::trunc | std::ios::binary);
    fs.write(STR(&header), sizeof(proto::Header));

    for (ProtoSymbolWrap& w : sym)
        fs.write(STR(w.ptr), w.size);

    for (ProtoINodeWrap& inode : inodes)
        fs.write(STR(inode.ptrs.pb), inode.size);

    for (ProtoFileDataWrap& data : datas)
    {
        fs.write(STR(data.ptr), sizeof(proto::FileData));
        data.compressor.write([&fs](const void *data, size_t size) -> void {
            fs.write(static_cast<const char*>(data), size);
        });

        data.compressor.freeCaches();
    }
}
#undef STR

} // namespace anonymous

Serializer::Serializer(INodeRoot *root)
{
    flatten_inode_tree(fFlattened, root);
}

void Serializer::write(const std::string& file, const std::string& packageName)
{
    // std::ofstream fs(file, std::ios::trunc);
    if (packageName.length() >= CRPKG_HDR_PKGNAME_SIZE)
    {
        throw RuntimeException::Builder(__FUNCTION__)
                .append("crpkg: Package name is too long")
                .make<RuntimeException>();
    }

    /* Prepare and initialize header */
    proto::Header hdr{};
    hdr.major_version = CRPKG_HDR_CURRENT_MAJOR;
    hdr.minor_version = CRPKG_HDR_CURRENT_MINOR;
    constexpr uint8_t magic[] = CRPKG_HDR_MAGIC;
    std::memcpy(hdr.magic, magic, CRPKG_HDR_MAGIC_SIZE);
    std::strncpy(reinterpret_cast<char*>(hdr.package_name),
                 packageName.c_str(), CRPKG_HDR_PKGNAME_SIZE);

    std::vector<ProtoSymbolWrap> protoSymWraps;
    std::vector<ProtoINodeWrap> protoINodeWraps;
    std::vector<ProtoFileDataWrap> protoDataWraps;
    BeforeLeaveScope leave([&protoSymWraps, &protoINodeWraps, &protoDataWraps]() -> void {
        for (ProtoSymbolWrap& w : protoSymWraps)
        {
            if (w.ptr != nullptr)
                std::free(w.ptr);
        }

        for (ProtoINodeWrap& w : protoINodeWraps)
        {
            if (w.ptrs.pb != nullptr)
                std::free(w.ptrs.pb);
        }

        for (ProtoFileDataWrap& w : protoDataWraps)
        {
            if (w.ptr != nullptr)
                std::free(w.ptr);
        }
    });
    fill_proto_inode_and_data_wrap(fFlattened, protoINodeWraps, protoDataWraps, protoSymWraps);
    relocate_wraps(fFlattened, hdr, protoSymWraps, protoINodeWraps, protoDataWraps);

#if 0
    int i = 0;
    for (ProtoINodeWrap& inode : protoWraps)
    {
        std::cout << "inode[" << i << "] " << inode << std::endl;
        i++;
    }

    i = 0;
    for (ProtoFileDataWrap& data : protoDataWraps)
    {
        std::cout << "data[" << i << "] " << data << std::endl;
        i++;
    }
#endif

    write_structure_to_file(file, hdr, protoSymWraps, protoINodeWraps, protoDataWraps);
}

CRPKG_END_NS
