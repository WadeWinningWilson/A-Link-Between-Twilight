#pragma once

// Plugin-hosted donor celestial layer (sun / moon / lens / stars) + §417
// cumulus (ww_vrkumo).
// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_kankyo_rain.cpp NonMatching, d/d_kankyo.cpp MatchingFor
//            setSunpos, wether_move_sun / wether_move_star
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
//
// Compiled against vanilla headers. Packets are plugin-owned. Does not write
// vanilla hide_vrbox, does not dComIfGp_onStatus(1). Gate is plugin vrbox
// visibility (Pale/Virt bound), the receiver twin of donor checkStatus(1).
// Draw is invoked immediately inside the proven vrbox sky flush (same GX
// moment as the dome UpdateDL), not deferred via entryImm into OPA. Donor
// places these on the XLU sky list after OPA sky; the plugin flush has no
// separate XLU sky pass wired, so immediate+Z-off is the consume-boundary.

void wwSky_reset();
void wwSky_move();
void wwSky_drawInto(void* drawBuf);
// Open when outdoor plugin sky host is live (after room BG bring-up).
void wwSky_openAlwaysBoot();
bool wwSky_bootWindowOpen();
// Advance WwAlways residency (no-op until openAlwaysBoot).
void wwSky_pollAlways();
// True once sky's WwAlways phase has completed (plugin Always residency).
bool wwSky_alwaysReady();
