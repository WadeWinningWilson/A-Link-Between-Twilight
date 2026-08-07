// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASBank.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASBankMgr.cpp NonMatching
// KIT-DONOR: JSystem/JAudio/JASBasicBank.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASBasicInst.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASInstEffect.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASInstRand.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASInstSense.cpp Matching
// KIT-DONOR: JSystem/JAudio/JASDrumSet.cpp Matching
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: UNKNOWN
// ============================================================
// §368 JAudio1 bank/instrument layer — donor-verbatim port of WW retail
// JSystem/JAudio bank TUs (D:/XXXXXXX/WW DP; dtk address markers kept).
// Phase A3 of the native JA1 campaign (bus §362/§363/§366). Donor
// JASystem::* flattens into JAudio1::* (§363 convention). Nothing in
// the game calls this TU yet; the ExtSeq bridge keeps playing until A4.
// Donor TU map (§368 named adaptation "TU consolidation"):
//   [1] JASBank.cpp        [2] JASBankMgr.cpp (noteOn/gateOn were
//       Nonmatching-EMPTY in the donor decomp — reconstructed
//       instruction-by-instruction from the donor's own retail asm,
//       build/GZLE01/asm/JSystem/JAudio/JASBankMgr.s)
//   [3] JASBasicBank.cpp   [4] JASBasicInst.cpp
//   [5] JASInstEffect.cpp  [6] JASInstRand.cpp  [7] JASInstSense.cpp
//   [8] JASDrumSet.cpp
// Donor include map: JASCalc.h/JUTAssert.h/JASRate.h(Kernel) ->
// ja1_boundary.h; JASChannel.h/JASChannelMgr.h -> ja1_jaschannel.h;
// JASDriverTables.h -> ja1_dsp_boundary.h; JMath/random.h -> receiver.
// §368 heap adaptation (A2 precedent): every donor
// `new (heap, 0)` / `new (JASDram, 0)` is a plain new — no JA1 arena
// exists on the PC side until A4 sizes the driver heaps. The paired
// donor delete[]s stay valid (global operator new/delete).
// ============================================================

#include "d/ext_seq/ja1_jasbank.h"
#include "dusk/logging.h"  // §373 pipeline probes

#include "d/ext_seq/ja1_jaschannel.h"
#include "JSystem/JMath/random.h"  // JMath::TRandom_fast_ (donor JSystem/JMath/random.h — same lineage)
#include "global.h"
#include "dolphin/os.h"

