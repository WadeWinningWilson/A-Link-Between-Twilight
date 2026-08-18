#pragma once

// ============================================================================
// WW MESSAGE READER — the BMG side of the WW dialogue port (CALLS §837).
//
// SCOPE OF THIS FILE, STATED SO IT IS NOT OVER-READ: this reads the DONOR'S
// OWN BMG CONTAINER and resolves a message index to its text and presentation
// record. It renders nothing, hooks nothing and owns no UI. The screen/font
// layers (W2/W3) and the process manager (W4) are separate rows.
//
// §801 DECLARATION, CARRIED IN THE CODE AND NOT ONLY IN THE QUEUE ROW: the WW
// message corpus is 7 of 10 TUs NONMATCHING, so a verbatim port is impossible.
// Everything in this file comes from ONE of two places and each is labelled at
// its use site:
//   · MEASURED — layout confirmed against the user's own disc bytes by
//     History/Bridge (INF1 table at 0x10, entrySize 0x18 read from the file
//     itself, header size field in 32-BYTE UNITS, groupID-per-archive).
//     These are facts about the data and are safe to build on.
//   · MATCHED SOURCE — `JMessage/data.h` + `control.cpp`, which ARE matching.
// NOTHING here is taken from a NONMATCHING body. Where donor BEHAVIOUR is
// unknown, this file refuses rather than reconstructs, and says so in the log.
//
// THE TRAPS THIS FILE EXISTS TO NOT FALL INTO (all six were measured, and each
// one fails SILENTLY rather than loudly — that is why they are listed here
// rather than discovered later):
//   1. INF1's entry table is at **0x10**, not 0x14. The NONMATCHING
//      `mesg_info` says 0x14 and is wrong; four bytes shifts every entry and
//      resolves strings to garbage with no error.
//   2. The header size field at **+0x08 is in 32-BYTE UNITS**, not bytes
//      (`0x4E0B * 32 = 639328`, exact). Bounds-check it as a byte count and
//      nearly every valid message is rejected.
//   3. The entry stride is **DATA** (`messageEntrySize`), never a constant.
//      4. Text is **MULTIBYTE** and the `encoding` field CANNOT choose a
//      decoder: both donor BMGs declare `encoding == 1` while one is
//      single-byte English (1.0% high bytes) and the other is 56.0% high
//      bytes. **Do NOT read that second figure as "Japanese" — I did, and it
//      is WW's HYLIAN-SCRIPT NEW GAME+ set.** High-byte density proves the
//      text is not ASCII; it does not say which non-ASCII it is. The trap
//      stands either way (an ASCII assumption corrupts both), but the REASON
//      matters: `encoding` is useless AND byte statistics do not substitute
//      for it.
//   5. `groupID` selects an ARCHIVE, not a table inside one.
//   6. There are THREE code packings in this system; the tag group is masked
//      to 8 bits while a message groupID is a full u16.
//   7. **A WW MESSAGE IS NOT A C STRING.** Tag payloads contain 0x00 bytes —
//      the colour tag is `1A 06 FF 00 <idx>`, and colour is 58.9% of the
//      8,888 tags in the donor corpus. Splitting on NUL truncates the message
//      at its first colour tag. Use `textLen`, never `strlen`. (Measured
//      after this reader was first written, and it had this bug.)
// ============================================================================

#include <mods/api.h>

#include <cstdint>

