#ifndef D_EXT_PLUGIN_WW_STAGE_LOADER_H
#define D_EXT_PLUGIN_WW_STAGE_LOADER_H

struct dStage_dt_c;

inline void dExtWwStage_loadStageDzs(void*, dStage_dt_c*) {}
inline int dExtWw_getLayerNo(int) { return -1; }
inline const char* dExtWw_getEventName(int) { return nullptr; }

#endif
