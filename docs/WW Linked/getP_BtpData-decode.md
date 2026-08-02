# getP_BtpData / getP_BtkData / getP_BrkData — donor decode (Foundry §251)

> **✅ LANDED + ACCEPTANCE-PASSED (History, §252, 2026-07-30).** All three methods landed on
> `dDemo_actor_c` in `src/d/d_demo.cpp` verbatim; `daNpc_Ls1_c::demo()` and `daNpc_Bm1_c` point at the
> native methods; the reconstruction shims are deleted. **Integration point resolved:** the prm blob is
> big-endian in port memory (as flagged), so `dDemoPrm_readS16/readU32` do BE reads — verified by the
> acceptance gate: **Aryll's face emotes perfectly throughout the cutscene** (user-confirmed). Both of
> Foundry's confirmations held in-game: "rebind only on resID change" IS native, and `0x10000` selects
> `mDemoArcName`. This is the first bridge→true-native promotion under the standing directive.

**What this is:** the finished decode of the three `/* Nonmatching */` donor stubs in
`dDemo_actor_c` (the scripted-face subsystem History reconstructed by RE in the audition
mount, bus §247–§250). Decoded **donor-verbatim from the donor's own binary** — not from
the reconstruction — so this is the true native subsystem, ready to land as the port's
`dDemo_actor_c::getP_BtpData()` et al. and retire the per-actor shims.

**Provenance (covenant receipts):**
- Binary: `D:\XXXXXXX\Ex WW\sys\main.dol` — sha1 `8d28bab68bb5078c38e43f29206f0bd01f7e7a67`
  = **byte-identical** to the decomp's target `build/GZLE01/framework.dol`
  (`config/GZLE01/build.sha1`), so the decomp source's VA comments address it directly.
- Disassembly: `tools/foundry/dol_disasm.py --func getP_BtpData__13dDemo_actor_cFPCc`
  (new Foundry instrument — DOL section walk + capstone PPC/BE + symbols.txt branch
  annotation). Ranges: Btp `80069434-80069550` (71 insns), Brk `80069550-800695E8`
  (38), Btk `800695E8-8006969C` (45).
- Field names: donor `include/d/d_demo.h` (layout 0x04 mFlags / 0x3C mTexAnimation /
  0x44 mTexAnimationFrameMax / 0x4C mPrm{mId,mData} / 0x60,0x64,0x68 mBtp/Btk/BrkId).
- SDA data receipt: the arc-override string at `r13-0x7664` = `0x803F6A7C` =
  `mDemoArcName__20dStage_roomControl_c` (symbols.txt) — **the donor natively encodes
  the demo-archive override History RE'd** (§247 recipe 8).

## Decoded semantics (all three confirmed against every instruction)

Common shape: derive a **resource id** either from the explicit STB texture-anim channel
or from the current **prm-stream entry** (switch on `mPrm.mId`, reading packed fields at
byte offsets); **rebind only on change** (`id == mBtpId` → return NULL — History's §247
"only rebind on resID change" lesson is native donor behavior, not a workaround); resolve
from **the actor's arc by default, the demo archive when bit 0x10000 is set in the id**.

| | prm id 1 | prm id 2 | prm id 4 | prm id 5 | prm id 6 | other |
|---|---|---|---|---|---|---|
| **Btp** id read | s16 @+1 | s16 @+2 | u32 @+1 | u32 @+2 | u32 @+2 | NULL |
| **Btk** id read | — | s16 @+4 | — | u32 @+6 | u32 @+6 | NULL |
| **Brk** id read | — | — | — | — | u32 @+0xA | NULL |

Btp only: if `mFlags & ENABLE_TEX_ANM` (checked FIRST, before the prm path), the id is
`mTexAnimation` (0x3C — the port's `mTexAnm`, already written by `JSGSetTextureAnimation`
per №186). Btp only: on a successful resolve, `mTexAnimationFrameMax = (f32)btp->mFrameMax`
(s16 at res+6, i.e. `J3DAnmBase::mFrameMax`). Btk/Btk have neither.

**No length guard:** unlike `getPrm_Morf` (which guards `field_0x54 < N`), the donor asm
for all three getters reads the prm fields with **no size check**. Donor-verbatim = no
guard. (Noted, not "fixed" — DO-NOT discipline.)

## Port-native code (drop into `src/d/d_demo.cpp`; port names, WW logic to the letter)

