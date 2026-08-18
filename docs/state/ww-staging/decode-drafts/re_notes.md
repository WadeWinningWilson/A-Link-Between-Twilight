# RE notes -- recovered facts from the WW decode drafts (DECODER lane)

> tp-style committed dump (DECODER-BRIEFING S3.2): names and layout facts
> recovered during draft work live HERE for review and reuse; the upstream
> scaffold uses upstream conventions (`field_0xNNN` for unknowns) and these
> notes justify any name that gets promoted into it. Every row carries its
> receipt. Naming is documentation, not the schedule's metric.
>
> Sibling-rule naming maps: `naming-map-auto-*.md` (per-TU, with tiers and
> the So misfire correction). Class-size and static facts below are from the
> debug maps / dtk splits where cited.

## Cross-TU facts

- `daMsw_Execute` (d_a_msw): pure register PERMUTATION on retail (identical
  opcodes/order, ~60 ARG_MISMATCH rows). EQUIVALENCE IS PROVEN, NOT ARGUED:
  `configure.py:1466` = `ActorRel(MatchingFor("D44J01"), ...)` -- the same
  source byte-matches upstream's demo build; only the retail allocator
  differs. PERMANENT CEILING (History/Bridge review s3e5f21a7): our donor is
  GZLE01 and D44J01 is the only matching version, so this function can NEVER
  go byte-true for us -- Equivalent is TERMINAL, stop-work permanent.
- THE SYMBOL-COUNTER METRIC, calibrated (History/Bridge): static-local
  counters (`xd$NNNN`) are IDENTICAL across GZLE01/GZLJ01/GZLP01 (version-
  invariant across retail); the DEMO build differs from retail by FIVE
  (`xd$4198`). Our d_a_msw compile reads `xd$1763` vs target `xd$4203` --
  a delta of ~2,440 where 5 = "genuinely different build of the same game":
  the reconstruction's include/inline set is CATEGORICALLY different, not
  merely lighter. Usable as an instrument across the queue.
