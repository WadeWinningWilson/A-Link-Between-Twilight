/**
 * ja1_bank.cpp — §53 bank residency via shadow-wave virtual addresses.
 *
 * Parse staged ibnk_*.bin / wsys_*.bin, keep .aw in ExtSeq RAM, register at
 * virtual bases. noteOn mints kShadowVirtualBase + virtBase + offset.
 */

#include "d/ext_seq/ja1_bank.h"

#include "dolphin/types.h"

#if TARGET_PC

#include <bit>
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

bool resolveInst(const Inst& inst, u8 key, u8 vel, u16* waveId, f32* vol, f32* pitch) {
    *vol = inst.volume;
    *pitch = inst.pitch;
    const KeyRegion* kr = nullptr;
    for (const auto& k : inst.keys) {
        if (key <= k.highKey) {
            kr = &k;
            break;
        }
    }
    if (kr == nullptr) {
        return false;
    }
    for (const auto& v : kr->velos) {
        if (vel <= v.maxVel) {
            *vol *= v.volScale;
            *pitch *= v.pitchScale;
            *waveId = v.waveId;
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
    if (!resolveInst(it->second, key, vel, &waveId, &vol, &pitch)) {
        return nullptr;
    }
    auto wit = arc->waves.find(waveId);
    if (wit == arc->waves.end()) {
        return nullptr;
    }
    const JASWaveInfo& wi = wit->second;
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
    channel->setInitFxmix(0.0f);
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

}  // namespace ExtSeq

#endif  // TARGET_PC