// One decoded entry. Field names and offsets are the MEASURED `JMSMesgEntry_c`
// (0x18 bytes, stride confirmed against the disc). Presentation is DATA: the
// donor stores how a message is shown, so a faithful reader gets WW-correct
// presentation without reimplementing the logic that chose it.
struct WwMessageEntry {
    uint32_t dataOffset;   // +0x00 — offset into DAT1. NOT text.
    uint16_t nextMsgNo;    // chaining is DATA, not control flow
    int16_t  itemPrice;    // signed
    // NOT the .blo filename number (measured negative). ONE VALUE IS NOW
    // NAMED: **12 == the HYLIAN-SCRIPT marker** (History/Bridge 2026-08-16,
    // closed three ways against the user's own disc — donor
    // `fopMsgM_hyrule_language_check` names 15 msgNos, `zel_01.bmg`'s INF1
    // holds 15 entries, and `zel_00` names the same 15; all three sets equal).
    // So this field selects a SCRIPT, not only a box shape — which is why
    // `zel_01` is a NEW GAME+ mechanic and not the "leftover Japanese" I
    // wrongly called it. A consumer that ignores 12 renders NG+ text in the
    // ordinary font and loses the mechanic silently.
    uint8_t  textboxType;
    uint8_t  drawType;
    uint8_t  textboxPosition;
    uint8_t  itemImage;
    uint8_t  textAlignment;
    uint8_t  initialSound;
    uint8_t  initialCamera;
    uint8_t  initialAnimation;
    const char* text;      // into the mounted buffer; NOT owned, NOT copied
    // LENGTH IS MANDATORY, NOT A CONVENIENCE — see trap 7. `text` is NOT a C
    // string: WW tag payloads contain 0x00 (the colour tag is `1A 06 FF 00
    // <idx>`, 58.9% of the corpus), so anything that stops at the first NUL
    // truncates the message. ALWAYS pair `text` with `textLen`; never strlen it.
    uint32_t    textLen;
};

// ============================================================================
// GENERAL RARC MEMBER LOCATOR — exposed 2026-08-16 because a SECOND consumer
// appeared outside the message system (the warp menu's parentage pass needs
// the `.dzr` out of a room arc to read its `SCLS` exit table).
//
// IT LIVES HERE FOR ONE REASON AND IT IS NOT TIDINESS: this is the estate's
// only tested RARC walk, ported from `tools/foundry/ww_disc.py:126` and
// already carrying six named refusal paths and full bounds checking. This
// file's own header says re-deriving that arithmetic would be a THIRD
// implementation of something already right twice — so a caller in another
// module gets THIS one rather than its own copy. The `wwMessage_` prefix is
// now a misnomer for this single function; that is a naming debt, and it is a
// cheaper one than a duplicated offset table.
//
// Returns a pointer INTO `arc` (never a copy, never owned) and writes the
// member length to `outLen`. NULL on any refusal, each logged by name.
// ============================================================================
const void* wwMessage_rarcFind(const void* arc, uint32_t size, const char* member,
                               uint32_t* outLen);

// Locate a member inside a RARC archive and open it as a BMG.
//
// OFFSET MATH IS PORTED FROM THE ESTATE'S OWN PROVEN READER, NOT DERIVED:
// `tools/foundry/ww_disc.py:126 rarc_list()`, which is the walker History used
// to find `zel_00.bmg` in the first place and which carries its own note that
// the math matches `gen_cam_data.py:160-170`. Re-deriving it would be a third
// implementation of arithmetic that is already right twice.
//
// YAZ0 IS REFUSED, NOT DECOMPRESSED. The donor `bmgres.arc` is uncompressed
// (measured), so a decompressor here would be untested code on a path nothing
// exercises. If a compressed archive ever arrives this says so by name instead
// of reading Yaz0 bytes as RARC and reporting a corrupt table.
bool wwMessage_openFromArc(const void* arc, uint32_t size, const char* member);

// Parse a BMG image already in memory. `bytes`/`size` must outlive every
// `WwMessageEntry.text` handed out — the reader NEVER copies string data, for
// the same reason the engine does not: the archive is the storage.
// Returns false and logs a NAMED reason on any refusal; never partially
// succeeds, because a half-parsed table resolves to plausible garbage.
bool wwMessage_open(const void* bytes, uint32_t size);

