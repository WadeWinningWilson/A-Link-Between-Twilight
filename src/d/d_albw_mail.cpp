// ============================================
// NEW CODE — ALBW Port — Junior Postman onboarding mail (Phase 0).
// ============================================
#include "d/d_albw_mail.h"

#if TARGET_PC

#include "d/actor/d_a_npc_post.h"
#include "d/actor/d_a_player.h"
#include "d/d_com_inf_game.h"
#include "d/d_meter2_info.h"
#include "d/d_msg_class.h"
#include "d/d_msg_string_base.h"
#include "d/d_save.h"
#include "dusk/settings.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_name.h"
#include "m_Do/m_Do_ext.h"
#if TARGET_PC_NATIVE_UI
#include "d/d_albw_ui_text.h"
#endif
#include <cstdio>
#include <cstring>

namespace {

// ============================================
// STORY GATE FIX (alpha cleanup): was F_0601 "Spoke to IMPRISONED Talo" —
// the END of the chase (and dialogue-dependent), so a fresh game entering
// North Faron mid-quest never armed the encounter. Intended trigger is the
// player's FIRST visit to the area DURING the save-Talo questline, so gate
// on F_0094 "Talo went after the monkey" — the quest-start flag, set
// before Link ever enters the woods.
// ============================================
constexpr u16 kTaloQuestStartEventIndex = 94;  // F_0094 — Talo went after the monkey

static bool sNorthFaronActorsSpawned = false;
static int sNorthFaronLastStayRoom = -1;
static bool sDeliverInProgress = false;
static bool sDeliverCooldown = false;

static const cXyz kNorthFaronMailPos = { -35379.7070f, 300.0000f, -15932.8701f };

bool isDeliveredSaved() {
    return dComIfGs_isEventBit(dSv_event_flag_c::saveBitLabels[kAlbwMailDeliveredEventIndex]);
}

void setDeliveredSaved(bool delivered) {
    if (delivered) {
        dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[kAlbwMailDeliveredEventIndex]);
    } else {
        dComIfGs_offEventBit(dSv_event_flag_c::saveBitLabels[kAlbwMailDeliveredEventIndex]);
    }
}

void setPendingSaved(bool pending) {
    if (pending) {
        dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[kAlbwMailPendingEventIndex]);
    } else {
        dComIfGs_offEventBit(dSv_event_flag_c::saveBitLabels[kAlbwMailPendingEventIndex]);
    }
}

bool storyGatePassed() {
    if (dAlbwMail_isTestMode()) {
        return true;
    }
    return dComIfGs_isEventBit(dSv_event_flag_c::saveBitLabels[kTaloQuestStartEventIndex]);
}

bool deliveredGateOpen() {
    if (dAlbwMail_isTestMode()) {
        return true;
    }
    return !isDeliveredSaved();
}

const char* kDeliverLines[] = {
    "HEEEYYYYYY!!!",
    "Nearly lost you in these woods!",
    "No time for introductions!",
    "Onward to mail!",
};

// Vanilla postman dialogue uses flow 0x1324 with ~3 visible lines per page; explicit breaks
// keep words intact instead of hard character clipping at the textbox edge.
const char* kDeliverSpeech =
    "HEEEYYYYYY!!!\n\n"
    "Nearly lost you in these woods!\n"
    "No time for introductions!\n\n"
    "Onward to mail!";

struct AlbwMailLetterDef {
    int index;
    const char* subject;
    const char* sender;
    const char* body;
};

const AlbwMailLetterDef kLetters[] = {
    {
        kAlbwMailLetterIndex,
        "New Delivery Services Coming Near You!",
        "Junior Postman",
        "Tired on your travels? Ever rested so deeply that you woke up to notice all your "
        "belongings missing? Never fear! Hyrulian Post Services are there for you! Taking great "
        "care to pick up any lost belongings, we safely store them at one of our Junior Postman "
        "locations who will return them to you with the most reasonable of prices.\n\n"
        " ~Ordon location coming soon ~",
    },
    {
        kAlbwMailLetterIndex2,
        "Hyrulian Delivery Services Announcement",
        "Junior Postman",
        "As you carry forward on your steps throughout Hyrule, you're bound to encounter scrolls "
        "that must be treated with the utmost care. As our talented Junior Postman employees "
        "know: a torn or folded scroll pays great disrespect to the knowledge of old! Inspect "
        "your scrolls carefully for any wear or tear. Be kind, please reseal!",
    },
};

