#ifndef DUSK_BMD_EXPORT_HPP
#define DUSK_BMD_EXPORT_HPP

#include <cstddef>
#include <string>

class J3DModelData;

namespace dusk::bmd_export {

enum class LooseBmdFormat {
    J3d2,
    BmdrBlob,
    Unknown,
};

enum class ExportKind {
    J3d2Raw,
    DiagnosticOnly,
    Failed,
};

struct ExportResult {
    bool ok = false;
    ExportKind kind = ExportKind::Failed;
    std::string message;
};

LooseBmdFormat sniff_loose_bmd(const void* data, std::size_t size);

u32 loader_tag_for_loose(LooseBmdFormat fmt);

// Write SuperBMD-ready J3D2bmd3 (PC endian restore applied) when mpRawData carries a
// standard header; always writes a sibling *_export_report.txt with live model stats.
ExportResult dump_model_data(J3DModelData* model, const char* out_bmd_path);

// Uses dComIfG_getObjectRes — arc must already be mounted (e.g. Skull Kid on-screen).
ExportResult dump_object_res_bmd(const char* arc_name, int res_index, const char* out_bmd_path);

}  // namespace dusk::bmd_export

#endif  // DUSK_BMD_EXPORT_HPP
