// KIT-LINEAGE: native-port
// KIT-DONOR: JSystem/JAudio/JASBNKParser.cpp
// KIT-DONOR-REF: zeldaret/tww@1d57f0468986987ec26a3d1800bdc1aaad3794db
// KIT-DONOR-STATUS: NonMatching
// ============================================================
// §368 JAudio1 IBNK parser — donor-verbatim port of WW retail
// JSystem/JAudio/JASBNKParser.cpp (D:/XXXXXXX/WW DP; dtk address
// markers kept). Phase A3 of the native JA1 campaign (bus
// §362/§363/§366). Donor JASystem::* flattens into JAudio1::*.
// Nothing in the game calls this TU yet; the ExtSeq bridge keeps
// playing until A4 wires bank registration.
// Donor include map: JASCalc.h/JUTAssert.h -> ja1_boundary.h;
// JSupport.h -> receiver JSystem/JSupport/JSupport.h (same
// JSUConvertOffsetToPtr template lineage).
// §368 heap adaptation (A2 precedent): every donor `new (heap, 0)` is
// a plain new — no JA1 arena exists on the PC side until A4. The donor
// tracked arena usage in sUsedHeapSize via heap->getFreeSize() before/
// after the build; with no arena that bookkeeping is inert (kept as a
// symbol, accumulates 0) — see the marked lines.
// ============================================================

#include "d/ext_seq/ja1_jasbank.h"

#include "JSystem/JSupport/JSupport.h"  // JSUConvertOffsetToPtr (donor JSystem/JSupport/JSupport.h)
#include "global.h"
#include "dolphin/os.h"