- Two parks, two strengths, filed separately: d_a_msw = PROVEN equivalent
  (upstream's own build demonstrates it); d_a_lod_bg = ASSERTED equivalent
  (rests on the machine-verified dead-register lists, NonMatching on every
  version upstream).
- `daLodbg_c::loadModelData` (d_a_lod_bg): 2-instruction regswap
  (r27 vs r29), machine-verified dead-register tie-break -- target reuses
  earliest-dead (r29, the dead `mModelData&` param reg), MWCC-ours reuses
  latest-dead (r27, after `oldHeap`). Both prior values provably dead at the
  swap (occurrence lists in CALLS row + tracker s7c1a0bd3). Guarded
  `#if VERSION > VERSION_DEMO` (no demo variant exists -- History/Bridge).

- `daNpc_Ko1_c` instance spans >= 0x8AA (draft struct extent); ko1 TU carries
  Ko1 + Ko2 (one TU, two actors), 139 functions, `.text` 0xAB08 bytes
  (debug map d_a_npc_ko1D.map).
- `dNpc_PathRun_c` is 0x08 bytes: mPath@0, field_0x04, mIdx@5, mbDir@6,
  field_0x07 (donor d_npc.h) -- flat-struct drafts that declare fields inside
  0x73C..0x743 are PathRun interior.
- `J3DAnmBase`: vt@0, mAttribute@4, mFrameMax(s16)@6, mFrame(f32)@8, mKind@C
  (donor J3DAnimation.h) -- any `->unk6` frame-max read through an anim
  pointer resolves here.
- Retail assert strings and HIO/staff tables leak original member names
  (`m_hed_tex_pttrn`, `m_hed_jnt_num` family, `ActNo`); debug-only HIO
  genMessage label strings do NOT exist in retail RELs (ct/dt only).
- The anime-tag channel is LIVE in ko1 (chngAnmTag 56 B, 14 instr) and
  retail-empty (4-byte blr) in Ym1/Yw1/Aj1 (History/Bridge size-verified).

## d_a_npc_aj1

| offset | member | type | tier | receipt |
|---|---|---|---|---|
| 0x744 | `mEventIdTable` | `s16` | NAMED-BY-RULE | event id table (template_name.py SS258 RECEIPT tier; naming-map-auto-aj1.md) |
| 0x746 | `mEventIdx` | `s16` | NAMED-BY-RULE | event index (template_name.py SS258 RECEIPT tier; naming-map-auto-aj1.md) |
| 0x7BA | `mOrderType` | `u8` | NAMED-BY-RULE | eventOrder selector (template_name.py SS258 RECEIPT tier; naming-map-auto-aj1.md) |
| 0x7BB | `mSttNum` | `u8` | NAMED-BY-RULE | setStt arg store (template_name.py SS258 RECEIPT tier; naming-map-auto-aj1.md) |
| 0x7BC | `mSttNumOld` | `u8` | NAMED-BY-RULE | stt old (template_name.py SS258 RECEIPT tier; naming-map-auto-aj1.md) |

## d_a_npc_ko1

| offset | member | type | tier | receipt |
|---|---|---|---|---|
| 0x6A4 | `unk6A4` | `s32` | RESOLVED | alias of daNpc_Ko1_c @same offset -- m2c merged actor-typed receivers into this JUTAssertion chimera; the actor struct's own row is where the name lands |
| 0x6B4 | `unk6B4` | `f32` | RESOLVED | alias of daNpc_Ko1_c @same offset -- m2c merged actor-typed receivers into this JUTAssertion chimera; the actor struct's own row is where the name lands |
| 0x6B8 | `unk6B8` | `u16` | RESOLVED | alias of daNpc_Ko1_c @same offset -- m2c merged actor-typed receivers into this JUTAssertion chimera; the actor struct's own row is where the name lands |
| 0x6BC | `m_manzai_stt` | `u8` | NAMED | manzai sync state -- chk_manzai_1 sets a partner to 1=requested (draft:1612), registers the group and sets own to 2=ready when all partners read 2 (draft:1602,1673); anmAtr treats ==2 && not-current-speaker as manzai-... |
| 0x330 | `mpMorf` | `mDoExt_McaMorf *` | NAMED-CONVENTION | body mDoExt_McaMorf*; anmNum_toResID feeds it (setAnm_anm draft:1340); setMorf(0) in createInit (draft:1165); donor-wide mpMorf idiom |
| 0x538 | `mStts` | `dCcD_Stts` | NAMED-CONVENTION | dCcD_Stts::Init(&this->0x538,..) (draft:1162); donor-wide mStts idiom |
| 0x574 | `mCyl` | `dCcD_Cyl` | NAMED-CONVENTION | dCcD_Cyl::Set(&this->0x574, &dNpc_cyl_src) (draft:1164); donor-wide mCyl idiom |
| 0x6BC | `m_manzai_stt` | `u8` | NAMED | manzai sync state -- chk_manzai_1 sets a partner to 1=requested (draft:1612), registers the group and sets own to 2=ready when all partners read 2 (draft:1602,1673); anmAtr treats ==2 && not-current-speaker as manzai-... |
| 0x6CC | `m_hed_jnt_num` | `u8` | NAMED | retail string a_cut_tbl$5860 'head/m_hed_jnt_num >= 0'; 1st in joint-search seq (draft:4647); feeds head-morf mtx (draft:1211); km1 m_head_jnt_num @0x6CC |
| 0x6CD | `m_bbone_jnt_num` | `u8` | NAMED | retail string 'backbone/m_bbone_jnt_num >= 0'; 2nd in seq (draft:4655); km1 m_backbone_jnt_num @0x6CD |
| 0x6CE | `m_armR2_jnt_num` | `u8` | NAMED | retail string 'armR2/m_armR2_jnt_num >= 0'; 3rd in seq (draft:4663); armR2 mtx copy (draft:1219) |
| 0x6CF | `m_hed_2_jnt_num` | `u8` | NAMED | retail string 'head2/m_hed_2_jnt_num >= 0'; 4th in seq; nodeCallBack_Hed compare (draft:837) |
| 0x6D0 | `m_bln_loc_jnt_num` | `u8` | NAMED | retail string 'balloon_loc/m_bln_loc_jnt_num >= 0'; 5th in seq; nodeCallBack_Bln compare (draft:863) |
| 0x6D1 | `m_bln_jnt_num` | `u8` | NAMED | retail string 'ko_balloon/m_bln_jnt_num >= 0'; 6th in seq; feeds balloon-morf mtx (draft:2743) |
| 0x704 | `mpBalloonMorf` | `mDoExt_McaMorf *` | NAMED-CONVENTION | balloon mDoExt_McaMorf*; balloon_anmNum_toResID feeds it (draft:1357); balloon-jnt mtx source (draft:2743) |
| 0x710 | `mpHedMorf` | `mDoExt_McaMorf *` | NAMED-CONVENTION | head mDoExt_McaMorf*; headAnmNum_toResID feeds it (draft:1341); head-jnt mtx target (draft:1211); split-morf idiom (bgn2/bmd mpHeadMorf); 'hed' spelling from this TU's retail strings |
| 0x714 | `m_hed_tex_pttrn` | `J3DAnmTexPattern *` | NAMED | retail assert 'm_hed_tex_pttrn != 0' fires at this member's null-check in setBtp (draft:1289); type CORRECTED from m2c's JUTAssertion misbind per History/Bridge review 2026-08-17 -- J3DAnmTexPattern* (fed to mDoExt_bt... |
| 0x718 | `mBtpAnm` | `mDoExt_btpAnm` | NAMED-ANALOGY | km1 mDoExt_btpAnm mBtpAnm = tex_pttrn+4 (0x6D8->0x6DC); ko1 0x714->0x718 same stride; in-TU mDoExt_btpAnm::init(&this->0x718) (draft:1296) |
| 0x72C | `mBtpFrame` | `u8` | NAMED-ANALOGY | km1 mBtpFrame = mBtpAnm+0x14 (0x6F0); ko1 0x718+0x14=0x72C; role: frame counter vs J3DAnmTexPattern frame max (plyTexPttrnAnm draft:1313) |
| 0x73C | `mPathRun` | `dNpc_PathRun_c` | NAMED | dNpc_PathRun_c; dNpc_PathRun_c::setInf(&this->0x73C,..) in createInit (draft:1107); .unk0 is mPath per d_npc.h @0x00 |
| 0x741 | `unk741` | `u8` | RESOLVED | dNpc_PathRun_c::mIdx @+0x05 of mPathRun (receipt d_npc.h, size 0x08) |
| 0x742 | `unk742` | `u8` | RESOLVED | dNpc_PathRun_c::mbDir @+0x06 of mPathRun (receipt d_npc.h) |
| 0x748 | `mEventCut` | `dNpc_EventCut_c` | NAMED-ANALOGY | dNpc_EventCut_c; setActorInfo2 (draft:1116); km1 mEventCut, same type |
| 0x7B4 | `m_partner_id0` | `u32` | NAMED | fpc_ProcID array base @0x7B4, stride 4, walked by chk_manzai_1 (draft:1595) and searchByID'd (draft:1619); entries registered into the manzai group block g_dComIfG+0x5C28 |
| 0x7B8 | `m_partner_id1` | `u32` | NAMED | second entry of the 0x7B4 partner-ID array; searchByID'd for the 3-way manzai cases (draft:1635,1660) |
| 0x7BC | `m_partner_num` | `u8` | NAMED | loop bound over the 0x7B4 partner-ID array in chk_manzai_1 (draft:1593-1594) |
| 0x7F4 | `m_tgt_pos` | `Vec` | NAMED | the setter is literally set_tgtPos (draft:2148); target of cLib_targetAngleY from current pos (draft:2239,2272); displacement-to-it measured for arrival in ko_movPass (draft:2201,2234,2267). cXyz spanning 0x7F4-0x7FF |
| 0x7F8 | `unk7F8` | `f32` | RESOLVED | m_tgt_pos.y (cXyz interior) |
| 0x7FC | `unk7FC` | `f32` | RESOLVED | m_tgt_pos.z (cXyz interior) |
| 0x82C | `m_mov_spd` | `f32` | NAMED | chase TARGET for fopAc speedF in ko_clcMovSpd -- cLib_chaseF(&speedF@0x254, this->0x82C, this->0x834) (draft:2254) |
| 0x830 | `m_swm_spd_y` | `f32` | NAMED | chase target for speed.y (fopAc 0x224) while swimming -- cLib_chaseF(&speed.y, this->0x830, HIO step) in ko_clcSwmSpd (draft:2275) |
| 0x834 | `m_mov_spd_step` | `f32` | NAMED | chase STEP of the same cLib_chaseF call (draft:2254) |
| 0x89C | `mAnmAtr` | `u8` | NAMED | chg_anmAtr compare-then-store (draft:1451-1454), 0xFF none-sentinel (draft:1501), indexes a_anm_prm_tbl$4763 in setAnm_ATR (draft:1482-1484) |
| 0x89D | `mAnmTag` | `u8` | NAMED | chg_anmTag/control_anmTag gate on it (draft:1406,1414; 0xFF reset draft:1418), set from the message anime-tag channel (draft:1507-1508). NOTE: ko1's tag channel is LIVE, unlike Ym1/Yw1/Aj1 whose chngAnmTag is retail-e... |
| 0x8A2 | `mOrderType` | `u8` | NAMED | eventOrder selector (draft:1524; cases 1/2 -> fopAcM_orderSpeakEvent), cleared by checkOrder on event ack (draft:1543) -- ba1 SS257 template rule hand-applied, same shape as So 0xB70 |
| 0x8A7 | `m_act_no` | `u8` | NAMED | retail token 'ActNo' in a_staff_tbl$4419; indexes the 9-entry staff tbl (draft:1116) and the 9-way init_HNA_0..4/init_BOU_0..3 switch (draft:1120) |

## d_a_npc_ob1

| offset | member | type | tier | receipt |
|---|---|---|---|---|
| 0x330 | `mpBtpRes` | `mDoExt_McaMorf *` | NAMED-BY-RULE | btp resource (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md) |
| 0x6F0 | `mBtpFrame` | `u8` | NAMED-BY-RULE | btp frame (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md) |
| 0x6F2 | `mBlinkTimer` | `s16` | NAMED-BY-RULE | blink timer (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md) |
| 0x7D0 | `mEventIdTable` | `s16` | NAMED-BY-RULE | event id table (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md) |
| 0x7D2 | `mEventIdx` | `s16` | NAMED-BY-RULE | event index (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md) |
| 0x802 | `mBtpNum` | `u8` | NAMED-BY-RULE | setAnm_tex store (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md) |
| 0x804 | `mOrderType` | `u8` | NAMED-BY-RULE | eventOrder selector (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md) |
| 0x805 | `mSttNum` | `u8` | NAMED-BY-RULE | setStt arg store (template_name.py SS258 RECEIPT tier; naming-map-auto-ob1.md) |

## d_a_npc_so

| offset | member | type | tier | receipt |
|---|---|---|---|---|
| 0x6CC | `mProcNo` | `s32` | NAMED | modeProc(Proc_e,i) case 0 stores the requested proc (draft:2140), case 1 dispatches the ptmf table by it *0x1C (draft:2144) |
| 0x6D2 | `mBckIdx` | `s8` | NAMED | passed as pBckIdx (s8*) to dLib_bcks_setAnm (draft:1348; signature receipt d_lib.h) |
| 0x6D3 | `mPrmIdx` | `u8` | NAMED | setAnm arg store with 6=keep sentinel (draft:1336) AND passed as pPrmIdx to dLib_bcks_setAnm (draft:1348; d_lib.h) |
| 0x84C | `mpMorf` | `mDoExt_McaMorf *` | NAMED | the morf arg of dLib_bcks_setAnm (draft:1348; d_lib.h) -- So's body mDoExt_McaMorf*; donor-wide mpMorf idiom |
| 0x870 | `mAcch` | `dBgS_ObjAcch` | NAMED | dBgS_ObjAcch& arg of dLib_getWaterY (draft:1461); donor-wide dBgS_ObjAcch member name mAcch (53 of 63 donor actor headers) |
| 0xB70 | `mOrderType` | `u8` | NAMED | eventOrder selector (draft:2153), cleared by checkOrder on event ack (draft:2192,2204) -- the ba1 SS257 template rule hand-applied; the auto regex missed it because So loads via temp_r5 |

## d_a_npc_ym1

| offset | member | type | tier | receipt |
|---|---|---|---|---|
| 0x8AC | `mOrderType` | `u8` | NAMED-BY-RULE | eventOrder selector (template_name.py SS258 RECEIPT tier; naming-map-auto-ym1.md) |
| 0x8AD | `mSttNum` | `u8` | NAMED-BY-RULE | setStt arg store (template_name.py SS258 RECEIPT tier; naming-map-auto-ym1.md) |
| 0x8AE | `mSttNumOld` | `u8` | NAMED-BY-RULE | stt old (template_name.py SS258 RECEIPT tier; naming-map-auto-ym1.md) |

## d_a_npc_yw1

| offset | member | type | tier | receipt |
|---|---|---|---|---|
| 0x7C1 | `mOrderType` | `u8` | NAMED-BY-RULE | eventOrder selector (template_name.py SS258 RECEIPT tier; naming-map-auto-yw1.md) |
| 0x7C2 | `mSttNum` | `u8` | NAMED-BY-RULE | setStt arg store (template_name.py SS258 RECEIPT tier; naming-map-auto-yw1.md) |