constexpr int kLetterCount = static_cast<int>(sizeof(kLetters) / sizeof(kLetters[0]));

const AlbwMailLetterDef* findLetterDef(int letterIndex) {
    for (int i = 0; i < kLetterCount; i++) {
        if (kLetters[i].index == letterIndex) {
            return &kLetters[i];
        }
    }
    return NULL;
}

constexpr int kLetterPageBufSize = 0x200;
constexpr int kLetterPageCap = D_MSG_CLASS_PAGE_CNT_MAX;
static char sLetterPages[kLetterPageCap][kLetterPageBufSize];
static int sLetterPageCount = 0;
static int sLetterBuiltLineMax = 0;
static int sLetterBuiltIndex = -1;

f32 getLetterWrapWidth(J2DTextBox* bodyBox) {
    if (bodyBox != NULL) {
        return bodyBox->getWidth() - 8.0f;
    }
    return 400.0f;
}

void getLetterFontMetrics(J2DTextBox* bodyBox, JUTFont** font, f32* fontSizeX, f32* charSpace) {
    if (bodyBox != NULL) {
        *font = bodyBox->getFont();
        J2DTextBox::TFontSize fontSize;
        bodyBox->getFontSize(fontSize);
        *fontSizeX = fontSize.mSizeX;
        *charSpace = bodyBox->getCharSpace();
        return;
    }

    *font = mDoExt_getMesgFont();
    *fontSizeX = 24.0f;
    *charSpace = 0.0f;
}

bool appendLineToPage(int& pageIndex, const char* line, size_t lineLen) {
    if (pageIndex < 0 || pageIndex >= kLetterPageCap || line == NULL) {
        return false;
    }

    if (lineLen >= kLetterPageBufSize) {
        lineLen = kLetterPageBufSize - 1;
    }

    char* page = sLetterPages[pageIndex];
    size_t pageLen = strlen(page);
    const size_t needed = lineLen + (pageLen > 0 ? 1 : 0);
    if (pageLen + needed >= kLetterPageBufSize) {
        if (pageIndex + 1 >= kLetterPageCap) {
            return false;
        }
        pageIndex++;
        sLetterPages[pageIndex][0] = '\0';
        return appendLineToPage(pageIndex, line, lineLen);
    }

    if (pageLen > 0) {
        page[pageLen] = '\n';
        page[pageLen + 1] = '\0';
        pageLen++;
    }

    memcpy(page + pageLen, line, lineLen);
    page[pageLen + lineLen] = '\0';
    return true;
}

