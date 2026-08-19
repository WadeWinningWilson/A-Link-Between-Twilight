#pragma once

// Plugin-hosted WW cumulus (§417). Donor wether_move_vrkumo + History's
// decoded vrkumo_move / drawVrkumo (fork d_kankyo_ww_vrkumo.inc).
//
// KIT-LINEAGE: native-port
// KIT-DONOR: d/d_kankyo_wether.cpp MatchingFor (wether_move_vrkumo);
//            d/d_kankyo_rain.cpp NonMatching stubs → decode in
//            src/d/d_kankyo_ww_vrkumo.inc (Winditor/binary + noclip REF)
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
//
// Plugin-owned packet. Does not write vanilla mpVrkumoPacket / hide_vrbox /
// onStatus(1). Gate = Pale/Virt live (same as celestial). Textures from the
// loaded Stage arc (donor outdoor cloudtx_0N.bti path).

void wwVrkumo_reset();
void wwVrkumo_move();
void wwVrkumo_drawInto(void* drawBuf);