namespace JAudio1 {

u32 BNKParser::sUsedHeapSize;

/* 802870F0-802879A0       .text createBasicBank__Q28JASystem9BNKParserFPv */
TBasicBank* BNKParser::createBasicBank(void* stream) {
    /* Nonmatching - regswap */
    // ===== §368: donor opens with `JKRHeap* heap = TBank::getCurrentHeap();
    // const u32 freeSize = heap->getFreeSize();` — arena bookkeeping, inert
    // on the PC side until A4 (see file header).
    THeader* header = (THeader*)stream;
    TBasicBank* bank = new TBasicBank();  // ===== §368: donor `new (heap, 0)`
    if (bank == NULL) {
        return NULL;
    }
    bank->setInstCount(0x100);

    for (int i = 0; i < 0x80; i++) {
        TInst* instRaw = JSUConvertOffsetToPtr<TInst>(header, header->mInstOffsets[i]);
        if (instRaw != NULL) {
            TBasicInst* instp = new TBasicInst();  // ===== §368: donor `new (heap, 0)`
            JUT_ASSERT(56, instp != NULL);
            instp->field_0x4 = instRaw->field_0x8;
            instp->field_0x8 = instRaw->field_0xC;

            instp->setOscCount(2);
            for (int oscIndex = 0, j = 0; j < 2; j++) {
                TOsc* oscRaw = JSUConvertOffsetToPtr<TOsc>(header, instRaw->mOscOffsets[j]);
                if (oscRaw != NULL) {
                    TOscillator::Osc_* osc = findOscPtr(bank, header, oscRaw);
                    if (osc == NULL) {
                        osc = new TOscillator::Osc_();  // ===== §368: donor `new (heap, 0)`
                        JUT_ASSERT(72, osc != NULL);
                        osc->field_0x0 = oscRaw->field_0x0;
                        osc->field_0x4 = oscRaw->field_0x4;
                        s16* oscTable = JSUConvertOffsetToPtr<s16>(header, oscRaw->field_0x8);
                        if (oscTable != NULL) {
                            s32 tableLength = getOscTableEndPtr(oscTable) - oscTable;
                            osc->table = new s16[tableLength];  // ===== §368: donor `new (heap, 0)`
                            JUT_ASSERT(82, osc->table != NULL);
                            Calc::bcopy(oscTable, osc->table, tableLength * sizeof(s16));
                        } else {
                            osc->table = NULL;
                        }
                        oscTable = JSUConvertOffsetToPtr<s16>(header, oscRaw->field_0xC);
                        if (oscTable != NULL) {
                            s32 tableLength = getOscTableEndPtr(oscTable) - oscTable;
                            osc->rel_table = new s16[tableLength];  // ===== §368: donor `new (heap, 0)`
                            JUT_ASSERT(94, osc->rel_table != NULL);
                            Calc::bcopy(oscTable, osc->rel_table, tableLength * sizeof(s16));
                        } else {
                            osc->rel_table = NULL;
                        }
                        osc->field_0x10 = oscRaw->field_0x10;
                        osc->field_0x14 = oscRaw->field_0x14;
                    }
                    instp->setOsc(oscIndex, osc);
                    oscIndex++;
                }
            }

            instp->setEffectCount(4);
            for (int j = 0; j < 2; j++) {
                TRand* randRaw = JSUConvertOffsetToPtr<TRand>(header, instRaw->mRandOffsets[j]);
                if (randRaw != NULL) {
                    TInstRand* randp = new TInstRand();  // ===== §368: donor `new (heap, 0)`
                    JUT_ASSERT(120, randp != NULL);
                    randp->setTarget(randRaw->field_0x0);
                    randp->field_0x8 = randRaw->field_0x4;
                    randp->field_0xc = randRaw->field_0x8;
                    instp->setEffect(j, randp);
                }
            }
            for (int j = 0; j < 2; j++) {
                TSense* senseRaw = JSUConvertOffsetToPtr<TSense>(header, instRaw->mSenseOffsets[j]);
                if (senseRaw != NULL) {
                    TInstSense* sensep = new TInstSense();  // ===== §368: donor `new (heap, 0)`
                    JUT_ASSERT(133, sensep != NULL);
                    sensep->setTarget(senseRaw->field_0x0);
                    sensep->setParams(senseRaw->field_0x1, senseRaw->field_0x2, senseRaw->field_0x4, senseRaw->field_0x8);
                    instp->setEffect(j + 2, sensep);
                }
            }

            instp->setKeyRegionCount(instRaw->mKeyRegionCount);
            for (int j = 0; j < (int)instRaw->mKeyRegionCount; j++) {
                TBasicInst::TKeymap* instKeymap = instp->getKeyRegion(j);
                TKeymap* keymapRaw = JSUConvertOffsetToPtr<TKeymap>(header, instRaw->mKeymapOffsets[j]);
                instKeymap->mBaseKey = keymapRaw->field_0x0;
                instKeymap->setVeloRegionCount(keymapRaw->field_0x4);
                for (int k = 0; k < (int)keymapRaw->field_0x4; k++) {
                    TBasicInst::TVeloRegion* instVeloRegion = instKeymap->getVeloRegion(k);
                    TVmap* vmapRaw = JSUConvertOffsetToPtr<TVmap>(header, keymapRaw->mVmapOffsets[k]);
                    instVeloRegion->mBaseVel = vmapRaw->field_0x0;
                    instVeloRegion->field_0x04 = vmapRaw->field_0x4 & 0xFFFF;
                    instVeloRegion->field_0x08 = vmapRaw->field_0x8;
                    instVeloRegion->field_0x0c = vmapRaw->field_0xC;
                }
            }
            bank->setInst(i, instp);
        }
    }

    for (int i = 0; i < 12; i++) {
        TPerc* percRaw = JSUConvertOffsetToPtr<TPerc>(header, header->mPercOffsets[i]);
        if (percRaw != NULL) {
            TDrumSet* setp = new TDrumSet();  // ===== §368: donor `new (heap, 0)`
            JUT_ASSERT(183, setp != NULL);
            for (int j = 0; j < 0x80; j++) {
                TPmap* pmapRaw = JSUConvertOffsetToPtr<TPmap>(header, percRaw->mPmapOffsets[j]);
                if (pmapRaw != NULL) {
                    TDrumSet::TPerc* drumSetPerc = setp->getPerc(j);
                    drumSetPerc->field_0x0 = pmapRaw->field_0x0;
                    drumSetPerc->field_0x4 = pmapRaw->field_0x4;
                    if (percRaw->mMagic == 'PER2') {
                        drumSetPerc->field_0x8 = percRaw->field_0x288[j] / 127.0f;
                        drumSetPerc->setRelease(percRaw->field_0x308[j]);
                    }
                    drumSetPerc->setEffectCount(2);
                    for (int effectIndex = 0, k = 0; k < 2; k++) {
                        TRand* randRaw = JSUConvertOffsetToPtr<TRand>(header, pmapRaw->mRandOffsets[k]);
                        if (randRaw != NULL) {
                            TInstRand* randp = new TInstRand();  // ===== §368: donor `new (heap, 0)`
                            JUT_ASSERT(207, randp != NULL);
                            randp->setTarget(randRaw->field_0x0);
                            randp->field_0x8 = randRaw->field_0x4;
                            randp->field_0xc = randRaw->field_0x8;
                            drumSetPerc->setEffect(effectIndex, randp);
                            effectIndex++;
                        }
                    }
                    drumSetPerc->setVeloRegionCount(pmapRaw->mVeloRegionCount);
                    for (int k = 0; k < (int)pmapRaw->mVeloRegionCount; k++) {
                        TBasicInst::TVeloRegion* instVeloRegion = drumSetPerc->getVeloRegion(k);
                        TVmap* vmapRaw = JSUConvertOffsetToPtr<TVmap>(header, pmapRaw->mVeloRegionOffsets[k]);
                        instVeloRegion->mBaseVel = vmapRaw->field_0x0;
                        instVeloRegion->field_0x04 = vmapRaw->field_0x4 & 0xFFFF;
                        instVeloRegion->field_0x08 = vmapRaw->field_0x8;
                        instVeloRegion->field_0x0c = vmapRaw->field_0xC;
                    }
                }
            }
            bank->setInst(i + 0xE4, setp);
        }
    }
    // ===== §368: donor `sUsedHeapSize += freeSize - heap->getFreeSize();`
    // — arena bookkeeping, inert until A4 (see file header).
    return bank;
}

/* 802879A0-80287AEC       .text findOscPtr__Q28JASystem9BNKParserFPQ28JASystem10TBasicBankPQ38JASystem9BNKParser7THeaderPQ38JASystem9BNKParser4TOsc */
TOscillator::Osc_* BNKParser::findOscPtr(TBasicBank* bank, THeader* header, TOsc* oscPtr) {
    u32* instOffsets = header->mInstOffsets - 1;
    for (int i = 0; i < 128; i++) {
        TInst* instRaw = JSUConvertOffsetToPtr<TInst>(header, instOffsets[i + 1]);
        if (instRaw != NULL) {
            for (int j = 0; j < 2; j++) {
                TOsc* oscRaw = JSUConvertOffsetToPtr<TOsc>(header, instRaw->mOscOffsets[j]);
                if (oscRaw == oscPtr) {
                    JAudio1::TInst* inst = bank->getInst(i);
                    if (inst != NULL) {
                        TInstParam param;
                        inst->getParam(60, 127, &param);
                        if (j < (int)param.mOscCount) {
                            return param.mOscData[j];
                        }
                    }
                }
            }
        }
    }
    return NULL;
}

/* 80287AEC-80287B00       .text getOscTableEndPtr__Q28JASystem9BNKParserFPs */
s16* BNKParser::getOscTableEndPtr(s16* param_1) {
    s16 v1;
    do {
        v1 = *param_1;
        param_1 += 3;
    } while (v1 <= 0xa);
    return param_1;
}

}  // namespace JAudio1
