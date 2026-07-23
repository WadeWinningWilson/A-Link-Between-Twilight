/**
 * ja1_bank.cpp — §53 bank residency via shadow-wave virtual addresses.
 *
 * Parse staged ibnk_*.bin / wsys_*.bin, keep .aw in ExtSeq RAM, register at
 * virtual bases. noteOn mints kShadowVirtualBase + virtBase + offset.
 */

#include "d/ext_seq/ja1_bank.h"

#include "dolphin/types.h"

#if TARGET_PC

#include <algorithm>
#include <bit>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "JSystem/JKernel/JKRHeap.h"

#include "JSystem/JAudio2/JASAiCtrl.h"
#include "JSystem/JAudio2/JASBank.h"
#include "JSystem/JAudio2/JASChannel.h"
#include "JSystem/JAudio2/JASOscillator.h"
#include "JSystem/JAudio2/JASWaveInfo.h"
#include "JSystem/JAudio2/JASWSParser.h"
#include "JSystem/JSupport/JSupport.h"
#include "d/d_com_inf_game.h"
#include "dusk/audio/DuskDsp.hpp"
#include "dusk/endian.h"
#include "dusk/logging.h"

namespace ExtSeq {
namespace {

namespace fs = std::filesystem;

// Virtual ARAM bases inside the shadow address space (after stripping
// kShadowVirtualBase). Distinct ranges — not twins of any vanilla bank.
constexpr u32 kVirtBaseZelda = 0x01000000u;
constexpr u32 kVirtBaseLink = 0x02000000u;
constexpr u32 kVirtBaseCharVoice = 0x03000000u;

struct VeloRegion {
    u8 maxVel = 127;
    u16 waveId = 0;
    f32 volScale = 1.0f;
    f32 pitchScale = 1.0f;
};

struct KeyRegion {
    u8 highKey = 127;
    std::vector<VeloRegion> velos;
};

struct Inst {
    f32 volume = 1.0f;
    f32 pitch = 1.0f;
    const JASOscillator::Data* osc[2]{};
    u8 oscCount = 0;
    std::vector<KeyRegion> keys;
};

struct WaveArc {
    u32 virtBase = 0;
    std::string leaf;
    std::vector<u8> bytes;
    std::unordered_map<u32, JASWaveInfo> waves;  // waveId → info
    bool registered = false;
};

struct Bank {
    u32 bankId = 0;
    u32 virtBase = 0;  // key into s_arcs (never store WaveArc* — vector realloc)
    std::unordered_map<u32, Inst> insts;  // program → inst
    // Osc Data owned here; Point tables point into ibnkRaw (BE-tagged).
    std::vector<u8> ibnkRaw;
    std::vector<JASOscillator::Data> oscStore;
};

std::vector<WaveArc> s_arcs;
std::vector<Bank> s_banks;
bool s_loaded = false;
bool s_registered = false;

/** §87 CharVoice — separate from BGM so stopOwned unregister cannot mute lines. */
WaveArc s_voiceArc{};
bool s_voiceLoaded = false;
bool s_voiceRegistered = false;
u16 s_voiceSeId = 0x481F;  // from manifest se_id= (default donor JA_SE_CV_COMMON_PEOPLE)
struct VoiceCue {
    u8 sound = 0;
    u16 port = 0;
    bool hasPort = false;
};
std::unordered_map<u32, VoiceCue> s_voiceCues;
JASChannel* s_voiceCh = nullptr;

void voiceCallback(u32 type, JASChannel* ch, JASDsp::TChannel*, void*) {
    if (type != JASChannel::CB_STOP || ch == nullptr) {
        return;
    }
    if (s_voiceCh == ch) {
        s_voiceCh = nullptr;
    }
}

bool readFile(const fs::path& p, std::vector<u8>* out) {
    std::ifstream in(p, std::ios::binary);
    if (!in) {
        return false;
    }
    in.seekg(0, std::ios::end);
    const auto n = in.tellg();
    if (n <= 0) {
        return false;
    }
    in.seekg(0, std::ios::beg);
    out->resize(static_cast<size_t>(n));
    in.read(reinterpret_cast<char*>(out->data()), n);
    return static_cast<bool>(in) || in.eof();
}

u32 rd32(const u8* p) {
    u32 v;
    std::memcpy(&v, p, 4);
    return RES_U32(v);
}

f32 rdf32(const u8* p) {
    return std::bit_cast<f32>(rd32(p));
}

const u8* offPtr(const u8* base, u32 off) {
    return off == 0 ? nullptr : base + off;
}

std::string pathLeaf(const char* name) {
    std::string s(name ? name : "");
    // Trim at first NUL inside the 0x70 WSYS name field.
    const auto nul = s.find('\0');
    if (nul != std::string::npos) {
        s.resize(nul);
    }
    for (char& c : s) {
        if (c == '\\') {
            c = '/';
        }
    }
    const auto slash = s.find_last_of('/');
    if (slash != std::string::npos) {
        s = s.substr(slash + 1);
    }
    return s;
}

bool parseWsys(const u8* wsys, size_t size, WaveArc* arc) {
    if (size < 0x18 || rd32(wsys) != 0x57535953u) {  // 'WSYS'
        return false;
    }
    using P = JASWSParser;
    const auto* header = reinterpret_cast<const P::THeader*>(wsys);
    const P::TCtrlGroup* ctrlGroup = header->mCtrlGroupOffset.ptr(header);
    const P::TWaveArchiveBank* archiveBank = header->mArchiveBankOffset.ptr(header);
    if (ctrlGroup == nullptr || archiveBank == nullptr) {
        return false;
    }
    const u32 groupCount = ctrlGroup->mGroupCount;
    for (u32 i = 0; i < groupCount; i++) {
        const P::TCtrlScene* ctrlScene = ctrlGroup->mCtrlSceneOffsets[i].ptr(header);
        if (ctrlScene == nullptr) {
            continue;
        }
        const P::TCtrl* ctrl = ctrlScene->mCtrlOffset.ptr(header);
        const P::TWaveArchive* archive = archiveBank->mArchiveOffsets[i].ptr(header);
        if (ctrl == nullptr || archive == nullptr) {
            continue;
        }
        if (arc->leaf.empty()) {
            arc->leaf = pathLeaf(archive->mFileName);
        }
        const u32 waveCount = ctrl->mWaveCount;
        for (u32 j = 0; j < waveCount; j++) {
            const P::TWave* wave = archive->mWaveOffsets[j].ptr(header);
            const P::TCtrlWave* ctrlWave = ctrl->mCtrlWaveOffsets[j].ptr(header);
            if (wave == nullptr || ctrlWave == nullptr) {
                continue;
            }
            const u32 waveId = JSULoHalf(ctrlWave->_00);
            JASWaveInfo info;
            info.mWaveFormat = wave->mWaveFormat;
            info.mBaseKey = wave->mBaseKey;
            info.mSampleRate = wave->mSampleRate;
            info.mOffsetStart = static_cast<int>(wave->mAWOffsetStart);
            info.mOffsetLength = static_cast<int>(wave->mAWOffsetEnd);
            info.mLoopFlag = wave->mLoopFlags == 0 ? 0 : 0xff;
            info.mLoopStartSample = wave->mLoopStartSample;
            info.mLoopEndSample = static_cast<int>(wave->mLoopEndSample);
            info.mSampleCount = static_cast<int>(wave->mSampleCount);
            info.mpLast = wave->mpLast;
            info.mpPenult = wave->mpPenult;
            arc->waves[waveId] = info;
        }
    }
    return !arc->leaf.empty() && !arc->waves.empty();
}

JASOscillator::Point const* oscTableEnd(JASOscillator::Point const* points) {
    const JASOscillator::Point* ptr = points;
    while (true) {
        const s16 tmp = ptr->mEnvelopeMode;
        ptr++;
        if (tmp > 10) {
            break;
        }
    }
    return ptr;
}

const JASOscillator::Data* internOsc(Bank* bank, const u8* ibnk, u32 oscOff) {
    if (oscOff == 0) {
        return nullptr;
    }
    const u8* tosc = ibnk + oscOff;
    // TOsc: target u8, pad, rate f32, atk off, rel off, scale, vertex
    JASOscillator::Data data{};
    data.mTarget = tosc[0];
    data.mRate = rdf32(tosc + 4);
    const u32 atkOff = rd32(tosc + 8);
    const u32 relOff = rd32(tosc + 12);
    data.mScale = rdf32(tosc + 16);
    data.mVertex = rdf32(tosc + 20);
    // Point tables stay in ibnkRaw (BE-tagged Point) — stable for bank life.
    data.mTable = atkOff ? reinterpret_cast<const JASOscillator::Point*>(ibnk + atkOff) : nullptr;
    data.rel_table = relOff ? reinterpret_cast<const JASOscillator::Point*>(ibnk + relOff) : nullptr;
    // Silence unused end-scan; keep tables valid.
    if (data.mTable != nullptr) {
        (void)oscTableEnd(data.mTable);
    }
    if (data.rel_table != nullptr) {
        (void)oscTableEnd(data.rel_table);
    }
    bank->oscStore.push_back(data);
    return &bank->oscStore.back();
}

bool parseIbnk(Bank* bank) {
    const u8* ibnk = bank->ibnkRaw.data();
    const size_t size = bank->ibnkRaw.size();
    if (size < 0x24 || rd32(ibnk) != 0x49424E4Bu) {  // 'IBNK'
        return false;
    }
    bank->bankId = rd32(ibnk + 8);

    // Ver0/WW: 'BANK' at 0x20, inst offsets[0x80] at 0x24, perc at 0x3B4.
    if (size < 0x3E4 || rd32(ibnk + 0x20) != 0x42414E4Bu) {  // 'BANK'
        return false;
    }

    for (int i = 0; i < 0x80; i++) {
        const u32 instOff = rd32(ibnk + 0x24 + static_cast<size_t>(i) * 4);
        if (instOff == 0 || instOff >= size) {
            continue;
        }
        const u8* tinst = ibnk + instOff;
        Inst inst;
        inst.volume = rdf32(tinst + 8);
        inst.pitch = rdf32(tinst + 12);
        for (int o = 0; o < 2; o++) {
            const u32 oscOff = rd32(tinst + 0x10 + static_cast<size_t>(o) * 4);
            const JASOscillator::Data* od = internOsc(bank, ibnk, oscOff);
            if (od != nullptr && inst.oscCount < 2) {
                inst.osc[inst.oscCount++] = od;
            }
        }
        const u32 keyCount = rd32(tinst + 0x28);
        for (u32 k = 0; k < keyCount; k++) {
            const u32 keyOff = rd32(tinst + 0x2C + static_cast<size_t>(k) * 4);
            if (keyOff == 0 || keyOff >= size) {
                continue;
            }
            const u8* tkey = ibnk + keyOff;
            KeyRegion kr;
            kr.highKey = tkey[0];
            // WW: velo count at +4; first vmap offset at +8.
            const u32 veloN = rd32(tkey + 4);
            for (u32 v = 0; v < veloN; v++) {
                const u32 vmapOff = rd32(tkey + 8 + static_cast<size_t>(v) * 4);
                if (vmapOff == 0 || vmapOff >= size) {
                    continue;
                }
                const u8* tv = ibnk + vmapOff;
                VeloRegion vr;
                vr.maxVel = tv[0];
                vr.waveId = static_cast<u16>(rd32(tv + 4) & 0xFFFFu);
                vr.volScale = rdf32(tv + 8);
                vr.pitchScale = rdf32(tv + 12);
                kr.velos.push_back(vr);
            }
            if (!kr.velos.empty()) {
                inst.keys.push_back(std::move(kr));
            }
        }
        if (!inst.keys.empty()) {
            bank->insts[static_cast<u32>(i)] = std::move(inst);
        }
    }
    return !bank->insts.empty();
}

bool resolveInst(const Inst& inst, u8 key, u8 vel, u16* waveId, f32* vol, f32* pitch,
                 u8* outHighKey, u8* outMaxVel, u32* outKeyIdx, u32* outVelIdx) {
    *vol = inst.volume;
    *pitch = inst.pitch;
    const KeyRegion* kr = nullptr;
    u32 keyIdx = 0;
    for (u32 i = 0; i < inst.keys.size(); i++) {
        if (key <= inst.keys[i].highKey) {
            kr = &inst.keys[i];
            keyIdx = i;
            break;
        }
    }
    if (kr == nullptr) {
        return false;
    }
    for (u32 vi = 0; vi < kr->velos.size(); vi++) {
        const auto& v = kr->velos[vi];
        if (vel <= v.maxVel) {
            *vol *= v.volScale;
            *pitch *= v.pitchScale;
            *waveId = v.waveId;
            if (outHighKey != nullptr) {
                *outHighKey = kr->highKey;
            }
            if (outMaxVel != nullptr) {
                *outMaxVel = v.maxVel;
            }
            if (outKeyIdx != nullptr) {
                *outKeyIdx = keyIdx;
            }
            if (outVelIdx != nullptr) {
                *outVelIdx = vi;
            }
            return true;
        }
    }
    return false;
}

WaveArc* findOrMakeArc(u32 virtBase) {
    for (auto& a : s_arcs) {
        if (a.virtBase == virtBase) {
            return &a;
        }
    }
    s_arcs.push_back({});
    s_arcs.back().virtBase = virtBase;
    return &s_arcs.back();
}

bool loadOnePair(const fs::path& root, const char* ibnkName, const char* wsysName,
                 u32 virtBase) {
    std::vector<u8> wsys;
    if (!readFile(root / "aaf_slices" / wsysName, &wsys)) {
        DuskLog.warn("[ExtSeq] §53 missing {}", wsysName);
        return false;
    }
    WaveArc* arc = findOrMakeArc(virtBase);
    if (!parseWsys(wsys.data(), wsys.size(), arc)) {
        DuskLog.warn("[ExtSeq] §53 WSYS parse failed {}", wsysName);
        return false;
    }
    if (arc->bytes.empty()) {
        if (!readFile(root / "banks" / arc->leaf, &arc->bytes)) {
            DuskLog.warn("[ExtSeq] §53 missing banks/{}", arc->leaf);
            return false;
        }
    }

    // Parse into the live vector slot so Inst osc* stay inside oscStore
    // (reserve first — no reallocation during internOsc).
    s_banks.push_back({});
    Bank& live = s_banks.back();
    if (!readFile(root / "aaf_slices" / ibnkName, &live.ibnkRaw)) {
        s_banks.pop_back();
        DuskLog.warn("[ExtSeq] §53 missing {}", ibnkName);
        return false;
    }
    live.virtBase = virtBase;
    live.oscStore.reserve(256);
    if (!parseIbnk(&live)) {
        s_banks.pop_back();
        DuskLog.warn("[ExtSeq] §53 IBNK parse failed {}", ibnkName);
        return false;
    }
    DuskLog.info("[ExtSeq] §53 bank id={} ibnk={} waves={} aw={}b virt=0x{:08X}",
                 live.bankId, ibnkName, arc->waves.size(), arc->bytes.size(), virtBase);
    return true;
}

Bank* findBank(u8 bankId) {
    for (auto& b : s_banks) {
        if (b.bankId == bankId) {
            return &b;
        }
    }
    return nullptr;
}

WaveArc* findArc(u32 virtBase) {
    for (auto& a : s_arcs) {
        if (a.virtBase == virtBase) {
            return &a;
        }
    }
    return nullptr;
}

}  // namespace

bool ja1Bank_loadPackage(const fs::path& packageRoot) {
    if (s_loaded) {
        return true;
    }
    s_arcs.clear();
    s_banks.clear();
    s_registered = false;
    s_arcs.reserve(4);
    s_banks.reserve(4);

    bool ok0 = loadOnePair(packageRoot, "ibnk_0.bin", "wsys_0.bin", kVirtBaseZelda);
    bool ok2 = loadOnePair(packageRoot, "ibnk_21.bin", "wsys_2.bin", kVirtBaseLink);
    s_loaded = ok0;  // bank 0 is required for island BGM; bank 2 optional
    if (!ok2) {
        DuskLog.warn("[ExtSeq] §53 bank-2 pair missing — continuing with bank 0 only");
    }
    if (s_loaded) {
        DuskLog.info("[ExtSeq] §53 banks loaded ({} banks, {} arcs)", s_banks.size(),
                     s_arcs.size());
        // §76 key-region audit artifact (diff vs Bridge ibnk_initvol.csv).
        const fs::path keyCsv = packageRoot / "seq_key_regions_engine.csv";
        ja1Bank_dumpKeyRegionsCsv(keyCsv.string().c_str());
    }
    return s_loaded;
}

bool ja1Bank_ready() {
    return s_loaded;
}

void ja1Bank_register() {
    if (!s_loaded || s_registered) {
        return;
    }
    for (auto& arc : s_arcs) {
        if (arc.bytes.empty()) {
            continue;
        }
        dusk::audio::registerShadowWave(arc.virtBase, static_cast<u32>(arc.bytes.size()),
                                        arc.bytes.data());
        arc.registered = true;
    }
    s_registered = true;
    DuskLog.info("[ExtSeq] §53 shadow-wave registered arcs={}", s_arcs.size());
}

void ja1Bank_unregister() {
    if (!s_registered) {
        return;
    }
    for (auto& arc : s_arcs) {
        if (arc.registered) {
            dusk::audio::unregisterShadowWave(arc.virtBase);
            arc.registered = false;
        }
    }
    s_registered = false;
    DuskLog.info("[ExtSeq] §53 shadow-wave unregistered");
}

void ja1Bank_clear() {
    ja1Bank_unregister();
    s_banks.clear();
    s_arcs.clear();
    s_loaded = false;
}

JASChannel* ja1Bank_noteOn(u8 bankId, u8 prog, u8 key, u8 vel, u16 prio,
                           void (*cb)(u32, JASChannel*, JASDsp::TChannel*, void*),
                           void* cbArg) {
    if (!s_loaded || !s_registered) {
        return nullptr;
    }
    if (prog >= 0xF0) {
        return JASBank::noteOnOsc(prog - 0xF0, key, vel, prio, cb, cbArg);
    }
    Bank* bank = findBank(bankId);
    WaveArc* arc = bank != nullptr ? findArc(bank->virtBase) : nullptr;
    if (bank == nullptr || arc == nullptr) {
        return nullptr;
    }
    auto it = bank->insts.find(prog);
    if (it == bank->insts.end()) {
        return nullptr;
    }
    u16 waveId = 0;
    f32 vol = 1.0f;
    f32 pitch = 1.0f;
    u8 highKey = 0;
    u8 maxVel = 0;
    u32 keyIdx = 0;
    u32 velIdx = 0;
    if (!resolveInst(it->second, key, vel, &waveId, &vol, &pitch, &highKey, &maxVel, &keyIdx,
                     &velIdx)) {
        return nullptr;
    }
    auto wit = arc->waves.find(waveId);
    if (wit == arc->waves.end()) {
        return nullptr;
    }
    const JASWaveInfo& wi = wit->second;
    {
        const char* kp = std::getenv("DUSK_EXTSEQ_KEY_AUDIT");
        const bool on = kp != nullptr && kp[0] != '\0' && !(kp[0] == '0' && kp[1] == '\0');
        if (on) {
            static u32 s_keyAuditLeft = 48;
            if (s_keyAuditLeft > 0) {
                s_keyAuditLeft--;
                DuskLog.info(
                    "[ExtSeq] §76 keyAudit bank={} prog={} key={} vel={} → wave={} "
                    "highKey={} keyReg={} velReg={} baseKey={} pitchScale={:.4f}",
                    static_cast<unsigned>(bankId), static_cast<unsigned>(prog),
                    static_cast<unsigned>(key), static_cast<unsigned>(vel),
                    static_cast<unsigned>(waveId), static_cast<unsigned>(highKey), keyIdx,
                    velIdx, static_cast<unsigned>(wi.mBaseKey), pitch);
            }
        }
    }
    const u32 start = static_cast<u32>(wi.mOffsetStart);
    if (start >= arc->bytes.size()) {
        return nullptr;
    }
    const u32 waveAddr = dusk::audio::kShadowVirtualBase + arc->virtBase + start;

    JASChannel* channel = JKR_NEW JASChannel(cb, cbArg);
    if (channel == nullptr) {
        return nullptr;
    }
    channel->setPriority(prio);
    channel->field_0xdc.mWaveInfo = wi;
    channel->mWaveAramAddress = waveAddr;
    channel->field_0xdc.mChannelType = 0;  // WAVE
    channel->setInitPitch(pitch * (wi.mSampleRate / JASDriver::getDacRate()));
    channel->setKey(static_cast<s32>(key) - static_cast<s32>(wi.mBaseKey));
    // №31-B: bake WW field_0x5c = initVol*(vel/127)²; neutralize TP vel².
    ja1Bank_applyWwVelocityCurve(channel, vol, vel);
    channel->setInitPan(0.5f);
    // §81: type-7 wetness is scene Fxline + mixer path — BMS fxmix is 0.
    // Put ExtSeq on AutoMixer (dolby) so DuskDsp freeverb receives mAutoMixerFxMix,
    // and seed initFxmix from the active scene send derived from type-7 buses.
    channel->mMixConfig[0].whole = 0xffff;
    channel->setInitFxmix(dusk::audio::getExtSeqFxSend());
    channel->setInitDolby(0.0f);
    for (u8 i = 0; i < it->second.oscCount; i++) {
        if (it->second.osc[i] != nullptr) {
            channel->setOscInit(i, it->second.osc[i]);
        }
    }
    if (!channel->play()) {
        return nullptr;
    }
    return channel;
}

void ja1Bank_applyWwVelocityCurve(JASChannel* ch, f32 initVol, u8 vel) {
    if (ch == nullptr) {
        return;
    }
    // WW BankMgr::noteOnOsc (WAVE noteOn same shape — noteOn itself still
    // nonmatching in tww, but noteOnOsc is the named authority):
    //   field_0x0  = velocity
    //   field_0x54 = init volume
    //   field_0x5c = field_0x54 * (field_0x0/127)²
    f32 vn = static_cast<f32>(vel) / 127.0f;
    if (vn < 0.0f) {
        vn = 0.0f;
    }
    if (vn > 1.0f) {
        vn = 1.0f;
    }
    ch->setInitVolume(initVol * (vn * vn));
    ch->setVelocity(0x7F);  // TP per-frame (vel/127)² → 1.0
    ch->setBankDisposeID(&dusk::audio::g_extSeqOwnerTag);
}

bool ja1Bank_isExtSeqChannel(const JASChannel* ch) {
    return ch != nullptr && dusk::audio::isExtSeqOwned(ch->mBankDisposeID);
}

u32 ja1Bank_dumpKeyRegionsCsv(const char* outPath) {
    if (outPath == nullptr || !s_loaded || s_banks.empty()) {
        return 0;
    }
    FILE* f = std::fopen(outPath, "wb");
    if (f == nullptr) {
        DuskLog.warn("[ExtSeq] §76 key-region dump: cannot write {}", outPath);
        return 0;
    }
    // Columns match Bridge ibnk_initvol.csv for direct diff.
    std::fputs(
        "bms_vir,phys_ibnk,prog,key_region,high_key,vel_region,max_vel,"
        "inst_volume,vel_vol_scale,resolved_init_vol,wave_id,pitch_scale\n",
        f);
    u32 rows = 0;
    for (u32 bi = 0; bi < s_banks.size(); bi++) {
        const Bank& bank = s_banks[bi];
        // Stable iteration by prog id.
        std::vector<u32> progs;
        progs.reserve(bank.insts.size());
        for (const auto& kv : bank.insts) {
            progs.push_back(kv.first);
        }
        std::sort(progs.begin(), progs.end());
        for (u32 prog : progs) {
            const Inst& inst = bank.insts.at(prog);
            for (u32 ki = 0; ki < inst.keys.size(); ki++) {
                const KeyRegion& kr = inst.keys[ki];
                for (u32 vi = 0; vi < kr.velos.size(); vi++) {
                    const VeloRegion& vr = kr.velos[vi];
                    const f32 resolved = inst.volume * vr.volScale;
                    std::fprintf(f,
                                 "0,%u,%u,%u,%u,%u,%u,%.6f,%.6f,%.6f,%u,%.6f\n", bi, prog, ki,
                                 static_cast<unsigned>(kr.highKey), vi,
                                 static_cast<unsigned>(vr.maxVel), inst.volume, vr.volScale,
                                 resolved, static_cast<unsigned>(vr.waveId), vr.pitchScale);
                    rows++;
                }
            }
        }
    }
    std::fclose(f);
    DuskLog.info("[ExtSeq] §76 key-region dump: {} rows → {}", rows, outPath);
    return rows;
}

namespace {

bool playCharVoiceWave(u16 waveId) {
    if (!s_voiceLoaded || !s_voiceRegistered) {
        return false;
    }
    auto wit = s_voiceArc.waves.find(waveId);
    if (wit == s_voiceArc.waves.end()) {
        DuskLog.warn("[ExtSeq] §87 CharVoice wave {} missing in WSYS",
                     static_cast<unsigned>(waveId));
        return false;
    }
    const JASWaveInfo& wi = wit->second;
    const u32 start = static_cast<u32>(wi.mOffsetStart);
    if (start >= s_voiceArc.bytes.size()) {
        return false;
    }
    // Donor charVoicePlay stops the prior common-people handle before restart.
    if (s_voiceCh != nullptr) {
        s_voiceCh->release(0);
        s_voiceCh = nullptr;
    }
    JASChannel* channel = JKR_NEW JASChannel(voiceCallback, nullptr);
    if (channel == nullptr) {
        return false;
    }
    const u32 waveAddr =
        dusk::audio::kShadowVirtualBase + s_voiceArc.virtBase + start;
    channel->setPriority(0x7F);
    channel->field_0xdc.mWaveInfo = wi;
    channel->mWaveAramAddress = waveAddr;
    channel->field_0xdc.mChannelType = 0;
    channel->setInitPitch(wi.mSampleRate / JASDriver::getDacRate());
    channel->setKey(0);  // one-shot at sample baseKey; no transpose
    // SoundTable SE class vol_u8=127 — full; do NOT apply BGM master (~0.47).
    ja1Bank_applyWwVelocityCurve(channel, 1.0f, 0x7F);
    channel->setInitPan(0.5f);
    channel->mMixConfig[0].whole = 0xffff;
    // §93: voice one-shots use per-room reverb like messageSePlay / playOneShotVoice
    // (dComIfGp_getReverb(room)/127) — NOT the §81 music-scene Fxline send (~0.5).
    // TP callers pass reverb=0 when stayNo==0; same gate here.
    const s32 stayNo = dComIfGp_roomControl_getStayNo();
    const s8 roomRev =
        (stayNo != 0) ? dComIfGp_getReverb(stayNo) : static_cast<s8>(0);
    f32 voiceFx = static_cast<f32>(static_cast<u8>(roomRev & 0x7F)) / 127.0f;
    if (voiceFx < 0.0f) {
        voiceFx = 0.0f;
    }
    if (voiceFx > 1.0f) {
        voiceFx = 1.0f;
    }
    const f32 musicSend = dusk::audio::getExtSeqFxSend();
    DuskLog.info(
        "[ExtSeq] §93 voice fxmix: music_send={:.3f} room={} reverb_s8={} voice_send={:.3f} "
        "(messageSePlay law)",
        musicSend, stayNo, static_cast<int>(roomRev), voiceFx);
    channel->setInitFxmix(voiceFx);
    channel->setInitDolby(0.0f);
    if (!channel->play()) {
        return false;
    }
    s_voiceCh = channel;
    return true;
}

void trimInPlace(std::string* s) {
    while (!s->empty() && (s->front() == ' ' || s->front() == '\t')) {
        s->erase(s->begin());
    }
    while (!s->empty() && (s->back() == ' ' || s->back() == '\t' || s->back() == '\r')) {
        s->pop_back();
    }
}

}  // namespace

bool ja1Voice_loadPackage(const fs::path& voiceRoot) {
    if (s_voiceLoaded) {
        return true;
    }
    s_voiceCues.clear();
    s_voiceArc = {};
    s_voiceArc.virtBase = kVirtBaseCharVoice;

    const fs::path mani = voiceRoot / "manifest.ini";
    std::ifstream in(mani);
    if (!in) {
        DuskLog.warn("[ExtSeq] §87 missing {}", mani.string());
        return false;
    }

    std::string awRel;
    std::string wsysRel;
    std::string ibnkRel;
    std::string line;
    while (std::getline(in, line)) {
        trimInPlace(&line);
        if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '[') {
            continue;
        }
        const size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue;
        }
        std::string key = line.substr(0, eq);
        std::string val = line.substr(eq + 1);
        trimInPlace(&key);
        trimInPlace(&val);
        if (key == "se_id") {
            s_voiceSeId = static_cast<u16>(std::strtoul(val.c_str(), nullptr, 0));
            continue;
        }
        if (key.size() > 5 && key.substr(key.size() - 5) == ".file") {
            awRel = val;
            continue;
        }
        if (key.size() > 11 && key.substr(key.size() - 11) == ".wsys_slice") {
            wsysRel = val;
            continue;
        }
        if (key.size() > 11 && key.substr(key.size() - 11) == ".ibnk_slice") {
            ibnkRel = val;
            continue;
        }
        // msg.0x357.sound=104 / msg.0x357.port=0x1910
        if (key.rfind("msg.", 0) == 0) {
            const size_t dot2 = key.find('.', 4);
            if (dot2 == std::string::npos) {
                continue;
            }
            const u32 msgId =
                static_cast<u32>(std::strtoul(key.substr(4, dot2 - 4).c_str(), nullptr, 0));
            const std::string field = key.substr(dot2 + 1);
            VoiceCue& cue = s_voiceCues[msgId];
            if (field == "sound") {
                cue.sound = static_cast<u8>(std::strtoul(val.c_str(), nullptr, 0));
            } else if (field == "port") {
                cue.port = static_cast<u16>(std::strtoul(val.c_str(), nullptr, 0));
                cue.hasPort = true;
            }
        }
    }

    if (awRel.empty() || wsysRel.empty()) {
        DuskLog.warn("[ExtSeq] §87 voice manifest missing .file / .wsys_slice");
        return false;
    }
    std::vector<u8> wsys;
    if (!readFile(voiceRoot / wsysRel, &wsys)) {
        DuskLog.warn("[ExtSeq] §87 missing {}", wsysRel);
        return false;
    }
    if (!parseWsys(wsys.data(), wsys.size(), &s_voiceArc)) {
        DuskLog.warn("[ExtSeq] §87 WSYS parse failed");
        return false;
    }
    // Prefer manifest leaf; fall back to WSYS-embedded name.
    if (!awRel.empty()) {
        s_voiceArc.leaf = pathLeaf(awRel.c_str());
    }
    if (!readFile(voiceRoot / awRel, &s_voiceArc.bytes)) {
        // try banks/<leaf> if relative path failed oddly
        if (!readFile(voiceRoot / "banks" / s_voiceArc.leaf, &s_voiceArc.bytes)) {
            DuskLog.warn("[ExtSeq] §87 missing {}", awRel);
            return false;
        }
    }
    (void)ibnkRel;  // staged for twin; wave-direct one-shot does not need INST keys

    s_voiceLoaded = true;
    DuskLog.info(
        "[ExtSeq] §87 CharVoice loaded se_id=0x{:04X} waves={} aw={}b cues={} virt=0x{:08X}",
        static_cast<unsigned>(s_voiceSeId), s_voiceArc.waves.size(), s_voiceArc.bytes.size(),
        s_voiceCues.size(), kVirtBaseCharVoice);
    return true;
}

