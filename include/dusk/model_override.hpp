#ifndef DUSK_MODEL_OVERRIDE_HPP
#define DUSK_MODEL_OVERRIDE_HPP

// ============================================
// NEW CODE — ALBW Port / Dusklight
// Repack-free custom-model override. Loads a loose BMD from the
// <config>/model_replacements/ folder at runtime (mirrors the texture-replacement
// philosophy, but for geometry) so mods can ship a modified model as a plain file
// instead of repacking the .arc. Generic by (arc_name, res_index); currently wired
// at the Armogohma (B_gm) model-load site as the first consumer.
// ============================================

class J3DModelData;

namespace dusk::model_override {

// Returns a finished J3DModelData* loaded from
// <ConfigPath>/model_replacements/<arc_name>_<res_index>.bmd, or nullptr if no
// override file exists or the load fails — the caller then falls back to the arc
// resource. The backing byte buffer is retained for the model's lifetime.
J3DModelData* try_load(const char* arc_name, int res_index);

}  // namespace dusk::model_override

#endif  // DUSK_MODEL_OVERRIDE_HPP
