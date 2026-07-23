#include "d/ext_seq/ja1_track.h"
#include "d/ext_seq/ja1_parser.h"
#include "d/ext_seq/ja1_bank.h"
#include "d/ext_seq/ja1_event_dump.h"

#include <cstdio>
#include <cstdlib>

#include "dusk/audio/DuskDsp.hpp"
#include "dusk/logging.h"
#include "JSystem/JAudio2/JASBank.h"
#include "JSystem/JAudio2/JASChannel.h"

namespace ExtSeq {
namespace {

bool volProbeEnv() {
    const char* v = std::getenv("DUSK_EXTSEQ_VOL_PROBE");
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

void voiceCallback(u32 type, JASChannel* ch, JASDsp::TChannel*, void* user) {
    if (type != JASChannel::CB_STOP || user == nullptr || ch == nullptr) {
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

int Ja1Track::noteOn(u8 voice, s32 key, s32 vel, s32 /*gate*/, u32 /*prio*/) {
    if (Ja1EventDump::active()) {
        (void)voice;
        (void)key;
        (void)vel;
        return 0;
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

bool Ja1Track::noteOff(u8 voice, u16 /*release*/) {
    if (voice >= kMaxVoices) {
        return false;
    }
    if (mVoices[voice] != nullptr) {
        mVoices[voice]->release(0);
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

void Ja1Track::writeRegDirect(u8 reg, u16 value) {
    if (reg < 32) {
        mRegisterParam.r[reg] = value;
    }
    if (reg == 6) {
        mRegisterParam.bankProg = value;
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
