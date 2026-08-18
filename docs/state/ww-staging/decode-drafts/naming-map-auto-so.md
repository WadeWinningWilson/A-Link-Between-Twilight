# So auto naming map (template_name.py §258 — the ba1 §257 rules, mechanized)

| offset | name | tier | receipt (rule) |
|---|---|---|---|
| — | `mSttNum` | [RECEIPT] | setStt arg store: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| 0xF8 | `mOrderType` | [RECEIPT] | eventOrder selector |
| — | `mEventIdx` | [RECEIPT] | event index: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mEventIdTable` | [RECEIPT] | event id table: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mBtpNum` | [RECEIPT] | setAnm_tex store: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mBlinkTimer` | [RECEIPT] | blink timer: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mBtpFrame` | [RECEIPT] | btp frame: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mpBtpRes` | [RECEIPT] | btp resource: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mEvtActionNo` | [RECEIPT] | event action no: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mAnmAtr` | [INFERENCE] | anm attribute: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mSttNumOld` | [RECEIPT] | stt old: NO MATCH (shape differs — name by hand, ba1 §257 method) |
| — | `mSttTimer` | [INFERENCE] | stt timer: NO MATCH (shape differs — name by hand, ba1 §257 method) |

12 renames applied → `d_a_npc_so.draft.cpp`. Rules that NO-MATCHed or COLLIDED need the manual ba1-method pass.

CORRECTION (DECODER 2026-08-17): the one match (0xF8 mOrderType) was a MISFIRE and has been REVERTED in the draft (%d sites). 0xF8 is inside the fopAc base (dEvt_info_c eventInfo @0xF4+0x4, receipt f_op_actor.h) - the rule's DOTALL regex crossed a function boundary for daNpc_So_c. So takes ZERO auto names; it needs the manual ba1-method pass.

## Manual ba1-method pass (DECODER 2026-08-17, rework-first replacement for the reverted auto pass)

| offset | name | tier | receipt |
|---|---|---|---|
| 0x6CC | `mProcNo` | [RECEIPT] | modeProc case-0 store (draft:2140) + ptmf dispatch *0x1C (draft:2144) |
| 0xB70 | `mOrderType` | [RECEIPT] | eventOrder selector (draft:2153); cleared by checkOrder (draft:2192,2204); auto regex missed the temp_r5 load |
| 0x6D2 | `mBckIdx` | [RECEIPT] | pBckIdx arg of dLib_bcks_setAnm (draft:1348; d_lib.h) |
| 0x6D3 | `mPrmIdx` | [RECEIPT] | setAnm arg store (draft:1336) + pPrmIdx arg (draft:1348; d_lib.h) |
| 0x84C | `mpMorf` | [RECEIPT] | morf arg of dLib_bcks_setAnm (draft:1348); donor idiom |
| 0x870 | `mAcch` | [RECEIPT] | dBgS_ObjAcch& arg of dLib_getWaterY (draft:1461); donor-wide mAcch (53/63) |

6 markers burned: so 171 -> 165.
