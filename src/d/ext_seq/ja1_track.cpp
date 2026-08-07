// KIT-LINEAGE: bridge-owed:§369
// KIT-DONOR: none
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
#include "d/ext_seq/ja1_track.h"
#include "d/ext_seq/ja1_parser.h"
#include "d/ext_seq/ja1_bank.h"
#include "d/ext_seq/ja1_event_dump.h"

#include <cstdio>
#include <cstdlib>
#include <unordered_map>

#include "dusk/audio/DuskDsp.hpp"
#include "dusk/logging.h"
#include "JSystem/JAudio2/JASBank.h"
#include "JSystem/JAudio2/JASChannel.h"
#include "JSystem/JAudio2/JASOscillator.h"

namespace ExtSeq {
namespace {

bool volProbeEnv() {
    const char* v = std::getenv("DUSK_EXTSEQ_VOL_PROBE");
    if (v == nullptr || v[0] == '\0' || (v[0] == '0' && v[1] == '\0')) {
        return false;
    }
    return true;
}

// ============================================================
// §P2 note-fidelity tap — receiver-side mirror of the donor
// DuskTap at TTrack::noteOn @80281258 (JASSeqParser.cpp:915:
//   track->noteOn(noteid, note, r25, time, r22)). Emits ONE line
// per note in the donor capture's exact shape
//   note=<voice> p=(<key>,<vel>,<gate>)
// so the P2 differ can diff our note stream against
// docs/WW Linked/dolphin-captures-bgm-notes-20260728.txt
// note-for-note. HIGH VOLUME — toggle DUSK_EXTSEQ_NOTE_TAP=1 for a
// capture session only, default OFF. Unbudgeted (full stream, like
// the donor's 8,044-note capture).
// ============================================================
bool noteTapEnv() {
    const char* v = std::getenv("DUSK_EXTSEQ_NOTE_TAP");
    if (v == nullptr || v[0] == '\0' || (v[0] == '0' && v[1] == '\0')) {
        return false;
    }
    return true;
}

/** Fresh budget each startOwned so a field→house hop still captures samples. */
u32& volProbeSetParamBudget() {
    static u32 s_left = 96;
    return s_left;
}

u32& volProbeNoteOnBudget() {
    static u32 s_left = 24;
    return s_left;
}

void volProbeResetBudgets() {
    volProbeSetParamBudget() = 96;
    volProbeNoteOnBudget() = 24;
}

// ============================================================
// §363 per-note JA1→JA2 envelope translation storage (Phase A seam).
// Donor lifetime: the channel keeps a POINTER to TTrack storage
// (field_0x2cc) for the note's whole life. The port's Ja1Track objects
// are deleted on close() while released voices keep ringing, so the
// converted JASOscillator::Data must NOT live in the track (J3D
// pointer-fix lesson: never free while a consumer caches the pointer).
// Each note that receives track osc data gets its own heap block,
// registered per channel and freed on the channel's CB_STOP (fired from
// ~JASChannel — guaranteed on every death path).
// ============================================================
constexpr u32 kOscPointCap = 16;

struct NoteOscBlock {
    JASOscillator::Data data[2]{};
    JASOscillator::Point atk[2][kOscPointCap]{};
    JASOscillator::Point rel[2][kOscPointCap]{};
};

std::unordered_map<JASChannel*, NoteOscBlock*>& noteOscMap() {
    static std::unordered_map<JASChannel*, NoteOscBlock*> s_map;
    return s_map;
}

NoteOscBlock* noteOscBlockFor(JASChannel* ch) {
    auto& map = noteOscMap();
    auto it = map.find(ch);
    if (it != map.end()) {
        return it->second;
    }
    NoteOscBlock* blk = new NoteOscBlock();
    map.emplace(ch, blk);
    return blk;
}

void noteOscBlockDrop(JASChannel* ch) {
    auto& map = noteOscMap();
    auto it = map.find(ch);
    if (it != map.end()) {
        delete it->second;
        map.erase(it);
    }
}

// ============================================================
// §363 JA1 envelope table → JA2 Point table.
// JA1 tables are raw s16 triples (mode,time,value) — TOscillator::calc
// reads i_table[idx..idx+2]; a JA2 Point is the SAME triple as a struct
// (BE-tagged on PC — assignment through the BE wrapper stores swapped).
// Semantics that carry 1:1:
//   - mode space: 0-3 curve ids (linear/square/sqroot/samplecell, same
//     table order both engines), 13 loop (value = point INDEX in both),
//     14 hold, 15 stop;
//   - value scale: v/0x8000 both sides;
//   - TIME UNITS map 1:1 EXACTLY: JA1 runs envTime*((dacRate/80)/600)
//     /updateInterval updates (updateInterval=1 retail); JA2 runs
//     mTime/(48000/dacRate) updates — and (dacRate/80)/600 ==
//     dacRate/48000, so the constants cancel and mTime = envTime.
// End rule = donor BankMgr oscTableEnd: copy through the first triple
// with mode > 10 (loop/hold/stop all terminate the walk).
// Returns the number of points written.
// ============================================================
u32 ja1TableToJa2Points(const s16* src, JASOscillator::Point* dst, u32 cap) {
    if (src == nullptr) {
        return 0;
    }
    u32 n = 0;
    while (n < cap) {
        dst[n].mEnvelopeMode = src[n * 3 + 0];
        dst[n].mTime = src[n * 3 + 1];
        dst[n].mValue = src[n * 3 + 2];
        const bool terminal = src[n * 3 + 0] > 10;
        n++;
        if (terminal) {
            return n;
        }
    }
    // ===== §363 memory-safety adaptation: a table unterminated within cap
    // would let the JA2 reader walk past the buffer — force a HOLD
    // terminator instead (sustains the last value; no invented shaping).
    // All donor tables fit easily (sAdsTable = 4 triples, sRelTable = 2,
    // vib/tre = 6).
    DuskLog.warn("[ExtSeq] §363 osc table unterminated within {} points — HOLD forced",
                 kOscPointCap);
    dst[cap - 1].mEnvelopeMode = static_cast<s16>(14);
    dst[cap - 1].mTime = static_cast<s16>(0);
    dst[cap - 1].mValue = static_cast<s16>(0);
    return cap;
}

void voiceCallback(u32 type, JASChannel* ch, JASDsp::TChannel*, void* user) {
    if (type != JASChannel::CB_STOP || ch == nullptr) {
        return;
    }
    // ===== §363: the channel is dying (fired from ~JASChannel) — its
    // translated osc block, if any, dies with it.
    noteOscBlockDrop(ch);
    if (user == nullptr) {
        return;
    }
    auto** slot = static_cast<JASChannel**>(user);
    if (*slot == ch) {
        *slot = nullptr;
    }
}

}  // namespace

void Ja1Track::initTimed() {
    // WW TTrack::initTimed — vol/defaults then overrides for pitch/pan/fx/…
    for (int i = 0; i < TIMED_PARAMS; i++) {
        mTimed[i].mMoveTime = 0.0f;
        mTimed[i].mMoveAmount = 0.0f;
        mTimed[i].mCurrent = 1.0f;
        mTimed[i].mTarget = 1.0f;
    }
    mTimed[TIMED_Pitch].mCurrent = 0.0f;
    mTimed[TIMED_Pitch].mTarget = 0.0f;
    mTimed[TIMED_Pan].mCurrent = 0.5f;
    mTimed[TIMED_Pan].mTarget = 0.5f;
    mTimed[TIMED_Fxmix].mCurrent = 0.0f;
    mTimed[TIMED_Fxmix].mTarget = 0.0f;
    mTimed[TIMED_Dolby].mCurrent = 0.0f;
    mTimed[TIMED_Dolby].mTarget = 0.0f;
    mTimed[5].mCurrent = 0.0f;
    mTimed[5].mTarget = 0.0f;
    mTimed[16].mCurrent = 0.5f;
    mTimed[16].mTarget = 0.5f;
    mTimed[17].mCurrent = 0.0f;
    mTimed[17].mTarget = 0.0f;
    for (int i = 1; i < 4; i++) {
        mTimed[12 + i].mCurrent = 0.0f;
        mTimed[12 + i].mTarget = 0.0f;
    }
}

void Ja1Track::init() {
    mSeq.init();
    mNoteMgr = NoteMgr{};
    mRegisterParam.init();
    mParent = nullptr;
    for (auto& c : mChildren) {
        c = nullptr;
    }
    for (auto& v : mVoices) {
        v = nullptr;
    }
    initTimed();
    mTranspose = 0;
    mTempo = 120;
    mTimebase = 48;
    mVolumeMode = 0;
    mActive = false;
    mIsPaused = false;
    mPauseStatus = 0;
    mTrackId = 0;
    // ============================================================
    // §363 donor osc/envelope table seeding. WW TTrack::init()
    // (JASTrack.cpp:59-62): routes → 0x0F (unrouted), both track oscs →
    // Player::sEnvelopeDef. WW TTrack ctor (JASTrack.cpp:41-43): the
    // SimpleADSR triple table ← Player::sAdsTable; field_0x374 ← 0
    // (ctor :27 / init :75). The port's init() runs once right after
    // new, so it carries the ctor seeding too.
    // ============================================================
    mOscRoute[0] = 0x0f;
    field_0x2cc[0] = JAudio1::Player::sEnvelopeDef;
    mOscRoute[1] = 0x0f;
    field_0x2cc[1] = JAudio1::Player::sEnvelopeDef;
    for (int i = 0; i < 12; i++) {
        field_0x304[i] = JAudio1::Player::sAdsTable[i];
    }
    field_0x374 = 0;
}

void Ja1Track::close() {
    noteOffAll();
    for (int i = 0; i < kMaxChildren; i++) {
        if (mChildren[i] != nullptr) {
            mChildren[i]->close();
            delete mChildren[i];
            mChildren[i] = nullptr;
        }
    }
    mActive = false;
    mSeq.init();
}

void Ja1Track::start(u8* base, u32 offset) {
    mSeq.start(base, offset);
    mActive = true;
    mSeq.wait(0);
}

Ja1Track* Ja1Track::openChild(u8 idx, u8 /*flags*/) {
    if (idx >= kMaxChildren) {
        return nullptr;
    }
    if (mChildren[idx] == nullptr) {
        mChildren[idx] = new Ja1Track();
        mChildren[idx]->init();
        mChildren[idx]->mParent = this;
        mChildren[idx]->mRegisterParam = mRegisterParam;
        // WW inherit(): copy current tempo/timebase; later root writes
        // re-push via updateTempo (copy-at-spawn alone is not enough).
        mChildren[idx]->mTempo = getTempo();
        mChildren[idx]->mTimebase = getTimebase();
        // WW TTrack::openTrack — pack hierarchy id; warn past 8 levels
        // (JASTrack.cpp:974–980). Report only — WW still returns the child.
        const u32 topNibble = mTrackId & 0xf0000000u;
        if (topNibble >= 0x70000000u) {
            DuskLog.warn(
                "[ExtSeq] openTrack: hierarchy exceeds 8 levels — invalid track ID "
                "(WW JASTrack.cpp:977)");
        }
        mChildren[idx]->mTrackId =
            (topNibble + 0x10000000u) |
            (((mTrackId << 4) | static_cast<u32>(idx)) & 0x0fffffffu);
    }
    return mChildren[idx];
}

int Ja1Track::noteOn(u8 voice, s32 key, s32 vel, s32 gate, u32 /*prio*/) {
    if (Ja1EventDump::active()) {
        (void)voice;
        (void)key;
        (void)vel;
        (void)gate;
        return 0;
    }
    // ============================================================
    // §P2 note-fidelity tap (donor-shape mirror; see noteTapEnv above).
    // Raw entry args = the exact quantities the donor tap logged at
    // TTrack::noteOn: voice=noteid, key/vel/gate = note/vel/time.
    // ============================================================
    if (noteTapEnv()) {
        DuskLog.info("[ExtSeq] §P2 noteTap note={:>3} p=({:04x},{:04x},{:04x})",
                     static_cast<unsigned>(voice),
                     static_cast<unsigned>(key) & 0xFFFFu,
                     static_cast<unsigned>(vel) & 0xFFFFu,
                     static_cast<unsigned>(gate) & 0xFFFFu);
    }
    if (voice >= kMaxVoices) {
        voice = 0;
    }
    if (mVoices[voice] != nullptr) {
        mVoices[voice]->release(0);
        mVoices[voice] = nullptr;
    }
    // §53: real WAVE via IBNK/WSYS/.aw (shadow-wave virt addr). ExtSeq owns
    // the channel (№89). Miss → silence (№91), not a foreign fallback.
    const u8 k = static_cast<u8>(key & 0x7F);
    const u8 v = static_cast<u8>(vel & 0x7F);
    JASChannel* ch = nullptr;
    if (ja1Bank_ready()) {
        ch = ja1Bank_noteOn(mRegisterParam.getBankNumber(),
                            mRegisterParam.getProgramNumber(), k, v, 0x7F, voiceCallback,
                            &mVoices[voice]);
    }
    if (ch == nullptr && mRegisterParam.getProgramNumber() >= 0xF0) {
        ch = JASBank::noteOnOsc(mRegisterParam.getProgramNumber() - 0xF0, k, v, 0x7F,
                                voiceCallback, &mVoices[voice]);
    }
    mVoices[voice] = ch;
    if (ch != nullptr) {
        // Osc fallback still needs №31-B WW vel curve (WAVE path bakes inside
        // ja1Bank_noteOn). Init vol for osc is 1.0.
        if (!ja1Bank_isExtSeqChannel(ch)) {
            ja1Bank_applyWwVelocityCurve(ch, 1.0f, v);
        }
        // Latch current mix onto the new voice (ramps keep updating via apply).
        ch->mParams.mVolume = composedVolume();
        ch->mParams.mPan = composedPan();
        ch->mParams.mFxMix = composedFxmix();
        ch->mParams.mDolby = composedDolby();
        // ============================================================
        // §363 donor JASTrack::noteOn tail (WW JASTrack.cpp:325-328):
        //     overwriteOsc(channel);
        //     if (field_0x374) channel->directReleaseOsc(0, field_0x374);
        // Route-gated track osc data lands on the voice; the SimpleADSR
        // direct release (arg 4) applies UNCONDITIONALLY of routing.
        // JA2 setDirectRelease targets mOscillators[0] — the same slot 0
        // as the donor call, with an identical u16 encoding
        // ((dr>>14)&3 curve id, dr&0x3FFF time; time units cancel exactly
        // — see ja1TableToJa2Points header).
        // ============================================================
        overwriteOsc(ch);
        if (field_0x374) {
            ch->setDirectRelease(field_0x374);
        }
        if (volProbeEnv()) {
            u32& left = volProbeNoteOnBudget();
            if (left > 0) {
                left--;
                DuskLog.info(
                    "[ExtSeq] §C.1 volProbe noteOn tid=0x{:x} key={} vel={} "
                    "chVol={:.4f} composed={:.4f} raw={:.4f}",
                    mTrackId, static_cast<unsigned>(k), static_cast<unsigned>(v),
                    ch->mParams.mVolume, composedVolume(), timedCurrent(TIMED_Volume));
            }
        }
    }
    return ch != nullptr ? 1 : 0;
}

bool Ja1Track::noteOff(u8 voice, u16 release) {
    if (voice >= kMaxVoices) {
        return false;
    }
    if (mVoices[voice] != nullptr) {
        // ============================================================
        // §363 donor JASTrack::noteOff → TChannel::stop(param_2)
        // (WW JASTrack.cpp:360-373, JASChannel.cpp:283-292): a nonzero
        // release overrides the osc's direct release before stopping.
        // JA2 JASChannel::release(u16) is the exact same contract
        // (0 = plain release on the osc's own rel_table). The parser's
        // cmdNoteOff release value (>100 → (r6-98)*20 expansion) now
        // reaches the voice instead of being dropped.
        // ============================================================
        mVoices[voice]->release(release);
        mVoices[voice] = nullptr;
        return true;
    }
    return false;
}

void Ja1Track::noteOffAll() {
    for (int i = 0; i < kMaxVoices; i++) {
        if (mVoices[i] != nullptr) {
            mVoices[i]->release(0);
            mVoices[i] = nullptr;
        }
    }
}

// ============================================================
// §363 donor JASTrack::overwriteOsc (WW JASTrack.cpp:333-358), Phase-A
// translated. Donor route nibble semantics kept verbatim:
//   0x0F        = unrouted → skip (init default; SimpleADSR does NOT
//                 route — sequences route via cmdOscRoute 0xF0);
//   bits 0-1    = destination channel osc slot;
//   bit 3 (0x8) = copy the channel's current osc into the track first;
//   bit 2 (0x4) = same copy but the track keeps its own rel_table.
// Phase-A adaptation: the track-side JA1 Osc_ is converted to a JA2
// JASOscillator::Data (field map in ja1TableToJa2Points) held in a
// per-note heap block, then installed with setOscInit — the donor's
// channel->overwriteOsc(r28, ...) = setOscInit + one immediate
// effectOsc step; JA2 applies on its next channel update instead.
// KNOWN Phase-A residual (named, not approximated): for curve modes 1-3
// on RISING segments JA1 mirrors the curve (calc: fIdx from the
// remaining fraction when the delta is positive) while JA2 always
// weights by the elapsed fraction — identical for linear (mode 0, the
// SimpleADSR default) and for all falling segments. The ported
// JAudio1::TOscillator is the authority to diff this against (Phase B).
// ============================================================
void Ja1Track::overwriteOsc(JASChannel* param_1) {
    for (int i = 0; i < 2; i++) {
        const u32 var1 = mOscRoute[i];
        if (var1 == 0x0f) {
            continue;
        }
        const u32 r28 = var1 & 3;
        if (r28 >= 2) {
            // ===== §363 BRIDGE-OWED: JA1 TChannel carries 4 osc slots
            // (osc[4]); the JA2 backend carries 2 (mOscillators[2]).
            // Routes to slots 2/3 have no Phase-A destination — owed to
            // the full JAudio1 voice path (Phase B+).
            static bool s_warnedSlot = false;
            if (!s_warnedSlot) {
                s_warnedSlot = true;
                DuskLog.warn(
                    "[ExtSeq] §363 BRIDGE-OWED osc route → slot {} (JA1 has 4 osc "
                    "slots, JA2 backend has 2) — route dropped",
                    r28);
            }
            continue;
        }
        if ((var1 & 8) || (var1 & 4)) {
            // ===== §363 BRIDGE-OWED: donor copyOsc flags read the CHANNEL's
            // current (bank) osc back into the track before overwriting
            // (0x8 = full copy, 0x4 = copy keeping the track rel_table —
            // JASTrack.cpp:341-355). That needs a JA2→JA1 table
            // reification; deferred to Phase B rather than approximated.
            // Donor behavior NOT reproduced for these flag bits.
            static bool s_warnedCopy = false;
            if (!s_warnedCopy) {
                s_warnedCopy = true;
                DuskLog.warn(
                    "[ExtSeq] §363 BRIDGE-OWED osc route copy flag 0x{:X} "
                    "(donor copyOsc) — route dropped, bank osc kept",
                    var1 & 0xC);
            }
            continue;
        }
        NoteOscBlock* blk = noteOscBlockFor(param_1);
        JASOscillator::Data& d = blk->data[i];
        const JAudio1::TOscillator::Osc_& src = field_0x2cc[i];
        // ===== §363 field-for-field translation (JA1 Osc_ → JA2 Data):
        //   field_0x0  (target id) → mTarget  — id space identical
        //              (0=volume, 1=pitch, 2=pan, 3=fxmix, 4=dolby);
        //   field_0x4  (rate)      → mRate    — durations cancel exactly
        //              (see ja1TableToJa2Points);
        //   table / rel_table      → mTable / rel_table (Point-converted);
        //   field_0x10 (scale)     → mScale;
        //   field_0x14 (vertex)    → mVertex.
        d.mTarget = src.field_0x0;
        d.mRate = src.field_0x4;
        d.mScale = src.field_0x10;
        d.mVertex = src.field_0x14;
        d.mTable = nullptr;
        d.rel_table = nullptr;
        if (src.table != nullptr && ja1TableToJa2Points(src.table, blk->atk[i], kOscPointCap) > 0) {
            d.mTable = blk->atk[i];
        }
        if (src.rel_table != nullptr &&
            ja1TableToJa2Points(src.rel_table, blk->rel[i], kOscPointCap) > 0) {
            d.rel_table = blk->rel[i];
        }
        param_1->setOscInit(r28, &d);
        // ===== §363 donor TOscillator::release (JASOscillator.cpp:139-141):
        // a relless osc with no direct release falls back to
        // mDirectRelease = 0x10. JA2 hard-stops in that case, so latch the
        // donor default at noteOn (slot 0 only — direct release is osc[0]
        // in both engines). sAdsrDef has rel_table = NULL, so SimpleADSR
        // notes with release arg 0 depend on this.
        if (r28 == 0 && d.rel_table == nullptr && field_0x374 == 0) {
            param_1->setDirectRelease(0x10);
        }
    }
}

void Ja1Track::writeRegDirect(u8 reg, u16 value) {
    if (reg < 32) {
        mRegisterParam.r[reg] = value;
    }
    if (reg == 6) {
        mRegisterParam.bankProg = value;
        // ===== §363 donor: any write to register 6 (bank/prog) resets the
        // osc routes to unrouted (WW JASTrack.cpp:1301-1306).
        mOscRoute[0] = 0x0f;
        mOscRoute[1] = 0x0f;
    }
}

u16 Ja1Track::readReg16(u8 reg) {
    if (reg == 0x20) {
        return mRegisterParam.getBankNumber();
    }
    if (reg == 0x21) {
        return mRegisterParam.getProgramNumber();
    }
    if (reg < 32) {
        return mRegisterParam.r[reg];
    }
    return 0;
}

u32 Ja1Track::readReg32(u8 reg) {
    return readReg16(reg);
}

u32 Ja1Track::exchangeRegisterValue(u8 reg) {
    return readReg16(reg);
}

int Ja1Track::getTranspose() const {
    int t = mTranspose;
    if (mParent != nullptr) {
        t += mParent->getTranspose();
    }
    return t;
}

u16 Ja1Track::getTempo() const {
    return mParent != nullptr ? mParent->getTempo() : mTempo;
}

u16 Ja1Track::getTimebase() const {
    return mParent != nullptr ? mParent->getTimebase() : mTimebase;
}

void Ja1Track::updateTempo() {
    // WW JASystem::TTrack::updateTempo — children take parent's timebase
    // (and tempo) so a post-spawn 0xFD/0xFE on the root cannot leave layers
    // running at divergent rates.
    if (mParent != nullptr) {
        mTempo = mParent->mTempo;
        mTimebase = mParent->mTimebase;
    }
    for (int i = 0; i < kMaxChildren; i++) {
        if (mChildren[i] != nullptr && mChildren[i]->mActive) {
            mChildren[i]->updateTempo();
        }
    }
}

void Ja1Track::setParam(u8 target, f32 value, int moveTime) {
    // WW JASystem::TTrack::setParam — timed ramp via MoveParam_.
    if (target >= TIMED_PARAMS) {
        // §C.1: WW JUT_ASSERTs; we still no-op but must not stay silent — first
        // hit only so a hot path cannot spam. Dump/log only; no behaviour change.
        static bool s_warnedOob = false;
        if (!s_warnedOob) {
            s_warnedOob = true;
            DuskLog.warn(
                "[ExtSeq] §C.1 setParam target {} >= TIMED_PARAMS ({}) — write dropped "
                "(WW JASTrack.cpp:815 asserts)",
                static_cast<unsigned>(target), static_cast<unsigned>(TIMED_PARAMS));
        }
        return;
    }
    MoveParam& param = mTimed[target];
    // §C.1 absolute-level probe — gated; first 96 volume (target 0) hits per start.
    bool probed = false;
    if (target == TIMED_Volume && volProbeEnv() && !Ja1EventDump::active()) {
        u32& left = volProbeSetParamBudget();
        if (left > 0) {
            left--;
            probed = true;
            DuskLog.info(
                "[ExtSeq] §C.1 volProbe setParam tid=0x{:x} raw={:.4f}→{:.4f} "
                "composed_now={:.4f} moveTime={} mode={} voices={}",
                mTrackId, param.mCurrent, value, composedVolume(), moveTime, mVolumeMode,
                activeVoiceCount());
        }
    }
    param.mTarget = value;
    if (moveTime <= 0) {
        param.mCurrent = param.mTarget;
        param.mMoveAmount = 0.0f;
        param.mMoveTime = 1.0f;
    } else {
        param.mMoveAmount = (param.mTarget - param.mCurrent) / static_cast<f32>(moveTime);
        param.mMoveTime = static_cast<f32>(moveTime);
    }
    if (probed && moveTime <= 0) {
        DuskLog.info(
            "[ExtSeq] §C.1 volProbe afterSnap tid=0x{:x} raw={:.4f} composed={:.4f}",
            mTrackId, param.mCurrent, composedVolume());
    }
}

void Ja1Track::updateTimedParam() {
    // WW updateTimedParam — one step per track tick.
    for (int i = 0; i < TIMED_PARAMS; i++) {
        MoveParam& param = mTimed[i];
        if (param.mMoveTime > 0.0f) {
            param.mCurrent += param.mMoveAmount;
            param.mMoveTime -= 1.0f;
            if (param.mMoveTime <= 0.0f) {
                param.mCurrent = param.mTarget;
                param.mMoveTime = 0.0f;
                param.mMoveAmount = 0.0f;
            }
        }
    }
}

f32 Ja1Track::composedVolume() const {
    // §76 / WW JASTrack.cpp:530-533, :637-644 — OWN track only:
    //   vol = timed[Volume]; if (volumeMode==0) vol *= vol; × outer (if switch).
    // NO parent.composedVolume() cascade — that was a port invention; BMS root
    // volume writes are inert for children in the donor.
    f32 vol = mTimed[TIMED_Volume].mCurrent;
    if (mVolumeMode == 0) {
        vol *= vol;
    }
    // §81 SoundTable BGM vol_u8/127 — donor master on the sequence (manifest bypassed it).
    vol *= dusk::audio::getExtSeqMasterVol();
    if (vol < 0.0f) {
        return 0.0f;
    }
    if (vol > 1.0f) {
        return 1.0f;
    }
    return vol;
}

f32 Ja1Track::composedPan() const {
    // WW: child pan is offset from parent center, then re-centered.
    f32 pan = mTimed[TIMED_Pan].mCurrent - 0.5f;
    if (mParent != nullptr) {
        pan += mParent->composedPan() - 0.5f;
    }
    pan += 0.5f;
    if (pan < 0.0f) {
        return 0.0f;
    }
    if (pan > 1.0f) {
        return 1.0f;
    }
    return pan;
}

f32 Ja1Track::composedFxmix() const {
    // BMS timed fxmix is 0 on Outset; donor wetness is type-7 scene Fxline.
    // PC approx: scene send → AutoMixer fxmix (sum timed chain once, then + send).
    f32 fx = 0.0f;
    for (const Ja1Track* t = this; t != nullptr; t = t->mParent) {
        fx += t->mTimed[TIMED_Fxmix].mCurrent;
    }
    fx += dusk::audio::getExtSeqFxSend();
    if (fx < 0.0f) {
        return 0.0f;
    }
    if (fx > 1.0f) {
        return 1.0f;
    }
    return fx;
}

f32 Ja1Track::composedDolby() const {
    f32 d = mTimed[TIMED_Dolby].mCurrent;
    if (mParent != nullptr) {
        d += mParent->composedDolby();
    }
    if (d < 0.0f) {
        return 0.0f;
    }
    if (d > 1.0f) {
        return 1.0f;
    }
    return d;
}

void Ja1Track::applyVoiceParamsDeep() {
    applyVoiceParams();
    for (int i = 0; i < kMaxChildren; i++) {
        if (mChildren[i] != nullptr && mChildren[i]->mActive) {
            mChildren[i]->applyVoiceParamsDeep();
        }
    }
}

int Ja1Track::activeVoiceCount() const {
    int n = 0;
    for (int i = 0; i < kMaxVoices; i++) {
        if (mVoices[i] != nullptr) {
            n++;
        }
    }
    return n;
}

void Ja1Track::logVolProbeTree(const char* tag) const {
    if (!volProbeEnv() || !mActive) {
        return;
    }
    const char* label = (tag != nullptr && tag[0] != '\0') ? tag : "tree";
    DuskLog.info(
        "[ExtSeq] §C.1 volProbe {} tid=0x{:x} raw={:.4f} tgt={:.4f} composed={:.4f} "
        "mode={} voices={}",
        label, mTrackId, timedCurrent(TIMED_Volume), timedTarget(TIMED_Volume),
        composedVolume(), mVolumeMode, activeVoiceCount());
    for (int i = 0; i < kMaxChildren; i++) {
        if (mChildren[i] != nullptr && mChildren[i]->mActive) {
            mChildren[i]->logVolProbeTree(tag);
        }
    }
}

void Ja1Track::volProbeOnSeqStart(Ja1Track* root) {
    if (!volProbeEnv() || root == nullptr) {
        return;
    }
    volProbeResetBudgets();
    DuskLog.info("[ExtSeq] §C.1 volProbe ON — absolute levels / composedVolume "
                 "(set DUSK_EXTSEQ_VOL_PROBE=0 to disable)");
    root->logVolProbeTree("post-start");
}

void Ja1Track::applyVoiceParams() {
    const f32 vol = composedVolume();
    const f32 pan = composedPan();
    const f32 fx = composedFxmix();
    const f32 dolby = composedDolby();
    for (int i = 0; i < kMaxVoices; i++) {
        JASChannel* ch = mVoices[i];
        if (ch == nullptr) {
            continue;
        }
        ch->mParams.mVolume = vol;
        ch->mParams.mPan = pan;
        ch->mParams.mFxMix = fx;
        ch->mParams.mDolby = dolby;
    }
}

void Ja1Track::setTempo(u16 t) {
    if (t == 0) {
        return;
    }
    mTempo = t;
    // WW: only the root updateTempo() fans out; child tempo is relative.
    if (mParent == nullptr) {
        updateTempo();
    }
}

void Ja1Track::setTimebase(u16 t) {
    if (t == 0) {
        return;
    }
    mTimebase = t;
    if (mParent == nullptr) {
        updateTempo();
    }
}

void Ja1Track::writeRegParam(u8 param) {
    // Minimal WW writeRegParam for bank/prog (0xA4 path) and direct regs.
    u8 iVar1 = param & 0xC;
    u8 iVar2 = param & 0x3;
    if ((param & 0xF) == 10 || (param & 0xF) == 11 || (param & 0xF) == 9) {
        // Extended forms — consume bytes so we don't desync, then return.
        if ((param & 0xF) == 10 || (param & 0xF) == 9) {
            (void)mSeq.readByte();
        }
        (void)mSeq.readByte();
        switch (iVar1) {
        case 0:
            (void)mSeq.readByte();
            break;
        case 4:
            (void)mSeq.readByte();
            break;
        case 12:
            (void)mSeq.read16();
            break;
        case 8:
            (void)mSeq.readByte();
            break;
        default:
            break;
        }
        if (Ja1EventDump::active()) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "0xA%X", static_cast<unsigned>(param & 0xF));
            Ja1EventDump::emit("write_reg", buf, "");
        }
        return;
    }

    u8 bVar0 = mSeq.readByte();
    s16 sVar0 = 0;
    switch (iVar1) {
    case 0:
        sVar0 = static_cast<s16>(readReg16(mSeq.readByte()));
        break;
    case 4:
        sVar0 = mSeq.readByte();
        break;
    case 12:
        sVar0 = static_cast<s16>(mSeq.read16());
        break;
    case 8: {
        u32 byte = mSeq.readByte();
        sVar0 = (byte & 0x80) ? static_cast<s16>(byte << 8)
                              : static_cast<s16>((byte << 8) | (byte << 1));
        break;
    }
    default:
        break;
    }

    if (bVar0 == 0x20) {
        // Set bank (high byte of bankProg)
        mRegisterParam.bankProg =
            static_cast<u16>((static_cast<u16>(sVar0) << 8) | mRegisterParam.getProgramNumber());
        mRegisterParam.setFlag(static_cast<u16>(sVar0));
        if (Ja1EventDump::active()) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(sVar0 & 0xFF));
            Ja1EventDump::emit("bank", buf, "");
        }
        return;
    }
    if (bVar0 == 0x21) {
        mRegisterParam.bankProg =
            static_cast<u16>((mRegisterParam.getBankNumber() << 8) | (sVar0 & 0xFF));
        mRegisterParam.setFlag(static_cast<u16>(sVar0));
        if (Ja1EventDump::active()) {
            char buf[16];
            std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(sVar0 & 0xFF));
            Ja1EventDump::emit("prog", buf, "");
        }
        return;
    }
    if (bVar0 < 32) {
        if (iVar2 == 1) {
            sVar0 = static_cast<s16>(mRegisterParam.r[bVar0] + sVar0);
        }
        mRegisterParam.r[bVar0] = static_cast<u16>(sVar0);
        mRegisterParam.setFlag(static_cast<u16>(sVar0));
        if (bVar0 == 6) {
            // ===== §363 donor: a write to register 6 resets the osc routes
            // to unrouted (WW JASTrack.cpp:1301-1306).
            mOscRoute[0] = 0x0f;
            mOscRoute[1] = 0x0f;
        }
    }
    if (Ja1EventDump::active()) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "0xA%X", static_cast<unsigned>(param & 0xF));
        Ja1EventDump::emit("write_reg", buf, "");
    }
    (void)iVar2;
}

void Ja1Track::tick(Ja1Parser& parser) {
    if (!mActive) {
        return;
    }
    // WW mainProc: advance timed params every tick, then wait/parse, then children.
    updateTimedParam();
    if (mSeq.waitCountDown()) {
        if (parser.parseSeq(this) < 0) {
            mActive = false;
            applyVoiceParams();
            return;
        }
    }
    applyVoiceParams();
    for (int i = 0; i < kMaxChildren; i++) {
        if (mChildren[i] != nullptr && mChildren[i]->mActive) {
            mChildren[i]->tick(parser);
        }
    }
}

}  // namespace ExtSeq
