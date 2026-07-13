#ifndef D_ALBW_MAIL_H
#define D_ALBW_MAIL_H

// ============================================
// NEW CODE — ALBW Port — Junior Postman onboarding mail (Phase 0).
// See docs/albw-junior-postman-mail-handoff.md.
// ============================================
#if TARGET_PC

#include "dolphin/types.h"

class daNpc_Post_c;
class J2DTextBox;
class dMsgStringBase_c;

static constexpr int kAlbwMailLetterIndex = 16;

// Runtime letter table sentinel (not a real BMG message id).
static constexpr u16 kAlbwMailRuntimeMsgId = 0xFFFE;

// saveBitLabels[] indices — F_0812 / F_0813 (N/A slots in d_save_bit_labels.inc).
static constexpr u16 kAlbwMailPendingEventIndex = 812;
static constexpr u16 kAlbwMailDeliveredEventIndex = 813;

// daNpc_Post_c spawn param: TYPE_DELIVER (low byte 1) + getBitSW 0x7A (high byte).
static constexpr u16 kAlbwMailDeliverPostParam = 0x7A01;

void dAlbwMail_init();

bool dAlbwMail_isTestMode();

bool dAlbwMail_isRuntimeLetter(int letterIndex);

bool dAlbwMail_isDeliverPostman(const daNpc_Post_c* postman);

bool dAlbwMail_shouldSpawnNorthFaron();

void dAlbwMail_onNorthFaronSpawn(int roomNo);

bool dAlbwMail_canCreateNorthFaronActors(int roomNo);

void dAlbwMail_trySpawnNorthFaronActors(int roomNo);

void dAlbwMail_tickNorthFaron();

bool dAlbwMail_canTriggerDeliver();

void dAlbwMail_onDeliverCutsceneOrdered();

void dAlbwMail_onDeliverCutsceneFinished(bool letterReceived);

const char* dAlbwMail_getDeliverSpeech();

void dAlbwMail_tryQueuePending();

void dAlbwMail_onDeliveryComplete();

int dAlbwMail_getDeliverLineCount();

const char* dAlbwMail_getDeliverLine(int pageIndex);

const char* dAlbwMail_getLetterSubject();

const char* dAlbwMail_getLetterSender();

const char* dAlbwMail_getLetterBodyPage(int pageIndex);

int dAlbwMail_getLetterBodyPageCount();

void dAlbwMail_drawLetterSubject(int letterIndex, J2DTextBox* textBox);

void dAlbwMail_drawLetterSender(int letterIndex, J2DTextBox* textBox);

void dAlbwMail_drawLetterBodyPage(int letterIndex, int pageIndex, int lineMax, J2DTextBox* bodyBox,
                                  J2DTextBox* rubyBox, dMsgStringBase_c* stringDrawer);

int dAlbwMail_getLetterBodyPageMax(int letterIndex, dMsgStringBase_c* stringDrawer, int lineMax);

#endif  // TARGET_PC

#endif /* D_ALBW_MAIL_H */