void buildLetterPages(int letterIndex, J2DTextBox* bodyBox, int lineMax) {
    const AlbwMailLetterDef* def = findLetterDef(letterIndex);
    if (def == NULL) {
        sLetterPageCount = 0;
        sLetterBuiltIndex = -1;
        return;
    }

    if (lineMax <= 0) {
        lineMax = 12;
    }
    if (sLetterPageCount > 0 && sLetterBuiltLineMax == lineMax && sLetterBuiltIndex == letterIndex) {
        return;
    }

    sLetterPageCount = 0;
    sLetterBuiltLineMax = lineMax;
    sLetterBuiltIndex = letterIndex;
    for (int i = 0; i < kLetterPageCap; i++) {
        sLetterPages[i][0] = '\0';
    }

    char source[2048];
    std::snprintf(source, sizeof(source), "%s\n\n%s", def->subject, def->body);

    char wrapped[4096];
    wrapped[0] = '\0';
    JUTFont* font = NULL;
    f32 fontSizeX = 24.0f;
    f32 charSpace = 0.0f;
    getLetterFontMetrics(bodyBox, &font, &fontSizeX, &charSpace);

#if TARGET_PC_NATIVE_UI
    if (font != NULL) {
        dALBW_wrapMesgWords(wrapped, sizeof(wrapped), source, getLetterWrapWidth(bodyBox), font,
                            fontSizeX, charSpace);
    } else
#endif
    {
        std::strncpy(wrapped, source, sizeof(wrapped) - 1);
        wrapped[sizeof(wrapped) - 1] = '\0';
    }

    int pageIndex = 0;
    int lineInPage = 0;
    const char* lineStart = wrapped;
    for (const char* p = wrapped;; ++p) {
        if (*p != '\n' && *p != '\0') {
            continue;
        }

        appendLineToPage(pageIndex, lineStart, static_cast<size_t>(p - lineStart));
        lineInPage++;

        if (*p == '\0') {
            break;
        }

        lineStart = p + 1;
        if (lineInPage >= lineMax && pageIndex + 1 < kLetterPageCap) {
            pageIndex++;
            lineInPage = 0;
            sLetterPages[pageIndex][0] = '\0';
        }
    }

    sLetterPageCount = pageIndex + 1;
    if (sLetterPageCount <= 0) {
        sLetterPageCount = 1;
    }
}

void setTextBox(J2DTextBox* textBox, const char* text) {
    if (textBox == NULL || text == NULL) {
        return;
    }
    // Match vanilla letter body panes (d_menu_letter.cpp uses 0x200 / 0x210).
    textBox->setString(kLetterPageBufSize, text);
}

bool anyRuntimeLetterMissing() {
    for (int i = 0; i < kLetterCount; i++) {
        if (!dComIfGs_isLetterGetFlag(kLetters[i].index)) {
            return true;
        }
    }
    return false;
}

}  // namespace

void dAlbwMail_init() {
    for (int i = 0; i < kLetterCount; i++) {
        dMenu_LetterData& slot = dMenu_Letter::letter_data[kLetters[i].index];
        slot.mSubject = kAlbwMailRuntimeMsgId;
        slot.mName = kAlbwMailRuntimeMsgId;
        slot.mText = kAlbwMailRuntimeMsgId;
        slot.mEventFlag = kAlbwMailPendingEventIndex;
    }
}

bool dAlbwMail_isTestMode() {
    return dusk::getSettings().game.albwJuniorMailTest.getValue();
}

bool dAlbwMail_isRuntimeLetter(int letterIndex) {
    return findLetterDef(letterIndex) != NULL;
}

bool dAlbwMail_hasReceivedBundle() {
    return !anyRuntimeLetterMissing();
}

bool dAlbwMail_hasTutorialScrolls() {
    // Granted with the Junior Postman North Faron bundle (either letter or delivered bit).
    if (isDeliveredSaved()) {
        return true;
    }
    for (int i = 0; i < kLetterCount; i++) {
        if (dComIfGs_isLetterGetFlag(kLetters[i].index)) {
            return true;
        }
    }
    return false;
}

bool dAlbwMail_isDeliverPostman(const daNpc_Post_c* postman) {
    if (postman == NULL) {
        return false;
    }
    return fopAcM_GetParam(postman) == kAlbwMailDeliverPostParam;
}

bool dAlbwMail_shouldSpawnNorthFaron() {
    if (dAlbwMail_hasReceivedBundle() && !dAlbwMail_isTestMode()) {
        return false;
    }
    return storyGatePassed() && deliveredGateOpen();
}

void dAlbwMail_onNorthFaronSpawn(int roomNo) {
    if (dAlbwMail_isTestMode()) {
        dComIfGs_offSwitch(0x7A, roomNo);
        setDeliveredSaved(false);
    }
    dAlbwMail_tryQueuePending();
}

bool dAlbwMail_canCreateNorthFaronActors(int roomNo) {
    if (!dAlbwMail_shouldSpawnNorthFaron()) {
        return false;
    }
    if (dAlbwMail_isTestMode()) {
        return true;
    }
    return !dComIfGs_isSwitch(0x7A, roomNo);
}

