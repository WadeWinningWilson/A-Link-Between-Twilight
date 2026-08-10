// ============================================================================
// ww_cam_crawl.h — the donor CRAWL camera algorithm, as a parallel stack.
//
// KIT-LINEAGE: host-plumbing
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
//
// WHY THIS EXISTS
// §668 measured the receiver's algorithm enum to be the donor's MINUS
// {HUNG, CRAWL} -- twelve mappings derived from shared style 4CCs, then
// confirmed outright by `dCamera_c::engine_tbl` (d_camera.cpp:201), which
// states the same twelve in the same order and continues into eight
// TP-specific algorithms that have no donor counterpart.
//
// The consequence is narrow and concrete: the donor camera TYPE `Subject` is
// defined by seven styles, and two of them (LN17, CC02) select CRAWL. Without
// that algorithm the type cannot be honoured, so the donor's room cameras fall
// back to receiver behaviour -- which is what the player sees as "the camera
// stops following Link once he climbs out of the water".
//
// WHY IT IS NOT IN d_camera.cpp
// A receiver TU that named a donor symbol would be a LEG, and the separability
// gate exists to keep the host linking with the whole WW layer removed. So the
// algorithm lives here, and the receiver reaches it through a NULL-default
// extension point that names no donor (`dCamera_setExtraEngineHook`).
// ============================================================================
#ifndef D_EXT_PLUGIN_WW_CAM_CRAWL_H
#define D_EXT_PLUGIN_WW_CAM_CRAWL_H

// The engine index this layer claims. The receiver's own table occupies
// 0..19; anything at or beyond its end routes to the extra-engine hook.
#define WW_CAM_ALG_CRAWL 20

// Installs the crawl algorithm on the receiver's extra-engine hook. Safe to
// call more than once. With the WW layer excluded this is never called and
// the hook stays NULL, which is exactly the pre-seam behaviour.
void dExtWwCam_installCrawl();

#endif