namespace JAudio1 {

// ============================================================
// §368 donor TU [1]: JASBank.cpp
// ============================================================

JKRHeap* TBank::sCurrentHeap;

/* 80284548-8028455C       .text getCurrentHeap__Q28JASystem5TBankFv */
JKRHeap* TBank::getCurrentHeap() {
    /* Nonmatching */
    if (sCurrentHeap) {
        return sCurrentHeap;
    }
    // ===== §368: donor falls back to JASDram (the WW JKRSolidHeap audio
    // arena). No JA1 arena exists on the PC side until A4; all A3-port
    // allocations use plain new, so this fallback is never dereferenced.
    return NULL;
}

// ============================================================
// §368 donor TU [2]: JASBankMgr.cpp
// ============================================================

s16 BankMgr::OSC_RELEASE_TABLE[6] = { 1, 10, 0, 15, 0, 0 };
TOscillator::Osc_ BankMgr::OSC_ENV = { 0, 1.0f, NULL, BankMgr::OSC_RELEASE_TABLE, 1.0f, 0.0f };

s32 BankMgr::sTableSize;
TBank** BankMgr::sBankArray;
u16* BankMgr::sVir2PhyTable;

/* 80288594-80288698       .text init__Q28JASystem7BankMgrFi */
void BankMgr::init(int param_1) {
    u32 r31 = param_1 * 4;
    sBankArray = (TBank**)new u8[r31];  // ===== §368: donor `new (JASDram, 0)`
    JUT_ASSERT(69, sBankArray != NULL);
    sVir2PhyTable = new u16[param_1];  // ===== §368: donor `new (JASDram, 0)`
    JUT_ASSERT(72, sVir2PhyTable != NULL);
    Calc::bzero(sBankArray, r31);
    for (int i = 0; i < param_1; i++) {
        sVir2PhyTable[i] = 0xFFFF;
    }
    sTableSize = param_1;
}

/* 80288698-8028874C       .text registBank__Q28JASystem7BankMgrFiPQ28JASystem5TBank */
bool BankMgr::registBank(int banknum, TBank* bank) {
    JUT_ASSERT(86, banknum >= 0);
    JUT_ASSERT(87, banknum < sTableSize);
    sBankArray[banknum] = bank;
    return true;
}

/* 8028874C-802887AC       .text registBankBNK__Q28JASystem7BankMgrFiPv */
bool BankMgr::registBankBNK(int banknum, void* param_2) {
    setVir2PhyTable(*((u32*)(param_2) + 2), banknum);
    TBasicBank* bank = BNKParser::createBasicBank(param_2);
    if (bank == NULL) {
        return false;
    }
    return registBank(banknum, bank);
}

/* 802887AC-802887E0       .text getBank__Q28JASystem7BankMgrFi */
TBank* BankMgr::getBank(int param_1) {
    if (param_1 < 0) {
        return NULL;
    }
    if (param_1 >= sTableSize) {
        return NULL;
    }
    return sBankArray[param_1];
}

/* 802887E0-802887F0       .text getPhysicalNumber__Q28JASystem7BankMgrFUs */
u16 BankMgr::getPhysicalNumber(u16 param_1) {
    return sVir2PhyTable[param_1];
}

/* 802887F0-802888A0       .text setVir2PhyTable__Q28JASystem7BankMgrFUli */
void BankMgr::setVir2PhyTable(u32 vir_id, int banknum) {
    if (vir_id == 0xFFFF) {
        return;
    }
    JUT_ASSERT(127, vir_id < (u32)sTableSize);
    if (sVir2PhyTable[vir_id] != 0xFFFF) {
        OSReport("Warning : Duplicated Bank vir_id ID %d (%d,%d)\n", vir_id, sVir2PhyTable[vir_id], banknum);
    }
    sVir2PhyTable[vir_id] = banknum;
}

/* 802888A0-80288904       .text assignWaveBank__Q28JASystem7BankMgrFii */
bool BankMgr::assignWaveBank(int param_1, int param_2) {
    TBank* bank = getBank(param_1);
    if (!bank) {
        return false;
    }
    TWaveBank* waveBank = WaveBankMgr::getWaveBank(param_2);
    if (!waveBank) {
        return false;
    }
    bank->field_0x4 = waveBank;
    return true;
}

/* 80288904-8028892C       .text clamp01__Q28JASystem7BankMgrFf */
f32 BankMgr::clamp01(f32 param_1) {
    if (param_1 < 0.0f) {
        return 0.0f;
    }
    if (param_1 > 1.0f) {
        return 1.0f;
    }
    return param_1;
}

/* 8028892C-80288CE8       .text noteOn__Q28JASystem7BankMgrFPQ28JASystem11TChannelMgriiUcUcUl */
// ===== §368: donor decomp body was Nonmatching-EMPTY; reconstructed
// instruction-by-instruction from GZLE01 retail asm (JASBankMgr.s
// :297-559). Stack TInstParam init matches the donor TInstParam()
// ctor exactly; vtable call order = getInst -> getParam ->
// getWaveHandle -> getWaveInfo -> getWavePtr, donor-verified.
TChannel* BankMgr::noteOn(TChannelMgr* param_1, int banknum, int prognum, u8 key, u8 velo, u32 param_6) {
#if TARGET_PC
    // §373c pipeline probe (voice stage) — strip at A5 acceptance.
    { static int n = 0; if (n < 4) { ++n; DuskLog.info("[JA1] §373c noteOn #{} bank={} prog={} key={} vel={}", n, banknum, prognum, (int)key, (int)velo); } }
#endif
    if (prognum > 0xEF) {
        return noteOnOsc(param_1, prognum - 0xF0, key, velo, param_6);
    }
    TBank* bank = getBank(banknum);
    if (bank == NULL) {
        return NULL;
    }
    TInst* inst = bank->getInst(prognum);
    if (inst == NULL) {
        return NULL;
    }
    TInstParam instParam;
    if (!inst->getParam(key, velo, &instParam)) {
        return NULL;
    }
    if (bank->field_0x4 == NULL) {
        return NULL;
    }
    TWaveHandle* handle = bank->field_0x4->getWaveHandle(instParam.field_0x4);
    if (handle == NULL) {
        return NULL;
    }
    const TWaveInfo* waveInfo = handle->getWaveInfo();
    if (waveInfo == NULL) {
        return NULL;
    }
    intptr_t wavePtr = handle->getWavePtr();  // ===== §368: donor const void* — x64 widening
    if (wavePtr == 0) {
        return NULL;
    }
    u32 allockey = (instParam.field_0x40 << 24) | (banknum << 8) | prognum;
    switch (instParam.field_0x40 & 0xC0) {
    case 0x80:
        allockey |= 0xFF;
        break;
    case 0x40:
        allockey |= instParam.field_0x3c << 16;
        break;
    case 0xC0:
        allockey |= 0xFFFFFF;
        break;
    }
    TChannel* channel = param_1->getLogicalChannel(allockey);
    if (channel == NULL) {
        return NULL;
    }
    channel->field_0x10 = (Driver::Wave_*)waveInfo;
    channel->field_0x14 = wavePtr;
    channel->field_0xc = instParam.field_0x0;
    channel->field_0x0 = velo;
    channel->field_0x1 = key;
    channel->field_0x50 = instParam.field_0x14 * (waveInfo->field_0x4 / Kernel::getDacRate());
    channel->field_0x58 = channel->field_0x50 * instParam.field_0x1c;
    if (instParam.field_0x38 == 0) {
        s32 r0 = (s32)key + 0x3C - waveInfo->field_0x2;
        if (r0 < 0) {
            r0 = 0;
        }
        if (r0 > 0x7F) {
            r0 = 0x7F;
        }
        channel->field_0x58 *= Driver::C5BASE_PITCHTABLE[r0];
    }
    channel->field_0x54 = instParam.field_0x10;
    channel->field_0x5c = channel->field_0x0 / 127.0f;
    channel->field_0x5c = channel->field_0x54 * (channel->field_0x5c * channel->field_0x5c);
    channel->field_0x5c *= instParam.field_0x18;
    channel->mPanVec.mSound = instParam.field_0x20;
    channel->mFxmixVec.mSound = instParam.field_0x24;
    channel->mDolbyVec.mSound = instParam.field_0x28;
    for (int i = 1; i < 3; i++) {
        (&channel->mPanPower)[i].mSound = clamp01((&channel->mPanPower)[i].mSound);
    }
    channel->mPanVec.mEffect = instParam.field_0x2c;
    channel->mFxmixVec.mEffect = instParam.field_0x30;
    channel->mDolbyVec.mEffect = instParam.field_0x34;
    channel->field_0x94 = 1.0f;
    channel->field_0x98 = 1.0f;
    for (u32 i = 0; i < instParam.mOscCount; i++) {
        channel->setOscInit(i, instParam.mOscData[i]);
    }
    channel->directReleaseOsc(0, instParam.field_0x3a);
    if (!channel->play(param_6)) {
        return NULL;
    }
    return channel;
}

/* 80288CE8-80288E44       .text noteOnOsc__Q28JASystem7BankMgrFPQ28JASystem11TChannelMgriUcUcUl */
TChannel* BankMgr::noteOnOsc(TChannelMgr* param_1, int param_2, u8 param_3, u8 param_4, u32 param_5) {
    /* Nonmatching */
    TChannel* channel = param_1->getLogicalChannel(0);
    if (!channel) {
        return NULL;
    }
    channel->field_0x14 = param_2;
    channel->field_0xc = 2;
    channel->field_0x0 = param_4;
    channel->field_0x1 = param_3;
    channel->field_0x50 = 16736.016f / Kernel::getDacRate();
    channel->field_0x58 = channel->field_0x50;
    s32 var1 = param_3;
    if (var1 < 0) {
        var1 = 0;
    }
    if (var1 > 127) {
        var1 = 127;
    }
    channel->field_0x58 *= Driver::C5BASE_PITCHTABLE[var1];
    channel->field_0x54 = 1.0f;
    channel->field_0x5c = channel->field_0x0 / 127.0f;
    channel->field_0x5c = channel->field_0x54 * (channel->field_0x5c * channel->field_0x5c);
    channel->mPanVec.mSound = 0.5f;
    channel->mFxmixVec.mSound = 0.0f;
    channel->mDolbyVec.mSound = 0.0f;
    channel->mPanVec.mEffect = 0.5f;
    channel->mFxmixVec.mEffect = 0.0f;
    channel->mDolbyVec.mEffect = 0.0f;
    channel->field_0x94 = 1.0f;
    channel->field_0x98 = 1.0f;
    channel->setOscInit(0, &OSC_ENV);
    channel->field_0xcc = 0;
    if (!channel->play(param_5)) {
        return NULL;
    }
    return channel;
}

/* 80288E44-80288F08       .text gateOn__Q28JASystem7BankMgrFPQ28JASystem8TChannelUcUcUl */
// ===== §368: donor decomp body was Nonmatching-EMPTY; reconstructed
// instruction-by-instruction from GZLE01 retail asm (JASBankMgr.s
// :660-715).
void BankMgr::gateOn(TChannel* channel, u8 key, u8 velo, u32 param_4) {
    if (channel->field_0x30 != -1) {
        return;
    }
    channel->field_0x30 = param_4;
    channel->field_0x34 = channel->field_0x30;
    s32 r0;
    if (channel->field_0xc == 2) {
        r0 = key;
    } else {
        r0 = (s32)key + 0x3C - channel->field_0x10->field_0x2;
    }
    if (r0 < 0) {
        r0 = 0;
    }
    if (r0 > 0x7F) {
        r0 = 0x7F;
    }
    channel->field_0x0 = velo;
    channel->field_0x1 = key;
    channel->field_0x58 = channel->field_0x50 * Driver::C5BASE_PITCHTABLE[r0];
    channel->field_0x5c = channel->field_0x0 / 127.0f;
    channel->field_0x5c = channel->field_0x54 * (channel->field_0x5c * channel->field_0x5c);
}

// ============================================================
// §368 donor TU [3]: JASBasicBank.cpp
// ============================================================

/* 80284570-8028459C       .text __ct__Q28JASystem10TBasicBankFv */
TBasicBank::TBasicBank() {
    mInstTable = NULL;
    mInstCount = 0;
}

/* 8028459C-80284610       .text __dt__Q28JASystem10TBasicBankFv */
TBasicBank::~TBasicBank() {
    delete[] mInstTable;
}

/* 80284610-802846B0       .text setInstCount__Q28JASystem10TBasicBankFUl */
void TBasicBank::setInstCount(u32 param_1) {
    delete[] mInstTable;
    mInstTable = new TInst*[param_1];  // ===== §368: donor `new (getCurrentHeap(), 0)`
    JUT_ASSERT(36, mInstTable != NULL);
    Calc::bzero(mInstTable, param_1 * sizeof(TInst*));  // ===== §372: donor `*4` = GC pointer size; x64 zeroed only HALF the table → garbage getInst → the first-light crash (findOscPtr→getParam). sizeof(TInst*) is the §368 widening class.
    mInstCount = param_1;
}

/* 802846B0-80284768       .text setInst__Q28JASystem10TBasicBankFiPQ28JASystem5TInst */
void TBasicBank::setInst(int prg_no, TInst* param_2) {
    JUT_ASSERT(45, prg_no < (int)mInstCount);
    JUT_ASSERT(46, prg_no >= 0);
    mInstTable[prg_no] = param_2;
}

/* 80284768-802847F0       .text getInst__Q28JASystem10TBasicBankCFi */
TInst* TBasicBank::getInst(int prg_no) const {
    JUT_ASSERT(53, prg_no >= 0);
    if (prg_no >= (int)mInstCount) {
        return 0;
    }
    return mInstTable[prg_no];
}

// ============================================================
// §368 donor TU [4]: JASBasicInst.cpp
// ============================================================

/* 80284844-80284888       .text __ct__Q28JASystem10TBasicInstFv */
TBasicInst::TBasicInst() {
    field_0x4 = 1.0f;
    field_0x8 = 1.0f;
    mEffect = NULL;
    mEffectCount = 0;
    mOsc = NULL;
    mOscCount = 0;
    mKeyRegionCount = 0;
    mKeymap = NULL;
}

/* 80284888-80284914       .text __dt__Q28JASystem10TBasicInstFv */
TBasicInst::~TBasicInst() {
    delete[] mKeymap;
    delete[] mEffect;
    delete[] mOsc;
}

/* 80284914-80284B4C       .text getParam__Q28JASystem10TBasicInstCFiiPQ28JASystem10TInstParam */
bool TBasicInst::getParam(int key, int velo, TInstParam* param) const {
    param->field_0x0 = 0;
    param->field_0x38 = 0;
    param->mOscData = mOsc;
    param->mOscCount = mOscCount;
    param->field_0x10 *= field_0x4;
    param->field_0x14 *= field_0x8;
    for (int i = 0; i < (int)mEffectCount; i++) {
        TInstEffect* effect = mEffect[i];
        if (!effect) {
            continue;
        }
        f32 y = effect->getY(key, velo);
        switch (effect->mTarget) {
        case 0:
            param->field_0x18 *= y;
            break;
        case 1:
            param->field_0x1c *= y;
            break;
        case 2:
            param->field_0x2c += y - 0.5;
            break;
        case 3:
            param->field_0x30 += y;
            break;
        case 4:
            param->field_0x34 += y;
            break;
        default:
            JUT_ASSERT(93, FALSE);
        }
    }
    const TKeymap* keymap = NULL;
    for (int i = 0; i < (int)mKeyRegionCount; i++) {
        if (key <= mKeymap[i].mBaseKey) {
            keymap = &mKeymap[i];
            param->field_0x3c = i;
            break;
        }
    }
    if (!keymap) {
        return false;
    }
    for (int i = 0; i < (int)keymap->mVeloRegionCount; i++) {
        const TVeloRegion* region = keymap->getVeloRegion(i);
        if (velo <= region->mBaseVel) {
            param->field_0x10 *= region->field_0x08;
            param->field_0x14 *= region->field_0x0c;
            param->field_0x4 = region->field_0x04;
            return true;
        }
    }
    return false;
}

/* 80284B4C-80284B54       .text getKeymapIndex__Q28JASystem10TBasicInstCFi */
int TBasicInst::getKeymapIndex(int index) const {
    return index;
}

/* 80284B54-80284C10       .text setKeyRegionCount__Q28JASystem10TBasicInstFUl */
void TBasicInst::setKeyRegionCount(u32 num) {
    delete[] mKeymap;
    mKeymap = new TKeymap[num];  // ===== §368: donor `new (TBank::getCurrentHeap(), 0)`
    JUT_ASSERT(140, mKeymap != NULL);
    mKeyRegionCount = num;
}

/* 80284C10-80284CC4       .text setEffectCount__Q28JASystem10TBasicInstFUl */
void TBasicInst::setEffectCount(u32 num) {
    delete[] mEffect;
    mEffectCount = num;
    if (num == 0) {
        mEffect = NULL;
        return;
    }
    mEffect = new TInstEffect*[num];  // ===== §368: donor `new (TBank::getCurrentHeap(), 0)`
    JUT_ASSERT(157, mEffect != NULL);
    Calc::bzero(mEffect, num * sizeof(mEffect[0]));  // ===== §372: same GC-pointer-size class
}

/* 80284CC4-80284D7C       .text setEffect__Q28JASystem10TBasicInstFiPQ28JASystem11TInstEffect */
void TBasicInst::setEffect(int index, TInstEffect* effect) {
    JUT_ASSERT(164, index < (int)mEffectCount);
    JUT_ASSERT(165, index >= 0);
    mEffect[index] = effect;
}

/* 80284D7C-80284E30       .text setOscCount__Q28JASystem10TBasicInstFUl */
void TBasicInst::setOscCount(u32 num) {
    delete[] mOsc;
    mOscCount = num;
    if (num == 0) {
        mOsc = NULL;
        return;
    }
    mOsc = new TOscillator::Osc_*[num];  // ===== §368: donor `new (TBank::getCurrentHeap(), 0)`
    JUT_ASSERT(193, mOsc != NULL);
    Calc::bzero(mOsc, num * sizeof(mOsc[0]));  // ===== §372: same GC-pointer-size class
}

/* 80284E30-80284EE8       .text setOsc__Q28JASystem10TBasicInstFiPQ38JASystem11TOscillator4Osc_ */
void TBasicInst::setOsc(int index, TOscillator::Osc_* osc) {
    JUT_ASSERT(199, index < (int)mOscCount);
    JUT_ASSERT(200, index >= 0);
    mOsc[index] = osc;
}

/* 80284EE8-80284F70       .text getKeyRegion__Q28JASystem10TBasicInstFi */
TBasicInst::TKeymap* TBasicInst::getKeyRegion(int index) {
    JUT_ASSERT(217, index >= 0);
    if (index >= (int)mKeyRegionCount) {
        return NULL;
    }
    return &mKeymap[index];
}

/* 80284F70-80284FC4       .text __dt__Q38JASystem10TBasicInst7TKeymapFv */
TBasicInst::TKeymap::~TKeymap() {
    delete[] mVelomap;
}

/* 80284FC4-80285058       .text setVeloRegionCount__Q38JASystem10TBasicInst7TKeymapFUl */
void TBasicInst::TKeymap::setVeloRegionCount(u32 num) {
    delete[] mVelomap;
    mVelomap = new TVeloRegion[num];  // ===== §368: donor `new (TBank::getCurrentHeap(), 0)`
    JUT_ASSERT(244, mVelomap != NULL);
    mVeloRegionCount = num;
}

/* 80285058-802850E0       .text getVeloRegion__Q38JASystem10TBasicInst7TKeymapFi */
TBasicInst::TVeloRegion* TBasicInst::TKeymap::getVeloRegion(int index) {
    JUT_ASSERT(252, index >= 0);
    if (index >= (int)mVeloRegionCount) {
        return NULL;
    }
    return &mVelomap[index];
}

/* 802850E0-80285168       .text getVeloRegion__Q38JASystem10TBasicInst7TKeymapCFi */
const TBasicInst::TVeloRegion* TBasicInst::TKeymap::getVeloRegion(int index) const {
    JUT_ASSERT(261, index >= 0);
    if (index >= (int)mVeloRegionCount) {
        return NULL;
    }
    return &mVelomap[index];
}

// ============================================================
// §368 donor TU [5]: JASInstEffect.cpp
// ============================================================

/* 8028684C-802868F0       .text setTarget__Q28JASystem11TInstEffectFi */
void TInstEffect::setTarget(int target) {
    JUT_ASSERT(16, target >= 0);
    JUT_ASSERT(17, target < 256);
    mTarget = target;
}

// ============================================================
// §368 donor TU [6]: JASInstRand.cpp
// ============================================================

/* 80286B58-80286BF4       .text getY__Q28JASystem9TInstRandCFii */
f32 TInstRand::getY(int, int) const {
    static JMath::TRandom_fast_ oRandom(0);
    f32 tmp = (oRandom.get_ufloat_1() * 2.0f - 0.9999999f);
    tmp *= field_0xc;
    return tmp + field_0x8;
}

// ============================================================
// §368 donor TU [7]: JASInstSense.cpp
// ============================================================

/* 802868F0-80286A1C       .text getY__Q28JASystem10TInstSenseCFii */
f32 TInstSense::getY(int param_1, int param_2) const {
    int r6 = 0;
    switch (field_0x8) {
    case 1:
        r6 = param_2;
        break;
    case 2:
        r6 = param_1;
        break;
    }
    f32 ret;
    if (field_0x9 == 0x7f || field_0x9 == 0) {
        ret = field_0xc + r6 * (field_0x10 - field_0xc) / 127.0f;
    } else if (r6 < field_0x9) {
        ret = field_0xc + (1.0f - field_0xc) * (r6 / (f32)field_0x9);
    } else {
        ret = (field_0x10 - 1.0f) * ((r6 - field_0x9) / (f32)(0x7f - field_0x9)) + 1.0f;
    }
    return ret;
}

/* 80286A1C-80286B58       .text setParams__Q28JASystem10TInstSenseFiiff */
void TInstSense::setParams(int trigger, int centerkey, f32 param_3, f32 param_4) {
    JUT_ASSERT(43, trigger >= 0);
    JUT_ASSERT(44, trigger < 256);
    field_0x8 = trigger;
    JUT_ASSERT(47, centerkey >= 0);
    JUT_ASSERT(48, centerkey < 256);
    field_0x9 = centerkey;
    field_0xc = param_3;
    field_0x10 = param_4;
}

// ============================================================
// §368 donor TU [8]: JASDrumSet.cpp
// ============================================================

/* 802851D4-80285470       .text getParam__Q28JASystem8TDrumSetCFiiPQ28JASystem10TInstParam */
bool TDrumSet::getParam(int key, int param_2, TInstParam* param_3) const {
    JUT_ASSERT(24, key >= 0);
    if (key >= (int)sPercCount) {
        OSReport("JASDrumSet: key %d >= sPercCount %d\n", key, sPercCount);
        return false;
    }
    const TPerc* perc = field_0x4 + key;
    param_3->field_0x0 = 0;
    param_3->field_0x38 = 1;
    param_3->field_0x10 *= perc->field_0x0;
    param_3->field_0x14 *= perc->field_0x4;
    param_3->field_0x20 = perc->field_0x8;
    param_3->field_0x3a = perc->field_0xc;
    static TOscillator::Osc_ osc;
    osc.field_0x0 = 0;
    osc.field_0x4 = 1.0f;
    osc.table = NULL;
    osc.rel_table = NULL;
    osc.field_0x10 = 1.0f;
    osc.field_0x14 = 0.0f;
    static TOscillator::Osc_* oscp = &osc;
    param_3->mOscData = &oscp;
    param_3->mOscCount = 1;
    for (int i = 0; i < (int)perc->mEffectCount; i++) {
        TInstEffect* effect = perc->mEffect[i];
        if (effect) {
            f32 y = effect->getY(key, param_2);
            switch (effect->mTarget) {
            case 0:
                param_3->field_0x18 *= y;
                break;
            case 1:
                param_3->field_0x1c *= y;
                break;
            case 2:
                param_3->field_0x2c += y - 0.5;
                break;
            case 3:
                param_3->field_0x30 += y;
                break;
            case 4:
                param_3->field_0x34 += y;
                break;
            default:
                JUT_ASSERT(77, FALSE);
                break;
            }
        }
    }
    for (int i = 0; i < (int)perc->mVelomapCount; i++) {
        TBasicInst::TVeloRegion* region = perc->mVelomap + i;
        if (param_2 > region->mBaseVel) {
            continue;
        }
        param_3->field_0x10 *= region->field_0x08;
        param_3->field_0x14 *= region->field_0x0c;
        param_3->field_0x4 = region->field_0x04;
        return true;
    }
    return false;
}

/* 80285470-80285520       .text getPerc__Q28JASystem8TDrumSetFi */
TDrumSet::TPerc* TDrumSet::getPerc(int index) {
    JUT_ASSERT(101, index < (int)sPercCount);
    JUT_ASSERT(102, index >= 0);
    return field_0x4 + index;
}

/* 80285520-80285554       .text __ct__Q38JASystem8TDrumSet5TPercFv */
TDrumSet::TPerc::TPerc() {
    field_0x0 = 1.0f;
    field_0x4 = 1.0f;
    field_0x8 = 0.5f;
    field_0xc = 1000;
    mEffect = NULL;
    mEffectCount = 0;
    mVelomapCount = 0;
    mVelomap = NULL;
}

/* 80285554-802855B0       .text __dt__Q38JASystem8TDrumSet5TPercFv */
TDrumSet::TPerc::~TPerc() {
    delete[] mEffect;
    delete[] mVelomap;
}

/* 802855B0-80285664       .text setEffectCount__Q38JASystem8TDrumSet5TPercFUl */
void TDrumSet::TPerc::setEffectCount(u32 param_1) {
    delete[] mEffect;
    mEffectCount = param_1;
    if (param_1 == 0) {
        mEffect = NULL;
        return;
    }
    mEffect = new TInstEffect*[param_1];  // ===== §368: donor `new (TBank::getCurrentHeap(), 0)`
    JUT_ASSERT(146, mEffect != NULL);
    Calc::bzero(mEffect, param_1 * sizeof(mEffect[0]));  // ===== §372: same GC-pointer-size class
}

/* 80285664-802856F8       .text setVeloRegionCount__Q38JASystem8TDrumSet5TPercFUl */
void TDrumSet::TPerc::setVeloRegionCount(u32 param_1) {
    delete[] mVelomap;
    mVelomap = new TBasicInst::TVeloRegion[param_1];  // ===== §368: donor `new (TBank::getCurrentHeap(), 0)`
    JUT_ASSERT(155, mVelomap != NULL);
    mVelomapCount = param_1;
}

/* 802856F8-802857AC       .text getVeloRegion__Q38JASystem8TDrumSet5TPercFi */
TBasicInst::TVeloRegion* TDrumSet::TPerc::getVeloRegion(int index) {
    JUT_ASSERT(161, index < (int)mVelomapCount);
    JUT_ASSERT(162, index >= 0);
    return mVelomap + index;
}

/* 802857AC-80285864       .text setEffect__Q38JASystem8TDrumSet5TPercFiPQ28JASystem11TInstEffect */
void TDrumSet::TPerc::setEffect(int index, TInstEffect* param_2) {
    JUT_ASSERT(177, index < (int)mEffectCount);
    JUT_ASSERT(178, index >= 0);
    mEffect[index] = param_2;
}

/* 80285864-802858D8       .text setRelease__Q38JASystem8TDrumSet5TPercFUl */
void TDrumSet::TPerc::setRelease(u32 release) {
    JUT_ASSERT(195, release < 0x10000);
    field_0xc = release;
}

/* 802858D8-8028595C       .text __dt__Q28JASystem8TDrumSetFv */
TDrumSet::~TDrumSet() {}

}  // namespace JAudio1
