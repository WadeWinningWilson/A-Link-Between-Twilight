#pragma once

// Plugin-hosted donor vrbox / vrbox2. KIT-DONOR: d_a_vrbox.cpp /
// d_a_vrbox2.cpp MatchingFor. Not a copy of fork d_a_vrbox (mount include).
// Colors come from plugin Virt/Pale (ww_kankyo), never vanilla vrbox_* cols.
//
// KIT-LINEAGE: native-port
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db

struct HookService;

int wwVrbox_bind(const HookService* hook);

int WwVrbox_create(void* self);
int WwVrbox_delete(void* self);
int WwVrbox_execute(void* self);
int WwVrbox_isDelete(void* self);
int WwVrbox_draw(void* self);

int WwVrbox2_create(void* self);
int WwVrbox2_delete(void* self);
int WwVrbox2_execute(void* self);
int WwVrbox2_isDelete(void* self);
int WwVrbox2_draw(void* self);

void wwVrbox_trySpawn(short vrboxIndex, short vrbox2Index);
void wwVrbox_resetSpawn();

// Painter-time dome submit. Actor draw only arms; packets go in at
// dDlst_list_c::drawOpaDrawList for the sky buffer (after daBg).
void wwVrbox_onSkyOpaDraw(void* drawBuf);

// Same 4-step finish as BG (registry wwFinishModelData). Sky BDLs loaded via
// getStageRes skip the getRes consume arm, so vrbox must finish them itself.
void wwFinishParsedModel(void** pParsed, const char* tag);