```cpp
// ============================================================================
// §251 donor decode — the scripted-face subsystem (dDemo_actor_c BTP/BTK/BRK).
// Donor-verbatim from GZLE01 80069434-8006969C (decode doc:
// docs/WW Linked/getP_BtpData-decode.md). Replaces the per-actor
// getDemoBtp/getDemoBtk shims (§247 recipe 8) with the one native method
// every WW demo actor shares. Flags are matched BY NAME (port ENABLE_TEX_ANM
// = 1<<8; WW's = 1<<7 — values differ, semantics identical).
// ============================================================================
J3DAnmTexPattern* dDemo_actor_c::getP_BtpData(const char* i_name) {
    u32 id;
    if (checkEnable(ENABLE_TEX_ANM)) {
        id = mTexAnm;
    } else {
        if (!checkEnable(ENABLE_UNK_e))
            return NULL;
        const u8* prm = (const u8*)mPrm.mData;
        switch (mPrm.field_0x0) {
        case 1: id = dDemoPrm_readS16(prm + 1); break;
        case 2: id = dDemoPrm_readS16(prm + 2); break;
        case 4: id = dDemoPrm_readU32(prm + 1); break;
        case 5:
        case 6: id = dDemoPrm_readU32(prm + 2); break;
        default:
            return NULL;
        }
    }
    if (id == mBtpId)
        return NULL;                       // donor: rebind only on CHANGE
    mBtpId = id;
    const char* arc = i_name;
    if (id & 0x10000)
        arc = dStage_roomControl_c::getDemoArcName();
    J3DAnmTexPattern* btp =
        (J3DAnmTexPattern*)dComIfG_getObjectIDRes(arc, (u16)id);
    if (btp != NULL)
        mTexAnmFrameMax = btp->getFrameMax();
    return btp;
}

J3DAnmTextureSRTKey* dDemo_actor_c::getP_BtkData(const char* i_name) {
    if (!checkEnable(ENABLE_UNK_e))
        return NULL;
    const u8* prm = (const u8*)mPrm.mData;
    u32 id;
    switch (mPrm.field_0x0) {
    case 2: id = dDemoPrm_readS16(prm + 4); break;
    case 5:
    case 6: id = dDemoPrm_readU32(prm + 6); break;
    default:
        return NULL;
    }
    if (id == mBtkId)
        return NULL;
    mBtkId = id;
    const char* arc = i_name;
    if (id & 0x10000)
        arc = dStage_roomControl_c::getDemoArcName();
    return (J3DAnmTextureSRTKey*)dComIfG_getObjectIDRes(arc, (u16)id);
}

void* dDemo_actor_c::getP_BrkData(const char* i_name) {
    if (!checkEnable(ENABLE_UNK_e))
        return NULL;
    u32 id;
    if (mPrm.field_0x0 == 6)
        id = dDemoPrm_readU32((const u8*)mPrm.mData + 0xA);
    else
        return NULL;
    if (id == mBrkId)
        return NULL;
    mBrkId = id;
    const char* arc = i_name;
    if (id & 0x10000)
        arc = dStage_roomControl_c::getDemoArcName();
    return dComIfG_getObjectIDRes(arc, (u16)id);
}
```

## [PORT-INTEGRATION] — the ONE open point (History's call)

`dDemoPrm_readS16` / `dDemoPrm_readU32`: the prm stream is raw STB file data =
**big-endian, unaligned** on GC. Donor asm does bare `lha`/`lwz` (BE machine). The port
must read these **big-endian regardless of host** — match however the working
reconstruction reads the `getDemoIDData` stream today (History proved the channel; reuse
the same byte order there). If the port's STB loader already swaps the prm block in
place, the helpers collapse to plain reads — **verify against the working Aryll face,
don't assume.** s16 cases sign-extend (donor `lha`) — a negative id compares against
`mBtpId` init `-1` and naturally no-ops, which is why the donor needs no -1 guard.

**Landing:** History owns `d_demo.cpp` (they landed the `setDemoData` fix there) — this
doc + code block is the ferry. On land: delete the per-actor `getDemoBtp`/`getDemoBtk`
NULL/reconstructed shims, point `daNpc_Ls1_c::demo()` (and every future cutscene villager)
at the native methods, and re-run the Aryll face playtest as the acceptance gate.