// Resolve by index within the opened archive. False if unopened or out of
// range — an out-of-range index is a caller bug and must not silently clamp.
//
// ⚠ `index` IS A RAW INF1 ROW NUMBER — 0..count-1. IT IS NOT A PACKED CODE,
// AND THIS IS THE TRAP THAT EATS CALLERS (trap 3, four addressing schemes in
// one system):
//     engine   (groupID << 16) | messageIndex
//     WW game  (groupID <<  8) | mResMsgNo
//     tag      (group8  << 16) | code16
//     ID       via a MID1 block
// **Never hand a `getMesgNumber()` result to this function.** Unpack first.
// Passing a packed value yields a valid-looking row from somewhere else in the
// table, which is the worst possible failure: no error, plausible text.
//
// MEASURED ABOUT THIS CORPUS, which removes one of the four from play here:
// **there is NO MID1 block in either donor BMG** — `zel_00.bmg` and
// `zel_01.bmg` are both INF1+DAT1 only. ID-based addressing is therefore not
// reachable in this data, and row index is the sole selection path.
//
// AND `groupID` IS PER-ARCHIVE, confirmed on the files (trap 5):
// `bmgres.arc/zel_00.bmg` is **groupID 0**, 4,411 entries, English single-byte.
// `bmgresh.arc/zel_01.bmg` is **groupID 1**, 15 entries, zero tags — and it
// is WW's **HYLIAN-SCRIPT NEW GAME+** set, NOT a Japanese language table.
// I called it Shift-JIS from high-byte density alone; non-ASCII bytes are
// equally consistent with a substituted script, and the discriminator was
// already in my own output: a parallel LANGUAGE table would carry a
// comparable tag load, and this one carries ZERO against zel_00's 8,888.
// Two archives, two groupIDs, one BMG each — the group does NOT select a table
// inside an archive, so this reader holds exactly one archive at a time.
bool wwMessage_get(uint32_t index, WwMessageEntry* out);

// ============================================================================
// THE DONOR'S OWN COLOUR TABLE — `color.bmc` (`MGCLbmc1`), the SECOND member of
// `bmgres.arc` beside `zel_00.bmg`. This is a PORT OF DONOR DATA, not a new
// system: the archive is already served from the user's ISO, so this is the
// consumer the estate was missing, and nothing new is mounted.
//
// WHY IT IS A DELIVERABLE AND NOT A DETAIL (measured this session):
// `d_ext_dmesg.cpp:313` carries a HARDCODED `kWwColor[9]`, and **7 of its 9
// entries DISAGREE with the donor's own file** — only white (0) and blue (3)
// match. Colour is **58.9% of all 8,888 tags in the corpus**, so the majority
// of WW coloured text currently renders in invented colours. A hand-written
// approximation of shipped donor data is exactly what zero-bake forbids.
//
// FORMAT, measured against the file itself:
//   · magic `MGCLbmc1`; size field @0x08 is in **32-BYTE UNITS** (34 x 32 =
//     1088, exact) — this file is the SECOND confirmation of that family
//     convention, the first being the BMG header (trap 2 above).
//   · one block, `CLT1`, at 0x20; block body at 0x28.
//   · a 4-byte field at 0x28 (observed `0x01000000`) whose meaning I have NOT
//     established — it is READ AND REPORTED, never interpreted, and it is not
//     used as a gate. Saying "unidentified" beats inventing a name for it.
//   · palette base **0x2C**: 256 x RGBA8 big-endian, 1024 bytes, then 20 bytes
//     trailing.
//
// ALL 256 ENTRIES ARE READ. In the shipped file indices 9-255 are white and
// only 0-8 are distinct, but that is an OBSERVATION ABOUT THIS FILE, not a
// rule about the format — hardcoding "9 colours then white" would repeat, one
// level down, the exact mistake this exists to correct.
// ============================================================================
bool wwMessageColor_openFromArc(const void* arc, uint32_t size, const char* member);
bool wwMessageColor_open(const void* bytes, uint32_t size);

// RGBA8 for a palette index, in the donor's own byte order. Any index is
// valid (the table is 256 entries by construction); an unopened table returns
// 0 and sets no colour — callers must check `wwMessageColor_isOpen()` rather
// than treat 0 as black.
uint32_t wwMessageColor_get(uint8_t index);
bool wwMessageColor_isOpen();
uint32_t wwMessageColor_distinctCount();   // for the boot receipt
void wwMessageColor_close();

// Release the reader's view of the buffer.
//
// NOT OPTIONAL HOUSEKEEPING. `wwMessage_open()` stores POINTERS INTO the
// caller's bytes and copies nothing — so the moment that buffer dies, every
// `WwMessageEntry.text` and the reader's own block pointers dangle. A caller
// that parses from a temporary MUST close before the temporary goes away.
// This is the same class as `TResource` caching raw pointers into a parsed
// archive: the engine has it too, and it is why "never free an arc while
// parsed data is cached" is a standing rule here.
void wwMessage_close();

// Counters for the boot receipt. Zero from an unopened reader and zero from an
// empty archive are DIFFERENT states and are reported as such.
uint32_t wwMessage_count();
bool wwMessage_isOpen();
