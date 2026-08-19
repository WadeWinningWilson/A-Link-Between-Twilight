#pragma once

// Plugin-hosted donor camera style/type tables. KIT-DONOR: d_cam_style.cpp /
// d_cam_type.cpp / d_cam_type2.cpp. Generated blob in ww_cam_data.inc.
//
// Vanilla has no dCamera_setSelectHook / extra-engine hook. The selector
// installs through nextType/nextMode/onStyleChange hooks; CRAWL occupies
// test2Camera (engine 19) because Algorythmn is an inline field read.

struct dExtWwCamType {
    char name[24];
    short styles[20];
};

const void* dExtWwCam_styleDatBlob(int* o_byteSize);
const dExtWwCamType* dExtWwCam_types(int* o_count);
const void* dExtWwCam_typeShadowBlob(int* o_byteSize);
int dExtWwCam_styleCount(void);
const char* const* dExtWwCam_bgTypeNames(int* o_count);
void dExtWwCam_installData(void);
