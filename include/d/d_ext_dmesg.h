#ifndef D_EXT_DMESG_H
#define D_EXT_DMESG_H

// ============================================================================
// §308 NATIVE WW dMesg — BRING-UP (M1: boot residency).
//
// Retires the §201/§193 demo-message reconstruction (TP dMsgObject + WW-Crew
// catalog + hand-rolled step-in-step) in favour of the donor's own dMesg
// subsystem, for WW host stages ONLY (dExtWwSave_isWwHostStage). TP areas keep
// dMsgObject untouched.
//
// Foundry §308 decode (docs/WW Linked/dmsg-native-decode.md) staged six donor
// archives to model_replacements/WW-Crew-Restoration/arcs/ (mounted by the R2
// overlay as res/Object/<name>.arc):
//   dmsgres.arc  — hukidashi_d00/_d09.blo box layouts + 5 BTIs
//   bmgres.arc   — zel_00.bmg (ALL 4,411 text entries)
//   bmgresh.arc  — zel_01.bmg (Hylian variants; must be resident even if unused)
//   fontres.arc  — rock_24_20_4i_usa.bfn (main text font)         [M1b]
//   rubyres.arc  — hyrule.bfn (US "ruby"; dMsg_Create asserts it) [M1b]
//   menures.arc  — outfont A-prompt BTIs + key.bti placeholder    (global search)
//
// IMPORTANT: these are held in MODULE-STATIC slots, NOT the game-info archive
// fields. The port's dComIfGp_getMsgDtArchive is TP's text and MUST NOT be
// clobbered; the ported dMesg reads the dExtDmesg_get* accessors below instead.
// (No game-info struct field either — offset-stable law.)
// ============================================================================

class JKRArchive;
class JUTFont;

// M1: make all six archives resident + latch them into the module slots.
// Idempotent; returns true once everything is resident. Call each frame on a WW
// host stage until it returns true (mirrors §297b residency).
bool dExtDmesg_ensureResident();

// Donor-shaped archive accessors the ported dMesg reads (parallel to — never
// replacing — the port's dComIfGp_getMsgDtArchive). NULL until resident.
JKRArchive* dExtDmesg_getDmsgArchive();   // dmsgres — hukidashi_d00/_d09.blo + BTIs
JKRArchive* dExtDmesg_getMsgArchive();    // bmgres  — zel_00.bmg (text)
JKRArchive* dExtDmesg_getMsgHArchive();   // bmgresh — zel_01.bmg (Hylian)
JKRArchive* dExtDmesg_getMenuArchive();   // menures — outfont BTIs (global search)

// The two WW fonts (main text + ruby) [M1b]. NULL until loaded. dMsg_Create
// requires BOTH non-NULL even though the tale never renders ruby (§308 decode §5).
JUTFont* dExtDmesg_getFont();
JUTFont* dExtDmesg_getRFont();

// M2 — resolve a zel_00.bmg message id → its DAT1 text bytes (NULL if unresolved).
// The STB setMessageCode value is this id (Foundry §308: 539→INF1 idx 2001, etc.).
const char* dExtDmesg_getMessageById(unsigned short id);

// M3/M4b — draw the donor hukidashi_d00 box. Call from the 2D draw pass on WW host
// stages. M4b: lifecycle-driven (draws only while a message beat is presenting), no
// longer an always-on overlay.
void dExtDmesg_drawTestBox();

// M4 — the storyboard sets the box's current message (STB setMessageCode value = the
// BMG index). Call from the demo message adaptor on WW host stages. M4b: in Native
// style this SHOWS the box for the beat and reads the donor zel_00.bmg text directly.
void dExtDmesg_setMessage(unsigned short id);

// M4b — per-frame lifecycle: advance/dismiss on A/B and release the STB's per-beat
// suspend(1) (dDemo_c::getControl()->unsuspend(1), the donor's box-close contract).
// Call each frame from the demo poll on WW host stages. No-op unless Native style +
// a beat is presenting.
void dExtDmesg_update();

// §313 — true while the native box is actively presenting a tale beat (holding the STB
// suspend, waiting on A/B). The door arrival-G-guard reads this so it never force-ends a
// legitimately-held cutscene as if it were a stuck arrival residual.
bool dExtDmesg_isBoxActive();

// ============================================================================
// §324 LIVE NPC TALK (generalization beyond the STB path). Contract:
//   * fopNpc_npc_c::talk() drives the TP dMsgObject lifecycle and polls its
//     mode (MSG_DISPLAYED → chain decision, BOX_CLOSED → done) — every WW
//     actor's flow logic stays untouched.
//   * In Native style the native box owns PRESENTATION + INPUT: dMsgObject's
//     isSend() (the single input funnel) returns 0 while the native box holds,
//     and one synthetic "A" (2) on the frame after final-page dismissal — so
//     the TP state machine advances at native pace with its visuals suppressed.
//   * Selects are NOT ported yet: if the TP object enters a select state the
//     native box cancels and hands presentation back to the TP box (debt).
// ============================================================================

// Open the native box for a live talk line. i_id = the WW BMG message id the
// actor passed to fopMsgM_messageSet; text = zel_00.bmg by id, else
// i_fallbackText (the §256 catalog line). False ⇒ caller stays Reconstructed.
bool dExtDmesg_openTalk(unsigned short i_id, const char* i_fallbackText);

// True while the native TALK box is presenting (suppresses TP input + draw).
bool dExtDmesg_isTalkActive();

// True only on the one-frame window after the final page was dismissed — the
// synthetic "A" isSend() feeds the TP state machine.
bool dExtDmesg_isTalkReleaseFrame();

// §324 chain: true within the short window after a release during which a
// CONTINUE-chained fopMsgM_messageSet may re-open the native box.
bool dExtDmesg_isTalkChainWindow();

// §227: re-arm the chain gap window when a chained line is accepted, so one
// conversation stays on the native box for its full length (the window then
// only bounds the PAUSE between consecutive lines, not the conversation).
void dExtDmesg_rearmTalkChain();

// Select handback / abort: drop the native talk box with no release.
void dExtDmesg_cancelTalk(const char* i_why);

#endif /* D_EXT_DMESG_H */
