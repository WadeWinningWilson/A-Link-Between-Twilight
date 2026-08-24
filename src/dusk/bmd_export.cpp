#include "dusk/bmd_export.hpp"

#if TARGET_PC

#include "dusk/bmd_endian_restore.hpp"
#include "JSystem/J3DGraphAnimator/J3DModelData.h"
#include "JSystem/J3DGraphLoader/J3DModelLoader.h"
#include "JSystem/J3DGraphBase/J3DTexture.h"
#include "JSystem/JUtility/JUTTexture.h"
#include "JSystem/JUtility/JUTNameTab.h"
#include "d/d_com_inf_game.h"
#include "dusk/logging.h"

#include <fmt/format.h>

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace dusk::bmd_export {
namespace {

constexpr u32 kMagicBmd2 = 'bmd2';
constexpr u32 kMagicBmd3 = 'bmd3';

bool is_j3d2_bmd(const void* data, std::size_t size) {
    if (data == nullptr || size < 8) {
        return false;
    }
    // mpRawData stores file bytes; use J3DModelFileData so BE<> fields byte-swap on PC.
    const J3DModelFileData* header = static_cast<const J3DModelFileData*>(data);
    const u32 magic1 = header->mMagic1;
    const u32 magic2 = header->mMagic2;
    return magic1 == 'J3D2' && (magic2 == 'bmd2' || magic2 == 'bmd3');
}

std::size_t j3d2_file_size(const void* data, std::size_t fallback) {
    if (data == nullptr || fallback < sizeof(J3DModelFileData)) {
        return fallback;
    }
    const J3DModelFileData* header = static_cast<const J3DModelFileData*>(data);
    const u8* bytes = static_cast<const u8*>(data);
    const u32 size = (u32(bytes[8]) << 24) | (u32(bytes[9]) << 16) | (u32(bytes[10]) << 8) |
                     u32(bytes[11]);
    if (size >= 0x20 && size <= fallback) {
        return size;
    }
    return fallback;
}

std::filesystem::path report_path_for(const std::filesystem::path& bmd_path) {
    return bmd_path.parent_path() / (bmd_path.stem().string() + "_export_report.txt");
}

void append_model_report(std::string& report, J3DModelData* model) {
    report += fmt::format("modelDataType: {}\n", model->getModelDataType());
    report += fmt::format("joints: {}\n", model->getJointNum());
    report += fmt::format("shapes: {}\n", model->getShapeNum());
    report += fmt::format("materials: {}\n", model->getMaterialNum());
    report += fmt::format("vertices: {}\n", model->getVtxNum());
    report += fmt::format("normals: {}\n", model->getNrmNum());

    const void* raw = model->getRawData();
    if (raw != nullptr) {
        const J3DModelFileData* header = static_cast<const J3DModelFileData*>(raw);
        report += fmt::format("raw magic: {:08x} {:08x} (J3D2={} bmd={})\n",
                              static_cast<unsigned>(static_cast<u32>(header->mMagic1)),
                              static_cast<unsigned>(static_cast<u32>(header->mMagic2)),
                              static_cast<u32>(header->mMagic1) == static_cast<u32>('J3D2') ? "yes"
                                                                                            : "no",
                              (static_cast<u32>(header->mMagic2) == static_cast<u32>('bmd2') ||
                               static_cast<u32>(header->mMagic2) == static_cast<u32>('bmd3'))
                                  ? "yes"
                                  : "no");
    } else {
        report += "raw magic: (null mpRawData)\n";
    }

    J3DTexture* tex = model->getTexture();
    if (tex != nullptr) {
        report += fmt::format("embedded textures: {}\n", tex->getNum());
        JUTNameTab* names = model->getTextureName();
        for (u16 i = 0; i < tex->getNum(); i++) {
            const char* name = (names != nullptr) ? names->getName(i) : "?";
            ResTIMG* timg = tex->getResTIMG(i);
            if (timg == nullptr) {
                report += fmt::format("  [{}] (null timg)\n", static_cast<int>(i));
                continue;
            }
            report += fmt::format("  [{}] {} {}x{} fmt={}\n", static_cast<int>(i),
                                  name != nullptr ? name : "?", static_cast<int>(timg->width),
                                  static_cast<int>(timg->height), static_cast<int>(timg->format));
        }
    } else {
        report += "embedded textures: 0\n";
    }
}

bool write_file(const std::filesystem::path& path, const void* data, std::size_t size) {
    std::error_code ec;
    std::filesystem::create_directories(path.parent_path(), ec);
    FILE* fp = std::fopen(path.string().c_str(), "wb");
    if (fp == nullptr) {
        return false;
    }
    const std::size_t wrote = std::fwrite(data, 1, size, fp);
    std::fclose(fp);
    return wrote == size;
}

bool dump_textures(J3DModelData* model, const std::filesystem::path& tex_dir,
                   std::string& report) {
    J3DTexture* tex = model->getTexture();
    if (tex == nullptr || tex->getNum() == 0) {
        return true;
    }

    std::error_code ec;
    std::filesystem::create_directories(tex_dir, ec);

    JUTNameTab* names = model->getTextureName();
    int written = 0;
    for (u16 i = 0; i < tex->getNum(); i++) {
        ResTIMG* timg = tex->getResTIMG(i);
        if (timg == nullptr) {
            continue;
        }
        const char* name = (names != nullptr) ? names->getName(i) : nullptr;
        std::string stem = (name != nullptr && name[0] != '\0')
                               ? name
                               : fmt::format("tex_{:02d}", static_cast<int>(i));
        const std::filesystem::path out = tex_dir / (stem + ".bti");
        const u32 img_off = timg->imageOffset;
        const u32 pal_off = timg->paletteOffset;
        u32 end = sizeof(ResTIMG);
        if (img_off > end) {
            end = img_off;
        }
        if (pal_off > end) {
            end = pal_off;
        }
        // Conservative upper bound: header + image/palette tail (exact BTI size unknown without
        // full decode — write from ResTIMG through image pointer tail for reference).
        u8* img_ptr = tex->getImgDataPtr(i);
        if (img_ptr != nullptr) {
            const std::size_t ptr_off =
                static_cast<std::size_t>(img_ptr - reinterpret_cast<u8*>(timg));
            end = static_cast<u32>(ptr_off + 256 * 256 * 4);  // cap — trimmed on write fail
        }
        const std::size_t max_avail = 4 * 1024 * 1024;
        const std::size_t dump_size = std::min<std::size_t>(max_avail, end);
        if (write_file(out, timg, dump_size)) {
            written++;
            report += fmt::format("  wrote {}\n", out.string());
        }
    }
    report += fmt::format("texture dumps: {}\n", written);
    return true;
}

}  // namespace

LooseBmdFormat sniff_loose_bmd(const void* data, std::size_t size) {
    if (data == nullptr || size < 4) {
        return LooseBmdFormat::Unknown;
    }
    if (is_j3d2_bmd(data, size)) {
        return LooseBmdFormat::J3d2;
    }
    const u32 head = *static_cast<const u32*>(data);
    // TP object BMDR baked blobs (E_PM family — not J3D2-wrapped).
    if (head == 0x00220022u || head == 0x02412205u) {
        return LooseBmdFormat::BmdrBlob;
    }
    return LooseBmdFormat::Unknown;
}

u32 loader_tag_for_loose(LooseBmdFormat fmt) {
    switch (fmt) {
    case LooseBmdFormat::J3d2:
        return 'BMDV';
    case LooseBmdFormat::BmdrBlob:
        return 'BMDR';
    default:
        return 'BMDV';
    }
}

ExportResult dump_model_data(J3DModelData* model, const char* out_bmd_path) {
    ExportResult result;
    if (model == nullptr || out_bmd_path == nullptr || out_bmd_path[0] == '\0') {
        result.message = "null model or path";
        return result;
    }

    const std::filesystem::path bmd_path(out_bmd_path);
    std::string report = "dusk bmd_export\n";
    append_model_report(report, model);

    const void* raw = model->getRawData();
    const std::filesystem::path tex_dir = bmd_path.parent_path() / (bmd_path.stem().string() + "_textures");
    dump_textures(model, tex_dir, report);

    if (raw != nullptr && is_j3d2_bmd(raw, 8)) {
        const std::size_t fallback = 16 * 1024 * 1024;
        const std::size_t size = j3d2_file_size(raw, fallback);
        std::vector<u8> export_bytes(static_cast<const u8*>(raw),
                                     static_cast<const u8*>(raw) + size);
        restore_pc_j3d2_for_export(export_bytes.data(), export_bytes.size());
        if (write_file(bmd_path, export_bytes.data(), export_bytes.size())) {
            result.ok = true;
            result.kind = ExportKind::J3d2Raw;
            result.message = fmt::format(
                "Wrote SuperBMD-ready J3D2 bmd ({} bytes) to {}", export_bytes.size(),
                bmd_path.string());
            report += result.message + "\n";
            report += "endian restore: VTX1/JNT1/SHP1 reverted to on-disk BE layout\n";
        } else {
            result.message = "failed to write bmd file";
            report += result.message + "\n";
        }
    } else {
        result.ok = true;
        result.kind = ExportKind::DiagnosticOnly;
        result.message =
            "No J3D2 header in mpRawData — wrote report + texture refs only. "
            "TP BMDR may need a future memory rebuild exporter.";
        report += result.message + "\n";
    }

    const std::filesystem::path report_path = report_path_for(bmd_path);
    std::ofstream report_file(report_path);
    if (report_file) {
        report_file << report;
    }

    DuskLog.info("[bmd_export] {} -> {}", result.message, bmd_path.string());
    return result;
}

ExportResult dump_object_res_bmd(const char* arc_name, int res_index, const char* out_bmd_path) {
    ExportResult result;
    if (arc_name == nullptr || out_bmd_path == nullptr) {
        result.message = "null arc or path";
        return result;
    }

    void* res = dComIfG_getObjectRes(arc_name, res_index);
    if (res == nullptr) {
        result.message =
            fmt::format("{}:{:#x} not resident — load the stage/actor first", arc_name, res_index);
        DuskLog.warn("[bmd_export] {}", result.message);
        return result;
    }

    return dump_model_data(static_cast<J3DModelData*>(res), out_bmd_path);
}

}  // namespace dusk::bmd_export

#else  // !TARGET_PC

namespace dusk::bmd_export {

LooseBmdFormat sniff_loose_bmd(const void*, std::size_t) { return LooseBmdFormat::Unknown; }

u32 loader_tag_for_loose(LooseBmdFormat) { return 'BMDV'; }

ExportResult dump_model_data(J3DModelData*, const char*) { return {}; }

ExportResult dump_object_res_bmd(const char*, int, const char*) { return {}; }

}  // namespace dusk::bmd_export

#endif  // TARGET_PC