bool ja1Voice_ready() {
    return s_voiceLoaded;
}

void ja1Voice_register() {
    if (!s_voiceLoaded || s_voiceRegistered) {
        return;
    }
    if (s_voiceArc.bytes.empty()) {
        return;
    }
    dusk::audio::registerShadowWave(s_voiceArc.virtBase,
                                    static_cast<u32>(s_voiceArc.bytes.size()),
                                    s_voiceArc.bytes.data());
    s_voiceArc.registered = true;
    s_voiceRegistered = true;
    DuskLog.info("[ExtSeq] §87 CharVoice shadow-wave registered");
}

void ja1Voice_unregister() {
    if (!s_voiceRegistered) {
        return;
    }
    if (s_voiceCh != nullptr) {
        s_voiceCh->release(0);
        s_voiceCh = nullptr;
    }
    if (s_voiceArc.registered) {
        dusk::audio::unregisterShadowWave(s_voiceArc.virtBase);
        s_voiceArc.registered = false;
    }
    s_voiceRegistered = false;
}

void ja1Voice_clear() {
    ja1Voice_unregister();
    s_voiceArc = {};
    s_voiceCues.clear();
    s_voiceLoaded = false;
}

void ja1Voice_onDemoMessageOpen(u32 donorMsgId) {
    if (!s_voiceLoaded) {
        return;
    }
    const auto it = s_voiceCues.find(donorMsgId);
    if (it == s_voiceCues.end()) {
        return;  // not in package map — do not invent
    }
    const VoiceCue& cue = it->second;
    if (cue.sound == 0) {
        DuskLog.info("[ExtSeq] §87 msg 0x{:X} silent (sound=0, donor)", donorMsgId);
        return;
    }
    if (!cue.hasPort) {
        DuskLog.warn("[ExtSeq] §87 msg 0x{:X} sound={} but no port in map", donorMsgId,
                     static_cast<unsigned>(cue.sound));
        return;
    }
    ja1Voice_register();
    // Donor: JA_SE_CV_COMMON_PEOPLE + setPortData(8, port). PC: port low = WSYS wave id
    // (sizes match Lago Aryll clips; SE BMS not ported).
    const u16 waveId = static_cast<u16>(cue.port & 0xFF);
    const bool ok = playCharVoiceWave(waveId);
    DuskLog.info(
        "[ExtSeq] §87 msg 0x{:X} se=0x{:04X} sound={} port=0x{:04X} wave={} → {}",
        donorMsgId, static_cast<unsigned>(s_voiceSeId), static_cast<unsigned>(cue.sound),
        static_cast<unsigned>(cue.port), static_cast<unsigned>(waveId),
        ok ? "play" : "FAIL");
}

}  // namespace ExtSeq

#else  // !TARGET_PC

namespace ExtSeq {

bool ja1Bank_loadPackage(const std::filesystem::path&) { return false; }
bool ja1Bank_ready() { return false; }
void ja1Bank_register() {}
void ja1Bank_unregister() {}
void ja1Bank_clear() {}
JASChannel* ja1Bank_noteOn(u8, u8, u8, u8, u16, void (*)(u32, JASChannel*, JASDsp::TChannel*, void*),
                           void*) {
    return nullptr;
}
void ja1Bank_applyWwVelocityCurve(JASChannel*, f32, u8) {}
bool ja1Bank_isExtSeqChannel(const JASChannel*) { return false; }
u32 ja1Bank_dumpKeyRegionsCsv(const char*) { return 0; }
bool ja1Voice_loadPackage(const std::filesystem::path&) { return false; }
bool ja1Voice_ready() { return false; }
void ja1Voice_register() {}
void ja1Voice_unregister() {}
void ja1Voice_clear() {}
void ja1Voice_onDemoMessageOpen(u32) {}

}  // namespace ExtSeq

#endif  // TARGET_PC
