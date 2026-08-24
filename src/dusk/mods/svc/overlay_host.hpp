#pragma once

// Host-side entry for the dusk-API Overlay service. custom_assets Layer A and
// .dusk package overlays must share ONE aurora_dvd_overlay_files push — Aurora
// replaces the entire set on each call, so either side pushing alone wipes the
// other (clothes RARCs like Kmdl/Bmdl then mount vanilla forever).

namespace dusk::mods::svc {

// Merge Layer A (model_replacements) + active .dusk overlays and push to Aurora.
// Safe before ModLoader::init() (mods half empty). Re-runnable.
void overlay_sync_files();

}  // namespace dusk::mods::svc