void dAlbwMail_trySpawnNorthFaronActors(int roomNo) {
    if (roomNo != 6 || strcmp(dComIfGp_getStartStageName(), "F_SP108") != 0) {
        return;
    }

    if (!dAlbwMail_canCreateNorthFaronActors(roomNo)) {
        if (dAlbwMail_shouldSpawnNorthFaron()) {
            dAlbwMail_onNorthFaronSpawn(roomNo);
        }
        return;
    }

    if (sNorthFaronActorsSpawned) {
        if (dAlbwMail_shouldSpawnNorthFaron()) {
            dAlbwMail_onNorthFaronSpawn(roomNo);
        }
        return;
    }

    if (dAlbwMail_isTestMode()) {
        setDeliveredSaved(false);
        dComIfGs_offSwitch(0x7A, roomNo);
    }

    static const csXyz kMailPostAngle = { 0, (s16)2775, 0 };
    static const cXyz kMailPostScale = { 1.0f, 1.0f, 1.0f };
    static const cXyz kMailAreaScale = { 2.0f, 1.0f, 2.0f };
    // home.angle.x = 0xFF keeps the type-21 ring always active (onEvt = none).
    static const csXyz kMailAreaAngle = { (s16)0x00FF, 0, 21 };
    static const u32 kMailAreaParam =
        (static_cast<u32>(kAlbwMailDeliveredEventIndex) << 12) | 0x0FFFu;

    OSReport("ALBW Mail: spawning deliver Postman + EvtArea in F_SP108 room 6 "
             "at (%.1f, %.1f, %.1f)\n",
             kNorthFaronMailPos.x, kNorthFaronMailPos.y, kNorthFaronMailPos.z);
    fopAcM_create(fpcNm_TAG_EVTAREA_e, kMailAreaParam, &kNorthFaronMailPos, roomNo,
                  &kMailAreaAngle, &kMailAreaScale, -1);
    fopAcM_create(fpcNm_NPC_POST_e, kAlbwMailDeliverPostParam, &kNorthFaronMailPos, roomNo,
                  &kMailPostAngle, &kMailPostScale, -1);

    sNorthFaronActorsSpawned = true;
    dAlbwMail_onNorthFaronSpawn(roomNo);
}

void dAlbwMail_tickNorthFaron() {
    if (strcmp(dComIfGp_getStartStageName(), "F_SP108") != 0) {
        sNorthFaronLastStayRoom = -1;
        return;
    }

    const int stayRoom = dComIfGp_roomControl_getStayNo();
    if (stayRoom != 6) {
        if (sNorthFaronLastStayRoom == 6) {
            sNorthFaronActorsSpawned = false;
        }
        sNorthFaronLastStayRoom = stayRoom;
        return;
    }

    if (sNorthFaronLastStayRoom != 6) {
        sNorthFaronActorsSpawned = false;
    }
    sNorthFaronLastStayRoom = stayRoom;

    dAlbwMail_trySpawnNorthFaronActors(stayRoom);

    if (!sDeliverInProgress && sDeliverCooldown && daPy_getPlayerActorClass() != NULL) {
        const cXyz playerPos = daPy_getPlayerActorClass()->current.pos;
        const f32 dx = playerPos.x - kNorthFaronMailPos.x;
        const f32 dz = playerPos.z - kNorthFaronMailPos.z;
        if ((dx * dx + dz * dz) > (2500.0f * 2500.0f)) {
            sDeliverCooldown = false;
        }
    }
}

bool dAlbwMail_canTriggerDeliver() {
    if (sDeliverInProgress || sDeliverCooldown) {
        return false;
    }
    return dMeter2Info_getNewLetterNum() > 0;
}

void dAlbwMail_onDeliverCutsceneOrdered() {
    sDeliverInProgress = true;
}

void dAlbwMail_onDeliverCutsceneFinished(bool letterReceived) {
    if (!sDeliverInProgress && !sDeliverCooldown) {
        return;
    }
    sDeliverInProgress = false;
    sDeliverCooldown = true;
    if (letterReceived) {
        dAlbwMail_onDeliveryComplete();
    } else if (dAlbwMail_isTestMode()) {
        setPendingSaved(false);
        setDeliveredSaved(true);
    }
}

const char* dAlbwMail_getDeliverSpeech() {
    return kDeliverSpeech;
}

void dAlbwMail_tryQueuePending() {
    if (!storyGatePassed()) {
        return;
    }
    if (!anyRuntimeLetterMissing()) {
        if (dAlbwMail_isTestMode()) {
            setPendingSaved(true);
            return;
        }
        setPendingSaved(false);
        return;
    }
    if (!deliveredGateOpen()) {
        return;
    }
    setPendingSaved(true);
}

void dAlbwMail_onDeliveryComplete() {
    // receiveLetter() calls this once per runtime slot mid-loop. Finalize only
    // after the full bundle is owned so the shared pending bit stays set for
    // every letter in the same grant pass.
    if (!dAlbwMail_hasReceivedBundle()) {
        return;
    }
    setPendingSaved(false);
    if (!dAlbwMail_isTestMode()) {
        setDeliveredSaved(true);
    }
}

int dAlbwMail_getDeliverLineCount() {
    return static_cast<int>(sizeof(kDeliverLines) / sizeof(kDeliverLines[0]));
}

const char* dAlbwMail_getDeliverLine(int pageIndex) {
    if (pageIndex < 0 || pageIndex >= dAlbwMail_getDeliverLineCount()) {
        return "";
    }
    return kDeliverLines[pageIndex];
}

const char* dAlbwMail_getLetterSubject(int letterIndex) {
    const AlbwMailLetterDef* def = findLetterDef(letterIndex);
    return def != NULL ? def->subject : "";
}

const char* dAlbwMail_getLetterSender(int letterIndex) {
    const AlbwMailLetterDef* def = findLetterDef(letterIndex);
    return def != NULL ? def->sender : "";
}

const char* dAlbwMail_getLetterBodyPage(int letterIndex, int pageIndex) {
    if (sLetterBuiltIndex != letterIndex || pageIndex < 0 || pageIndex >= sLetterPageCount) {
        return "";
    }
    return sLetterPages[pageIndex];
}

int dAlbwMail_getLetterBodyPageCount(int letterIndex) {
    if (sLetterBuiltIndex != letterIndex) {
        return 0;
    }
    return sLetterPageCount;
}

void dAlbwMail_drawLetterSubject(int letterIndex, J2DTextBox* textBox) {
    if (!dAlbwMail_isRuntimeLetter(letterIndex)) {
        return;
    }
    setTextBox(textBox, dAlbwMail_getLetterSubject(letterIndex));
}

void dAlbwMail_drawLetterSender(int letterIndex, J2DTextBox* textBox) {
    if (!dAlbwMail_isRuntimeLetter(letterIndex)) {
        return;
    }
    setTextBox(textBox, dAlbwMail_getLetterSender(letterIndex));
}

void dAlbwMail_drawLetterBodyPage(int letterIndex, int pageIndex, int lineMax, J2DTextBox* bodyBox,
                                  J2DTextBox* rubyBox, dMsgStringBase_c* stringDrawer) {
    (void)rubyBox;
    (void)stringDrawer;
    if (!dAlbwMail_isRuntimeLetter(letterIndex)) {
        return;
    }
    buildLetterPages(letterIndex, bodyBox, lineMax);
    setTextBox(bodyBox, dAlbwMail_getLetterBodyPage(letterIndex, pageIndex));
}

int dAlbwMail_getLetterBodyPageMax(int letterIndex, dMsgStringBase_c* stringDrawer, int lineMax) {
    (void)stringDrawer;
    if (!dAlbwMail_isRuntimeLetter(letterIndex)) {
        return 0;
    }
    if (sLetterBuiltIndex != letterIndex || sLetterPageCount <= 0) {
        buildLetterPages(letterIndex, NULL, lineMax);
    }
    return dAlbwMail_getLetterBodyPageCount(letterIndex);
}

#endif  // TARGET_PC
