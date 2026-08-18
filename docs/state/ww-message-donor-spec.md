# WW MESSAGE / DIALOGUE SYSTEM — DONOR SPEC, FIELD BY FIELD

**Lane:** History/Bridge · **Assignment:** CALLS row `HISTORY, HOUSING` (Integrator,
on the user's order, 2026-08-16) — *"read WW's OWN message system in
`D:\XXXXXXX\WW DP\src` and publish it field-by-field — not a prose summary."*
**Precedent:** the §991 `tag_so` decode, which caught two zeroed donor fields
because a field-by-field spec is checkable against a field-by-field fill and a
summary is not.

**DN-10 step 1.** This document is the donor's own system. No receiver reasoning
appears in it. The fork's receiver-side history lives in
[`ww-tale-dmesg-live-state.md`](ww-tale-dmesg-live-state.md) (18,641 lines,
`src/d/d_ext_dmesg.cpp`) and is deliberately NOT folded in here — that file is
the *implementation* record, this is the *donor* record, and the assignment
exists because `d_ext_dmesg` has **zero symbols on the user's vanilla image**.

---

## 0. SCOPE OF THIS PASS — READ THIS BEFORE CITING ANYTHING BELOW

**WHAT IS DECODED: STRUCTURE. WHAT IS NOT: FLOW.**

Every offset below is transcribed from donor *headers*. The headers give the
shape of the data; **they do not give the sequence.** How a message advances
from `MSG_PREPARING` to `BOX_OPENING` to `MSG_TYPING`, what resets
`mWaitRest`, when `mShortCutFlag` is honoured — all of that lives in the `.cpp`
bodies, which this pass has NOT read:

| donor TU | lines | read? |
|---|---|---|
| `src/f_op/f_op_msg_mng.cpp` | 7637 | **NO** |
| `src/d/d_msg.cpp` | 3117 | **NO** |
| `src/d/d_mesg.cpp` | 2257 | **NO** |
| `src/d/d_message.cpp` | 1262 | **NO** |
| `src/d/d_message_paper.cpp` | 325 | **NO** |
| `src/f_op/f_op_msg.cpp` | 88 | **NO** |
| `src/JSystem/JMessage/{control,data,processor,resource}.cpp` | — | **NO** |

**A fill built only from this document will have correct field widths and wrong
behaviour.** Flow is pass 2. Nothing here may be cited as "the donor does X" —
only as "the donor stores X".

---

## 0.5 DECOMP STATUS — MEASURED 2026-08-16, AND IT CHANGES WHAT PASS 2 CAN BE

**I did not check this before publishing §0, and the user asked whether I had.
I had not — I inferred "decompiled" from file size.** Measured with
`tools/foundry/decomp_status.py` (two axes: `configure.py` authoritative +
source-marker count):

| donor TU | lines | AXIS A | markers |
|---|---|---|---|
| `f_op/f_op_msg.cpp` | 88 | **MATCHED** | 0 |
| `JSystem/JMessage/control.cpp` | — | **MATCHED** | 0 |
| `JSystem/JMessage/data.cpp` | — | **MATCHED** | 0 |
| `JSystem/JMessage/processor.cpp` | — | NONMATCHING | 4 |
| `JSystem/JMessage/resource.cpp` | — | NONMATCHING | 2 |
| `d/d_msg.cpp` | 3117 | NONMATCHING | 5 |
| `d/d_mesg.cpp` | 2257 | NONMATCHING | 7 |
| `d/d_message.cpp` | 1262 | NONMATCHING | 7 |
| `d/d_message_paper.cpp` | 325 | NONMATCHING | **55** |
| `f_op/f_op_msg_mng.cpp` | 7637 | NONMATCHING | 13 |

**Axes AGREE on all ten.** 3 MATCHED · 7 NONMATCHING · **93 markers total.**

**WHAT THIS MEANS, stated plainly:**

1. **THE MESSAGE SYSTEM IS NOT VERBATIM-CLEAN, AND THE PORT PRECEDENT IT WAS
   ASSIGNED AGAINST WAS.** `tag_so` and `tag_kb_item` both landed as *"MATCHED 0
   markers BOTH AXES (verbatim-clean per the user's gate ruling)"*. **This system
   is the opposite case and there is no precedent here for porting one.** Whether
   the gate ruling admits a NONMATCHING donor at all is the **user's call, not
   mine** — flagged, not decided.
2. **THE STRUCTURE IN THIS DOCUMENT IS UNAFFECTED.** Every offset in §1–§6 comes
   from *headers*, and field layout is pinned by the ABI regardless of whether a
   function body matches. **Nothing above is withdrawn.**
3. **PASS 2 IS DEMOTED FROM "READ THE FLOW" TO "READ THE FLOW, THEN VERIFY IT."**
   A NONMATCHING body is a *reconstruction*: it compiles to something that is not
   what shipped. Citing it as "the donor's own algorithm" is exactly the DN-10
   step-1 claim the estate keeps catching — the source is the donor's *project*,
   not the donor's *binary*.
4. **THE SPLIT IS USEFUL, AND IT FALLS ON A SEAM WORTH KNOWING:**
   - **CITABLE AS DONOR TRUTH:** `f_op_msg.cpp` (the process shell) ·
     `JMessage/control.cpp` · `JMessage/data.cpp`. **The message-code resolution
     and data-entry path — including the §4 packing question — sits mostly in
     MATCHED code.**
   - **RECONSTRUCTION, VERIFY BEFORE CITING:** the tag interpreter
     (`processor.cpp`, 4) and the resource layer (`resource.cpp`, 2) — **so W1's
     engine is HALF matched, and the non-matching half is the tag interpreter,
     which IS the dialogue feature set** · the whole `d_*` message layer ·
     `f_op_msg_mng.cpp` (13 across 7,637 lines).
   - **`d_message_paper.cpp` IS BARELY DECOMPILED: 55 markers in 325 lines**, a
     marker roughly every six lines. Treat it as unavailable.
5. **ON THE INSTRUMENT ITSELF:** `decomp_status.py` is on `control.py`'s
   **UNDECLARED** list — no negative control is registered for it, so by the
   estate's own rule its verdicts are UNVERIFIABLE rather than clean (№31-C).
   Two things partly offset that and neither is a declared control: its
   `--selftest` passes 8/8 checks against a 1,093-TU live corpus, and **this run
   produced BOTH verdicts — 3 MATCHED and 7 NONMATCHING — so the tool is
   demonstrably not stuck on one answer.** That is evidence, not a gate.

---

## 1. THE PROCESS SHAPE

WW messages are **fpc leaf-draw processes**, not actors — the same process
machinery the actor port already solved, one profile kind over.

```
struct msg_process_profile_definition {
    /* 0x00 */ leaf_process_profile_definition base;
    /* 0x24 */ msg_method_class* sub_method;   // subclass methods
};

struct msg_method_class {
    /* 0x00 */ leafdraw_method_class base;
};
```

`f_op_msg_mng.h:42`, `f_op_msg.h:36`. Note this is a **fifth profile tier** of
exactly the kind the outgoing Housing handoff flagged as unmirrored
(`actor_process_profile_definition2`); the message tier is `leaf`-based, not
`actor`-based.

### `msg_class` — the message process itself · **size 0xFC**

```
/* 0x00 */ leafdraw_class base;
/* 0xC0 */ int             mMsgType;
/* 0xC4 */ create_tag_class draw_tag;
/* 0xD8 */ msg_method_class* sub_method;
/* 0xDC */ fopAc_ac_c*     mpActor;      // the talking actor
/* 0xE0 */ cXyz            mPos;
/* 0xEC */ u32             mMsgNo;
/* 0xF0 */ u32             field_0xf0;   // UNNAMED
/* 0xF4 */ u32             field_0xf4;   // UNNAMED
/* 0xF8 */ u16             mStatus;      // fopMsg_MessageStatus_e
/* 0xFA */ u8              mSelectNum;   // index of the choice the player picked
```
`f_op_msg.h:40`. **`mStatus` at 0xF8 is a `u16` holding a 24-value enum** —
width matters for any mirror.

### Creation parameters · `fopMsg_prm_class` **size 0x1C**

```
/* 0x00 */ fopAc_ac_c* mpActor;
/* 0x04 */ cXyz        mPos;
/* 0x10 */ u32         mMsgNo;
/* 0x14 */ u32         field_0x14;   // UNNAMED
/* 0x18 */ int         field_0x18;   // UNNAMED
```

### `fopMsg_prm_timer : fopMsg_prm_class` · **size 0x34**

```
/* 0x1C */ int  mTimerMode;
/* 0x20 */ u16  mLimitTimeMs;
/* 0x22 */ u8   mShowType;
/* 0x23 */ u8   mIconType;
/* 0x24 */ cXy  mTimerPos;
/* 0x2C */ cXy  mRupeePos;
```
`f_op_msg_mng.h:47/55`. The timer variant is a **separate parameter block**, so
a plugin-side creator that assumes one prm shape will truncate it.

---

## 2. THE STATE MACHINE — `fopMsg_MessageStatus_e`, 24 states, 0x00–0x17

`f_op_msg.h:9`. Transcribed verbatim; comments are the donor's own.

| val | name | donor comment |
|---|---|---|
| 0x00 | `fopMsgStts_MSG_UNK0_e` | |
| 0x01 | `fopMsgStts_MSG_PREPARING_e` | |
| 0x02 | `fopMsgStts_BOX_OPENING_e` | |
| 0x03 | `fopMsgStts_SCOPE_OPENING_1_e` | Scope opening (picto box type) |
| 0x04 | `fopMsgStts_SCOPE_OPENING_2_e` | Scope opening (demo type) |
| 0x05 | `fopMsgStts_MSG_UNK5_e` | |
| 0x06 | `fopMsgStts_MSG_TYPING_e` | |
| 0x07 | `fopMsgStts_STOP_e` | |
| 0x08 | `fopMsgStts_SELECT_2_e` | Selection box with 2 choices |
| 0x09 | `fopMsgStts_SELECT_3_e` | Selection box with 3 choices |
| 0x0A | `fopMsgStts_CLOSE_WAIT_e` | Waiting for player input before closing |
| 0x0B | `fopMsgStts_SCOPE_ACTIVE_e` | Scope active/idle |
| 0x0C | `fopMsgStts_SCOPE_DEMO_e` | Scope demo (cutscene) mode |
| 0x0D | `fopMsgStts_SCOPE_WAIT_e` | Scope waiting for event/demo |
| 0x0E | `fopMsgStts_MSG_DISPLAYED_e` | |
| 0x0F | `fopMsgStts_MSG_CONTINUES_e` | |
| 0x10 | `fopMsgStts_MSG_ENDS_e` | |
| 0x11 | `fopMsgStts_BOX_CLOSING_e` | |
| 0x12 | `fopMsgStts_BOX_CLOSED_e` | |
| 0x13 | `fopMsgStts_MSG_DESTROYED_e` | |
| 0x14 | `fopMsgStts_SELECT_YOKO_e` | 2 choices arranged horizontally |
| 0x15 | `fopMsgStts_INPUT_e` | WW song tutorial (practising) / number input (auction) |
| 0x16 | `fopMsgStts_TACT_e` | close box after playing a WW song |
| 0x17 | `fopMsgStts_DEMO_e` | WW song tutorial (demonstration) |

**SIX of these 24 states are WW-exclusive subject matter** — `SCOPE_*` ×4 is the
Picto Box, `TACT_e`/`DEMO_e`/`INPUT_e` are the Wind Waker conductor. **The
receiver's message enum has no counterpart for them**, so the status field is a
donor-value surface, not a mappable one — the same class as the §843 particle
IDs (WW names, TP values) and it must carry donor values.

---

## 3. THE WW SUBCLASS · `sub_mesg_class : msg_class` — 0xFC → 0x168

```
/* 0x0FC */ JKRExpHeap*        heap;
/* 0x100 */ JKRExpHeap*        field_0x100;    // UNNAMED, second heap
/* 0x104 */ dMesg_outFont_c*   outfont[18];    // 18 slots
/* 0x14C */ dMesg_screenData_c* screen;
/* 0x150 */ u8                 field_0x150[4]; // UNNAMED gap
/* 0x154 */ char*              text[4];        // 4 text lines
/* 0x164 */ u8                 field_0x164;    // UNNAMED
```
`d_mesg.h:19`. **The message owns its own `JKRExpHeap` — two of them.** Any
plugin-side port that allocates from the host heap instead has changed the
lifetime model, not just the allocator.

### `dMesg_outFont_c` — the per-glyph draw record, 18 live at once

```
/* 0x04 */ J2DPicture* icon;
/* 0x08 */ J2DPicture* kage;      // "kage" = shadow
/* 0x0C */ int  field_0xc;
/* 0x10 */ s16  field_0x10;
/* 0x12 */ s16  field_0x12;
/* 0x14 */ s16  field_0x14;
/* 0x16 */ s16  mTimer;
/* 0x18 */ u8   mAlpha;
/* 0x19 */ u8   field_0x19;
```

---

## 4. THE JMESSAGE ENGINE BINDING

### `JMessage::TControl` (JSystem base) — `JMessage/control.h:10`

```
/* 0x04 */ TResourceContainer*  mResourceContainer;
/* 0x08 */ mutable TResource*   mResource;
/* 0x0C */ TSequenceProcessor*  mBaseProcSeq;
/* 0x10 */ TRenderingProcessor* mBaseProcRender;
/* 0x14 */ u16                  mGroupID;
/* 0x16 */ u16                  mMessageIndex;
/* 0x18 */ const void*          mMessageEntry;
/* 0x1C */ const char*          mMessageDataStart;
/* 0x20 */ const char*          mMessageDataCurrent;
/* 0x24 */ const char*          mCurrentText;
/* 0x28 */ TProcessor::TStack_  mRenderStack;
```

### `dMesg_tControl : JMessage::TControl` — WW's extension, from 0x3C

```
/* 0x3C */ JUTFont* mMainFont;
/* 0x40 */ JUTFont* mRubyFont;       // furigana — a SECOND font, always
/* 0x44 */ f32      mLineLength[4];
/* 0x54 */ int      mLineCount;
/* 0x58 */ int      mLineStart;
/* 0x5C */ int      mInitFontSize;
/* 0x60 */ int      mNowFontSize;
/* 0x64 */ int      mCharSpace;
/* 0x68 */ int      mCharCode;
/* 0x6C */ int      mTextBoxWidth;
/* 0x70 */ u8       mbHeader;
/* 0x71 */ u8       mCode16Fg;
```
`d_mesg.h:54`. **Gap 0x2C–0x3B is unaccounted for in the header** — `mRenderStack`
at 0x28 has undeclared size here. **Do not assume 0x3C is packed; measure it.**

### ⚠ TWO DIFFERENT MESSAGE-CODE PACKINGS IN ONE SYSTEM — REVIEW ITEM

- `JMessage::TControl`: **`(groupID << 16) | messageIndex`** — `control.h:18,55`
- `fopMsgM_msgGet_c::getMesgNumber()`: **`(mGroupID << 8) | mResMsgNo`** —
  `f_op_msg_mng.h:100`

**These are not the same packing and they are not the same index.** `msgGet_c`
carries `mMsgNo` *and* `mResMsgNo` as separate `u16`s, so there is a message
number and a *resource-local* message number, and the shift differs with it.
**This is exactly the §991 class of finding** — a port that picks one packing
for both will resolve the wrong string for every message past group 0, and it
will look like a content bug rather than a field bug.

### Processors

`dMesg_tSequenceProcessor : JMessage::TSequenceProcessor` (`d_mesg.h:107`) —
fields 0x038–0x163; named: `mMesg` 0x038, `mNowColor` 0x06C, `mWaitRest` 0x090,
`mStopFlag` 0x15F, `mShortCutFlag` 0x161. **Two 100-byte char buffers inline at
0x097 and 0x0FB.** Virtuals: `do_begin/do_end/do_isReady/do_jump/do_branch_query/
do_branch/do_character/do_tag/do_systemTagCode` — **the tag interpreter is the
dialogue feature set** (colour, pause, ruby, branch, select).

`dMesg_tMeasureProcessor` (0x38–0x54) measures without drawing;
`dMesg_tRenderingProcessor` has vtable only, no new fields.

---

## 5. THE SCREEN LAYER

### `fopMsgM_pane_class` · **size 0x38** — `f_op_msg_mng.h:72`

```
/* 0x00 */ J2DPane*          pane;
/* 0x04 */ fopMsgM_f2d_class mPosTopLeftOrig;   // {f32 x, y}
/* 0x0C */ fopMsgM_f2d_class mPosTopLeft;
/* 0x14 */ fopMsgM_f2d_class mPosCenterOrig;
/* 0x1C */ fopMsgM_f2d_class mPosCenter;
/* 0x24 */ fopMsgM_f2d_class mSizeOrig;
/* 0x2C */ fopMsgM_f2d_class mSize;
/* 0x34 */ u8                mInitAlpha;
/* 0x35 */ u8                mNowAlpha;
/* 0x36 */ s16               mUserArea;
```
**Every pane keeps an ORIG and a live copy of position and size** — the open/close
animation is expressed as interpolation between them, not as a separate tween.

### `dMesg_screenData_c : dDlst_base_c` — `d_mesg.h:202`

```
/* 0x004 */ JKRExpHeap*         mHeap;
/* 0x008 */ sub_mesg_class*     mMesg;
/* 0x00C */ J2DScreen*          scrn;
/* 0x010 */ JUTFont*            field_0x10;   // UNNAMED (main)
/* 0x014 */ JUTFont*            field_0x14;   // UNNAMED (ruby)
/* 0x018 */ fopMsgM_pane_class  field_0x18;
/* 0x050 */ fopMsgM_pane_class  field_0x50;
/* 0x088 */ fopMsgM_pane_class  field_0x88[4];   // the 4 text lines
/* 0x168 */ fopMsgM_pane_class  field_0x168;
/* 0x1A0 */ u8                  field_0x1a0[4];
/* 0x1A4 */ int                 field_0x1a4;
/* 0x1A8 */ s16                 mTimer;
/* 0x1AA */ u8                  field_0x1aa[2];
/* 0x1AC */ JUtility::TColor    field_0x1ac;
/* 0x1B0 */ JUtility::TColor    field_0x1b0;
```
Pure virtuals `openAnime()` / `closeAnime()` — **a subclass MUST supply both**.

Subclasses: `dMesg_screenDataTalk_c` (+0x1B4 `f32`) and `dMesg_screenDataItem_c`
(+0x1B4 → 0x3EC, carrying 8 more panes, a **`JPABaseEmitter*`** at 0x3E4 and a
**`ResTIMG* texBuffer`** at 0x3EC).

---

## 6. DATA ACCESS — where the strings come from

```
class fopMsgM_msgGet_c {            class fopMsgM_itemMsgGet_c {
  /* 0x04 */ u32 mMsgIdx;             /* 0x04 */ u32 mMsgIdx;
  /* 0x08 */ u16 mGroupID;            /* 0x08 */ u16 mMsgNo;
  /* 0x0A */ u16 mMsgNo;              /* 0x0A */ u16 mResMsgNo;
  /* 0x0C */ u16 mResMsgNo;         };
};
```
**`itemMsgGet_c` has NO `mGroupID`** — the item variant is group-implicit, and its
field at 0x08 is `mMsgNo` where the general variant holds `mGroupID`. **The two
classes are NOT layout-compatible at 0x08 despite looking parallel.**

Both expose `getMesgHeader / getMesgInfo / getMesgData / getMesgEntry /
getMessage` over `mesg_header` · `mesg_info` · `mesg_data` · `JMSMesgEntry_c`
(`f_op_msg_mng.h:17-22`) — **struct bodies not yet transcribed; pass 2.**

Archives are the user's own ISO bytes, unmodified: `bmgres.arc` / `bmgresh.arc`
at `res/Object/`, reached as
`dComIfG_getObjectRes("bmgres", "zel_00.bmg")`.

---

## 7. THE WIRES THIS SYSTEM NEEDS — DERIVED, NOT GUESSED

The user's instruction was that the dialogue port needs **four other wires,
system-based, alongside it**. I did not pick four; I read what the donor's own
message system refuses to run without. It comes to **four load-bearing systems**,
plus two I am explicitly excluding and saying why.

| # | wire | why it is load-bearing (donor evidence) |
|---|---|---|
| **W1** | **JMessage engine** — `TControl`, `TResource`, `TResourceContainer`, `TSequenceProcessor`, `TRenderingProcessor`, `TProcessor::TStack_` | `dMesg_tControl` **IS** a `JMessage::TControl` (extends it at 0x3C). Tag interpretation, branching, message-code resolution and the render stack all live here. Nothing draws without it. |
| **W2** | **JUTFont** — glyph source, main **and ruby** | `mMainFont` 0x3C / `mRubyFont` 0x40; `changeFont(JUTFont*)` is virtual on every screen subclass; `ruby_character()` on the sequence processor. **Two fonts, always** — a one-font port silently drops furigana. |
| **W3** | **J2D screen layer** — `J2DScreen`, `J2DPane`, `J2DPicture` | The box *is* a `J2DScreen` (`scrn` 0x00C); every moving part is a `fopMsgM_pane_class` wrapping a `J2DPane`; glyphs and their shadows are `J2DPicture` (`icon`/`kage`). |
| **W4** | **`fopMsg` leaf-process manager** — `f_op_msg.h` + `f_op_msg_mng.h` (7,637 lines) | `msg_class` is a `leafdraw_class` created from a `msg_process_profile_definition` with its own `prm` blocks and the 24-state machine. This is the *lifecycle*; W1–W3 are what it drives. |

**EXCLUDED FROM THE FOUR, and these are judgement calls the Integrator should
overrule if they disagree:**

- **`JKRExpHeap`** — real and load-bearing (`sub_mesg_class` owns two), but it is
  an *allocator*, not a system to port; it is a lifetime constraint on however
  W4 lands. **Flagged, not queued.**
- **`JPABaseEmitter`** — appears only in `dMesg_screenDataItem_c` (item-get
  sparkle). Leaf polish on one screen subclass; the talk box does not touch it.
  **Not a wire; a dependency of one variant.**

---

## 7.5 PASS 2, PART 1 — THE PROCESS SHELL, FROM **MATCHED** SOURCE

`src/f_op/f_op_msg.cpp` is **MATCHED, 0 markers, 88 lines** — the only core flow
in this system citable as donor truth today. Read in full. Addresses are the
donor's own (`8002A688`–`8002A860`).

**`g_fopMsg_Method`** = `{ Create, Delete, Execute, IsDelete, Draw }` — the
`leafdraw_method_class` slot order.

- **`fopMsg_Create`** — on `fpcM_IsFirstCreating`: `mMsgType =
  fpcBs_MakeOfType(&fopMsg_MSG_TYPE)` (a file-static handle) · `sub_method =
  profile->sub_method` · `fopDwTg_Init(&draw_tag, this)` · then
  `fopMsgM_GetAppend(this)` returns a `fopMsg_prm_class*` and **five fields are
  copied**. Then `fpcMtd_Create(sub_method)`; **only on `cPhs_COMPLEATE_e`** does
  it call `fopDwTg_ToDrawQ(&draw_tag, fpcM_DrawPriority(this))`.
- **`fopMsg_Execute`** — **gated: `if (!dScnPly_ply_c::isPause())`**. While paused
  the sub-method never runs and the method returns 1. **Behavioural fact, not a
  layout one, and it is donor truth rather than reconstruction.**
- **`fopMsg_IsDelete`** — `fpcMtd_IsDelete(sub_method)`; **iff it returns 1**,
  `fopDwTg_DrawQTo(&draw_tag)`.
- **`fopMsg_Delete`** — `fpcMtd_Delete(sub_method)`, then `fopDwTg_DrawQTo`
  **unconditionally**.
- **`fopMsg_Draw`** — pure forward to `fpcLf_DrawMethod(sub_method, this)`.

### This closes §8 item 5 — `field_0xf0` / `field_0xf4` are no longer opaque

They are **copied verbatim from the creation parameters**: `field_0xf0 =
prm->field_0x14` and `field_0xf4 = prm->field_0x18`. Still unnamed on both
sides, but their **provenance is pinned** — they are caller-supplied at create
time, not computed. **A fill must carry them through, and zeroing them silently
drops whatever the caller passed** (the §991 zeroed-field failure exactly).

### ⚠ CORRECTION TO §7 — I MISSED A DEPENDENCY, AND IT TURNS OUT NOT TO BE A WIRE

The shell binds **`fopDwTg_*` (the f_op draw-tag queue)** and
**`dScnPly_ply_c::isPause()`**. Neither appears in my four-wire table — **the
wire list was drawn from headers, and this one only shows up in a body.**

**Measured rather than assumed, on BOTH images** (`upstream_conformance.py
--symbol`):

| symbol | our fork | vanilla upstream |
|---|---|---|
| `fopDwTg_ToDrawQ` | SAFE, one entry | **SAFE, one entry** |
| `fopDwTg_DrawQTo` | SAFE, one entry | **SAFE, one entry** |
| `fopDwTg_Init` | SAFE, one entry | **SAFE, one entry** |
| `fpcLf_DrawMethod` | SAFE, one entry | **SAFE, one entry** |
| `fpcBs_MakeOfType` | SAFE, one entry | **SAFE, one entry** |

**So it is NOT a fifth wire — it is receiver infrastructure already present on a
clean dusklight**, and `dScnPly_c::isPause()` is at `include/d/d_s_play.h:80`.
**The message process shell has somewhere to stand on vanilla.** That is the
first measured evidence that this port is feasible plugin-side, and it is the
same shape as the stage-loader finding: the *policy* was fork-only, the
*machinery* was not.

**THE CAVEAT, per the Integrator's standing rule: `--symbol SAFE` IS LEG 1 OF 3.**
It proves the name resolves uniquely. It does **not** prove a call site reaches
it (**inlining** — the host warns `'<sym>' was inlined into callers`) and does
**not** prove the loader can bind it (**MULTI_NAME** — needs `--find` on the
user's actual image). **Do not approve a binding on this table alone.**

---

## 7.6 PASS 2, PART 2 — THE RESOLUTION CHAIN AND THE BMG RECORD, FROM **MATCHED** SOURCE

`JMessage/control.cpp` and `JMessage/data.cpp` are **MATCHED, 0 markers**.
Addresses `8029E930`–`8029ED88`.

### ⚠ THE §4 "TWO PACKINGS" QUESTION IS RESOLVED — AND MY FRAMING OF IT WAS WRONG

I flagged them as **rival encodings, unreconciled**. They are not rivals. **They
are two different layers, and the engine half is now settled from MATCHED code:**

- **ENGINE (JMessage) — `(groupID << 16) | messageIndex`.** `setMessageCode(u32
  packed)` splits it as `packed >> 16` / `packed & 0xFFFF` and the whole
  resolution chain runs on that pair. **Authoritative, MATCHED.**
- **GAME (`fopMsgM_msgGet_c::getMesgNumber`) — `(mGroupID << 8) | mResMsgNo`.**
  This is a **WW game-side numbering**, not an engine message code. **Its consumer
  is in `f_op_msg_mng.cpp`, which is NONMATCHING — so what reads this number is
  still UNRESOLVED and I am not guessing it.**

**The correction that matters for a fill: do not feed a `getMesgNumber()` result
to `setMessageCode()`.** They are different namespaces with different shift
widths. My §4 warning said "pick one and every message past group 0 breaks" —
**the real hazard is narrower and sharper: crossing the two layers.**

### The resolution chain, end to end (all MATCHED)

```
setMessageCode(groupID, messageIndex) -> setMessageCode_flush_()
  reset_()
  mMessageEntry = getMessageEntry(groupID, messageIndex)
      getResource_groupID(groupID)              // cache-then-container
          isResourceCached_groupID()  -> mResource if its INF1 groupID matches
          mResourceContainer->Get_groupID(groupID)
      resource->getMessageEntry(messageIndex)
          if (messageIndex >= messageEntryNumber) return NULL      // BOUNDS
          return INF1.content + (messageIndex * messageEntrySize)  // FIXED STRIDE
  offs = *(u32*)mMessageEntry                   // FIRST u32 OF THE ENTRY
  mMessageDataStart = mResource->mMessageData + offs
```

**THREE FACTS A FILL CANNOT INVENT:** the entry table is **fixed-stride**, and the
stride is data (`messageEntrySize` from INF1) — **not a compile-time constant** ·
**the first `u32` of every entry is an OFFSET into the DAT1 block**, not text ·
**`groupID` is matched against the resource's own INF1 field**, so a resource
"is" its group.

### BMG container — `data.h` / `data.cpp`, MATCHED

- **Signature `'MESG'`** (`ga4cSignature`). Header: signature `+0x0`, type `+0x4`,
  **blockNumber `+0xC`**, encoding `+0x10`; **content begins at `+0x20`**.
- **INF1 · `JUTMesgInfo`** — `header` 0x00 (`JUTDataBlockHeader`, 8) ·
  `messageEntryNumber` **0x08 u16** · `messageEntrySize` **0x0A u16** · `groupID`
  **0x0C u16** · `defaultColor` 0x0E u8 · `reserved` 0x0F · `messageEntryTable[]`
  **0x10**.
- **MID1 · `JUTMesgIDData`** — `mHeader` 0x00 · `numEntries` 0x08 u16 · `format`
  0x0A u8 · `info` 0x0B u8 · `reserved[4]` 0x0C · `messageIDTable[]` 0x10.
- **TAG PACKING IS A THIRD ENCODING AND DIFFERS AGAIN:** `getTagCode(tag) = tag &
  0xFFFF`, `getTagGroup(tag) = (tag >> 0x10) & 0xFF` — **the tag group is masked
  to 8 bits**, unlike the message groupID which is a full `u16`. Three encodings
  live in this system, not two.

### `TResource` · `TResourceContainer`

```
TResource:  /* 0x00 */ JGadget::TLinkListNode mLinkNode;
            /* 0x08 */ TParse_THeader        mHeader;
            /* 0x0C */ TParse_TBlock_info    mInfo;
            /* 0x10 */ const char*           mMessageData;      // DAT1
            /* 0x14 */ const char*           mStringAttribute;  // STR1/FLW?
            /* 0x18 */ JUTMesgIDData*        mMessageID;        // MID1
TResourceContainer (TLinkList_factory, keyed by groupID):
            /* 0x10 */ u8   mEncoding;
            /* 0x14 */ bool (*mIsLeadByteFunc)(int);
```
**`mIsLeadByteFunc` is a MULTIBYTE-TEXT hook** — the container carries an encoding
and a lead-byte predicate, so text is not assumed single-byte. **A port that
treats message data as ASCII will corrupt every multi-byte glyph.**

### Driver loop

`update()` — bails on `isReady_update_()`; **first call latches `mCurrentText =
mMessageDataStart` and calls `mBaseProcSeq->setBegin(entry, dataStart)`**; then
`mCurrentText = mBaseProcSeq->process(NULL)`; a NULL return clears
`mMessageDataStart` and returns false (**message finished**).
`render()` — `setBegin(mMessageEntry, mMessageDataCurrent)`, **copies
`mRenderStack` into the processor**, then `process(mCurrentText)`. **Note it
begins from `mMessageDataCurrent` but processes from `mCurrentText`** — two
different pointers, and conflating them is a silent rendering bug.
`reset_()` clears the four pointers and `mRenderStack.clear()`.

**`do_word()` returns NULL in the base** — WW overrides it (`dMesg_tControl::do_word`).

---

## 7.7 THE MESSAGE ENTRY RECORD — `JMSMesgEntry_c`, and a CONFLICT between two decoders

### The record a fill actually reads · **0x18 bytes** · `f_op_msg_mng.h:22`

```
/* 0x00 */ u32 mDataOffs;          // offset into DAT1  <-- CROSS-CONFIRMED, see below
/* 0x04 */ u16 mMsgNo;
/* 0x06 */ s16 mItemPrice;         // SIGNED
/* 0x08 */ u16 mNextMsgNo;         // message chaining lives in DATA
/* 0x0A */ u16 field_0x0a;         // UNNAMED
/* 0x0C */ u8  mTextboxType;
/* 0x0D */ u8  mDrawType;
/* 0x0E */ u8  mTextboxPosition;
/* 0x0F */ u8  mItemImage;
/* 0x10 */ u8  mTextAlignment;
/* 0x11 */ u8  mInitialSound;
/* 0x12 */ u8  mInitialCamera;
/* 0x13 */ u8  mInitialAnimation;
/* 0x14 */ u8  field_0x14;         // UNNAMED
/* 0x15 */ u8  field_0x15;         // UNNAMED
/* 0x16 */ u8  field_0x16;         // UNNAMED
/* 0x17 */ u8  field_0x17;         // UNNAMED
```

**`mDataOffs` AT 0x00 IS INDEPENDENTLY CORROBORATED:** the MATCHED engine does
`u32 offs = *(u32*)messageEntry` (§7.6) — two sources, one derived from headers
and one from matching code, agreeing on the same four bytes. **That is a real
cross-check, not two readings of one file.**

**THE PRESENTATION IS DATA-DRIVEN, AND THIS IS THE BEST NEWS IN THE DECODE.**
Textbox type, draw type, position, alignment, item image, and the initial
**sound / camera / animation** are all per-message FIELDS. **A plugin that reads
this record faithfully gets WW-correct presentation without reimplementing the
logic that chose it** — the behaviour is in the user's own disc bytes. It also
means `mNextMsgNo` makes message chaining a data property, not control flow.

### ⚠ CONFLICT — TWO STRUCTS DESCRIBE THE INF1 HEADER AND THEY DO NOT AGREE

| field | `JUTMesgInfo` (engine, **MATCHED**) | `mesg_info` (game-side, **NONMATCHING**) |
|---|---|---|
| entry count | `messageEntryNumber` **0x08** u16 | `mNumEntry` **0x08** u16 |
| entry stride | `messageEntrySize` **0x0A** u16 | `mEntrySize` **0x0A** u16 |
| group | `groupID` **0x0C** u16 | `mGroupID` **0x0C** u16 |
| colour | `defaultColor` **0x0E** u8 | `mColor` **0x10** u8 |
| **entry table** | `messageEntryTable[]` **0x10** | `mEntries[]` **0x14** |

**THE FIRST THREE AGREE. THE LAST TWO DO NOT, AND THE ENTRY TABLE IS THE ONE THAT
MATTERS: 0x10 vs 0x14 IS A FOUR-BYTE MISALIGNMENT ON EVERY MESSAGE IN THE FILE.**
A fill built on the wrong one does not fail loudly — it reads each entry shifted,
so `mDataOffs` picks up `mMsgNo`+`mItemPrice` and every string resolves to
garbage or out of bounds.

**WHICH ONE I WOULD TRUST, AND WHY IT IS NOT A RULING.** The engine's version is
**MATCHED** — its accessor `mInfo.getContent() + (messageIndex *
getMessageEntrySize())` compiles to the shipped instructions with
`messageEntryTable` at 0x10, so 0x10 is validated by codegen. `mesg_info` lives
in **NONMATCHING** `f_op_msg_mng.cpp`, leaves 0x0E–0x0F and 0x11–0x13
unaccounted, and has the shape of a hand-written approximation. **So the weight
is strongly on 0x10 — but I am recording this as a CONFLICT to be settled
against the user's own BMG bytes, not ruling it from source rank alone.** This is
exactly the case R5's tier system exists for: LAW is proven against the binary,
and neither of these has been.

### ✅ SETTLED AGAINST THE USER'S OWN DISC BYTES — **0x10 IS CORRECT**

Measured on `D:\XXXXXXX\Ex WW\files\res\Msg\bmgres.arc` (RARC, uncompressed;
one `MESG` at `0x540`). **This is a measurement, not a source-rank argument.**

```
BMG header @ 0x540 : type 'bmg1' · blockNumber 2 · encoding 1
INF1 @ +0x20       : numEntries 4411 · entrySize 0x18 · groupID 0
                     byte@0x0E 0x00 · byte@0x0F 0x00
                     byte@0x10 0x00 · bytes 0x11-0x13 = 00 00 01
DAT1 @ 0x1A300     : size 0x823A0
```

**`entrySize` IS 0x18 — exactly `sizeof(JMSMesgEntry_c)`.** The record layout in
§7.7 is confirmed by the file itself.

| table offset | first six `mDataOffs` | ascending | inside DAT1 |
|---|---|---|---|
| **0x10 — engine, MATCHED** | `0x1, 0x35, 0x70, 0x120, 0x1cb, 0x202` | 39/39 | **40/40** |
| 0x14 — game, NONMATCHING | `0x10000, 0x20000, 0x30000, …` | 39/39 | 8/40 |
| *0x12 — deliberate control* | `0x10001, 0x350002, …` | 39/39 | 1/40 |

**VERDICT: `messageEntryTable[]` is at 0x10. `mesg_info`'s `mEntries[]` at 0x14
is WRONG**, and the NONMATCHING TU is where it lives — the reconstruction is the
one that drifted.

**THE FAILURE MODE IS SELF-DEMONSTRATING:** reading at 0x14 yields `0x10000,
0x20000, 0x30000 …` — that is `mMsgNo << 16 | mItemPrice` of each entry, i.e.
**precisely the predicted four-byte shift, visible in the data.** And `bytes
0x10-0x13 = 00 00 00 01` is entry[0]'s `mDataOffs = 1`, corroborating from the
other side.

**THE TEST COULD HAVE FAILED AND DID NOT:** a deliberately-wrong offset (0x12)
was scored alongside the two candidates and came last (1/40 in range). Had it
scored level, the probe would have reported INCONCLUSIVE rather than a verdict —
**the discrimination is demonstrated, not assumed.**

### ✅ RE-DERIVED WITH THE ESTATE'S OWN READER — AND MY FIRST PROBE WAS A DUPLICATE INSTRUMENT

**METHOD ERROR, MINE:** I hand-rolled a RARC/Yaz0 scanner for the first probe.
**`tools/foundry/ww_disc.py` already is the donor disc reader (FST, RARC, Yaz0)**
and is listed in `workflow.py` §3. I did not check the tool map before writing a
parallel one. It worked, which is not the same as it being right to write.

Re-run through `ww_disc.rarc_list` (reader `--selftest` PASSES first: 361
`camstyle.dat` records, matching the shipped reader). **Same verdict, and the
sanctioned reader supplies three facts a raw `MESG` byte-scan could not:**

```
bmgres.arc contains 2 members:
   color.bmc      1,088 bytes   magic 'MGCL'
   zel_00.bmg   639,328 bytes   magic 'MESG'
zel_00.bmg: type 'bmg1' · blockNumber 2 · encoding 1
            INF1 numEntries 4411 · entrySize 0x18 · groupID 0
  [0x10 engine] 0x1, 0x35, 0x70, 0x120, 0x1cb   in-DAT1 40/40
  [0x14 game]   0x10000, 0x20000, 0x30000, …    in-DAT1  8/40
  [0x12 control]0x10001, 0x350002, …            in-DAT1  1/40
```

1. **THE MEMBER NAME IS `zel_00.bmg`** — exactly the name the receiver requests
   as `dComIfG_getObjectRes("bmgres", "zel_00.bmg")`. **The consumer's name and
   the disc's content are confirmed to be the same object**, which had been
   assumed everywhere and measured nowhere.
2. **⚠ THERE IS A SECOND MEMBER NOBODY HAS MENTIONED: `color.bmc`, magic
   `MGCL`.** A colour companion to the message archive — almost certainly what
   backs `defaultColor` (INF1 0x0E) and `dMesg_tSequenceProcessor::mNowColor`
   (§4). **A port that serves only the `.bmg` ships the text without its colour
   table.** → **NOW FULLY DECODED IN §7.9** — `MGCL`/`CLT1`, a byte-indexed
   `RGBA8[256]` palette, every offset closing.
3. **THE 32-BYTE-UNIT FINDING IS NOW PROVEN AGAINST THE RARC-REPORTED FILE
   LENGTH**, not my arithmetic on the enclosing archive: `0x4E0B × 32 = 639328 =
   len(zel_00.bmg)`, exact.

**Two independent readers, one hand-written and one selftested, agree on the
verdict.** That is corroboration rather than repetition.

### ⚠ BONUS FINDING — THE BMG HEADER SIZE FIELD IS IN **32-BYTE UNITS**

Header `+0x08` reads **0x4E0B**, while the BMG plainly spans far more than that.
`640672 (arc) − 0x540 (BMG start) = 639328 = 0x4E0B × 32`, **exact**. So that
field counts 32-byte blocks, not bytes. **A fill that bounds-checks message
offsets against it as a byte count will under-read by 32× and reject almost every
valid message.** The engine's `TParse_THeader` does not expose `+0x08` at all,
which is why this never surfaced in source.

---

## 7.8 THE PROCESSOR — layout, the tag dispatch contract, and a HARD DEPTH LIMIT

From `JMessage/processor.h`. **The TU is NONMATCHING but this is the HEADER** —
field offsets, vtable slot annotations and inline bodies are layout facts; only
the out-of-line bodies are reconstruction.

```
TProcessor:  /* 0x00 */ vtable
             /* 0x04 */ TControl*   mControl;
             /* 0x08 */ const char* mCurrent;
             /* 0x0C */ TStack_     mStack;       // 0x14
             /* 0x20 */ TStatusData_ mStatusData; // 0x14
                                                  // => sizeof 0x34
TSequenceProcessor : TProcessor
             /* 0x34 */ int mStatus;              // => 0x38
```

**✅ CROSS-CHECK THAT VALIDATES §4:** `dMesg_tSequenceProcessor`'s own fields
begin at **0x038** (§4), and `TSequenceProcessor` ends at exactly **0x38**. Two
independent transcriptions meeting at the same boundary — **the base-class chain
in this spec is consistent, which is not something a single reading proves.**

### ⚠ NESTING DEPTH IS HARD-CAPPED AT 4, AND `push()` DOES NOT ENFORCE IT

```
TStack_:  /* 0x00 */ const char* mStack[4];
          /* 0x10 */ u32         mNum;
          IsStorable() const { return mNum < 4; }
          push(str) { mStack[mNum] = str; mNum++; }   // NO bounds check
```
**`push()` writes without checking; `IsStorable()` is the CALLER's obligation.**
Jump/branch/select nesting deeper than four overruns the array. **A port that
reimplements the stack with a different depth changes what messages are legal,
and one that reimplements it without the caller-side check corrupts memory on
deep nesting.** Four is a donor constant, not an implementation detail.

### THE TAG DISPATCH CONTRACT — the extension point the whole port hangs on

```
void on_tag(u32 tag, const void* data, u32 size) {
    if (!do_tag(tag, data, size)) {   // subclass gets FIRST REFUSAL
        do_tag_(tag, data, size);     // base handles what the subclass declined
    }
}
```
**Two-tier: the subclass returns `true` to consume a tag, `false` to defer.**
That is precisely how `dMesg_tSequenceProcessor::do_tag` / `do_systemTagCode`
(§4) hook WW's own tag vocabulary onto the generic engine. **Collapse this into a
single handler and WW's custom tags are either swallowed or dropped — silently,
because neither path errors.** `do_tag_` / `do_systemTagCode_` / `do_begin_` /
`do_end_` are **pure virtual** on `TProcessor`: a subclass MUST supply all four.

### Flow-control vocabulary (virtuals, so this IS the feature set)

- **select:** `do_select_begin(u32)` · `do_select_end()` · `do_select_separate()`
- **jump:** `do_jump_isReady()` · `do_jump(entry, data)`
- **branch:** `do_branch_query(u16)` · `do_branch_queryResult()` · `do_branch(...)`
- **status:** `kStatus_Begin · kStatus_End · kStatus_Normal · kStatus_Jump ·
  kStatus_Branch` (0–4) in `mStatus` @0x34
- **character-end mode switches:** `mStatusData.mCallBack` defaults to
  `process_onCharacterEnd_normal_` and swaps to `process_onCharacterEnd_select_`
  — **so choice boxes change per-character behaviour, not just layout.**

`setBegin(entry, data)` is **`reset_(data)` THEN `do_begin_(entry, data)`** —
order is part of the contract.

### ⚠ A FOURTH ADDRESSING SCHEME: MESSAGE **IDs**, VIA MID1

`setBegin_messageID(u32, u32, bool*)` and `toMessageCode_messageID(u32, u32,
bool*)` convert a message **ID** into a message **code**, backed by the **MID1**
block (`TResource::mMessageID`, `JUTMesgIDData`, §7.6). **So the system carries
FOUR distinct addressing schemes, not three:** engine code
`(groupID<<16)|messageIndex` · WW game number `(groupID<<8)|mResMsgNo` · tag
`(group8<<16)|code16` · **and message ID, resolved through MID1.** The `bool*`
out-parameter suggests the conversion can fail and reports it. **Any fill must be
explicit about which scheme it holds at every boundary** — this is the §4 hazard
generalised.

**⚠ CORRECTION TO MYSELF, MEASURED IN §7.10 — I OVERSTATED THIS.** I wrote that
four schemes are in play and called it *"the single most likely source of silent
wrong-string bugs in this port."* **WW'S OWN DATA CARRIES NO MID1 BLOCK.** Both
`zel_00.bmg` and `zel_01.bmg` declare `blockNumber 2` and walk to exactly
`INF1 + DAT1`. **The message-ID path is real ENGINE surface and DEAD for this
game's data.** Three schemes are in play, not four — **the port is simpler than I
said.** The path stays documented because a fill could still call it by accident,
but it is not a live hazard.

---

## 7.9 `color.bmc` / **MGCL** — FULLY DECODED, a format no lane had named

The second member of `bmgres.arc` (§7.6 re-derivation). **Nobody had mentioned
it; it is not in any census.** Decoded from disc bytes via `ww_disc.rarc_list`.
**Every offset below closes arithmetically — no field is inferred.**

```
MGCL FILE HEADER — 0x20 bytes
  0x00  'MGCL' 'bmc1'      8-byte magic, SAME SHAPE as BMG's 'MESG' 'bmg1'
  0x08  u32  0x22 = 34     SIZE IN 32-BYTE UNITS -> 34 x 32 = 1088 = len   ✓
  0x0C  u32  1             blockNumber
  0x10  u8   0             encoding (unused here)
  0x14  ..                 zero padding to 0x20
  0x20  content begins     (same +0x20 convention as BMG)

CLT1 BLOCK @ 0x20
  +0x00  'CLT1'
  +0x04  u32  0x420        block size;  0x20 + 0x420 = 0x440 = EOF exactly  ✓
  +0x08  u16  256          entry count
  +0x0A  u16  0            pad
  +0x0C  RGBA8[256]        file 0x2C .. 0x42C — ALL 256 alphas are 0xFF      ✓
  0x42C .. 0x440           20 bytes ZERO padding to the 32-byte boundary     ✓
```

### ⚠ THIS INDEPENDENTLY CONFIRMS THE 32-BYTE-UNIT FINDING ON A SECOND FORMAT

`color.bmc` is a different file type by a different magic, and **its size field
uses the same 32-byte-unit convention** (`34 × 32 = 1088`). The BMG result
(§7.6, `0x4E0B × 32 = 639328`) is therefore **a convention of the family, not a
quirk of one file** — and the trailing zero padding on both is explained by the
same alignment. **Two formats, one rule, measured separately.**

### The palette — 256 entries, 9 distinct colours

| idx | RGB | idx | RGB |
|---|---|---|---|
| 0 | `FFFFFF` white | 5 | `00FFFF` cyan |
| 1 | `FF6400` orange | 6 | `FF00FF` magenta |
| 2 | `00FF00` green | 7 | `828282` grey |
| 3 | `7878FF` blue | 8 | `FF8000` orange (bright) |
| 4 | `FFFF3C` yellow | 9–255 | `FFFFFF` (default white) |

**A BYTE-INDEXED PALETTE — which is exactly what INF1's `defaultColor` (a u8 at
0x0E, §7.6) and `dMesg_tSequenceProcessor::mNowColor` (§4) index into.** The
colour tag in a message selects a palette slot; the slot's RGBA lives here.
**Indices 9–255 all resolve to white, so an out-of-range colour degrades to
white rather than to garbage — that is the donor's own behaviour, not a guard to
add.**

### What this means for the port

**A plugin serving only `zel_00.bmg` ships the text without its colour table**,
and every coloured word in WW renders white. The archive must be served whole.
`bmgres.arc` is already served from the user's ISO at `res/Object/` — **this
member comes along with it and needs no new serve, only a consumer that reads
it.** Two files, one archive, and the colour half was invisible until the disc
was opened.

---

## 7.10 THE RESOURCE MAP — [H1] answered, and W3's data located

Walked all 35 archives in `D:\XXXXXXX\Ex WW\files\res\Msg` with
`ww_disc.rarc_list`. **The block walk cross-checks itself — walked count vs the
header's declared `blockNumber`. Both BMGs AGREE, so the walk is trusted.**

### The text: two BMGs, and the groupID mechanism made concrete

| archive | member | blocks | entries | **groupID** |
|---|---|---|---|---|
| `bmgres.arc` | `zel_00.bmg` 639,328 B | INF1 + DAT1 | **4411** | **0** |
| `bmgresh.arc` | `zel_01.bmg` 1,504 B | INF1 + DAT1 | **15** | **1** |

**THIS IS WHAT `getResource_groupID()` RESOLVES AGAINST (§7.6): ONE ARCHIVE PER
GROUP.** The container's group→resource lookup is not abstract — **it is an
archive selector**, and mounting only `bmgres.arc` cannot answer group 1 at all.

### ⚠⚠ CORRECTION 2026-08-16 — THE BYTES BELOW ARE RIGHT, THE WORD "LEFTOVER" WAS WRONG

I described group 1 as **"15 leftover Japanese lines"**, and a lane flagged the
conflict with `d_ext_dmesg.h:17`, which calls `zel_01.bmg` the **"Hylian
variants"**. I routed it to Foundry as a disc question. **It was not a disc
question — it fell to reading the donor's own selector, and the header was right
and I was wrong.**

**`fopMsgM_itemMsgGet_c::getMesgHeader` (donor `f_op_msg_mng.cpp:1611`) chooses
the archive PER MESSAGE:**

```c
if (fopMsgM_hyrule_language_check(i_msgNo)) {
    arc = dComIfGp_getMsgDt2Archive();   // bmgresh -> zel_01.bmg
} else {
    arc = dComIfGp_getMsgDtArchive();    // bmgres  -> zel_00.bmg
}
```

**And `fopMsgM_hyrule_language_check` (`:8002AD4C`) is exactly fifteen msgNos,
gated on the clear count:**

```c
if (dComIfGs_getClearCount() != 0) return false;   // readable after one clear
switch (i_msgNo) {
    case 0xD49: case 0xD4B: case 0xD4D: case 0xD4F: case 0xD51:
    case 0xD53: case 0xD54: case 0xD55: case 0xD57: case 0xD59:
    case 0x1178: case 0x117A: case 0x1389: case 0x138A: case 0x196E:
        return true;
```

**MY COUNT OF 15 WAS RIGHT AND MY READING OF WHY WAS WRONG.** These are not
residue. They are the **Hylian-script lines**, served untranslated on a first
playthrough and replaced by `zel_00.bmg` text **after `getClearCount() != 0`** —
Wind Waker's New Game+ "you can now read Hylian" mechanic, implemented as an
archive swap keyed on message number.

The decoded content below is the tell I already had and misread: it is
Ganondorf addressing the King. **Live story text, not dead bytes.**

**CONSEQUENCE FOR THE PORT:** `bmgresh.arc` is **not** "resident even if unused"
(`d_ext_dmesg.h:17`) — it is *used*, for 15 messages, on every save that has not
cleared the game. A port that mounts only `bmgres.arc` renders those 15 lines in
the wrong script, and **only for players who have not finished the game**, which
is the worst possible reproduction profile.

### GROUP 1 IS SHIFT-JIS ENCODED — AND THE `encoding` FIELD DOES NOT SAY SO

| | `zel_00.bmg` | `zel_01.bmg` |
|---|---|---|
| declared `encoding` @0x10 | **1** | **1** |
| high bytes (≥0x80) in DAT1 | 5473 / 533400 = **1.0%** | 605 / 1080 = **56.0%** |
| actual text | single-byte English | **Shift-JIS katakana** |

**BOTH DECLARE `encoding = 1`, AND THEY ARE NOT THE SAME ENCODING.** So the
header field **cannot** be used to choose a decoder — **a fill that branches on
`encoding` will mangle every string in group 1.** This is the concrete instance
of the multibyte hazard §7.6 could only state abstractly: the real discriminator
is `TResourceContainer`'s **`mIsLeadByteFunc`** predicate, set per container, not
a value in the file.

Decoded (Shift-JIS): `ヒサシブリダナ　ハイラルオウヨ` — *"it has been a long
time, King of Hyrule"* · `モシヤ　デンセツノ　トキノユウシャカ？` — *"could
this be the legendary Hero of Time?"*

**These are LEFTOVER JAPANESE MESSAGES in an English build** — 15 of them,
`msgNo` 3401–6510, all `mTextboxType = 12`. **Archaeology, not gameplay.**

**PRACTICAL CONSEQUENCE, and it revises the sentence above: mounting
`bmgresh.arc` is mechanically required to answer group 1, but group 1 carries no
English gameplay text — so it is CORRECTNESS-OPTIONAL for a first port** and
should not gate anything. Stated plainly because "must mount both" without this
would have Housing chasing a dependency that buys nothing.

### ✅ [H1] ANSWERED — **NO MID1 BLOCK EXISTS IN EITHER FILE**

Both declare `blockNumber 2` and walk to exactly `INF1 + DAT1`. **WW addresses
messages by INDEX only; the MID1 / message-ID path is dead for this data**
(§7.8 corrected accordingly). **The prediction was made before the walk and a
MID1 would have falsified it.**

### `color.bmc` is BYTE-IDENTICAL in both archives

SHA-1 `c9e471b39a36f7f3…` in each — **duplicated, not split.** A consumer needs
one table and either copy serves.

### ⚠ W3's DATA IS LOCATED — the message box is a BLO screen set

`dMesg_screenData_c` builds a `J2DScreen` in `createScreen()` (§5). **Its data is
three more archives nobody had connected to this system:**

| archive | members | what |
|---|---|---|
| `msgres.arc` | **30** | `hukidashi_*.blo` — *hukidashi* = speech bubble. **The message-box J2D screens.** |
| `dmsgres.arc` | 8 | `hukidashi_d*.blo` + `*_d.bti` — the **demo/cutscene** variants |
| `tmsgres.arc` | 3 | `baton_input.blo` (uncompressed, magic `SCRN`) + 2 BTI |

**`tmsgres.arc` IS THE CONDUCTOR SCREEN**, and it lands exactly on the
WW-exclusive states from §2 — `INPUT_e` (song practice / number entry), `TACT_e`,
`DEMO_e`. **The six states with no receiver counterpart have their own resources
on the user's disc**, which is what makes them portable rather than merely
unmapped.

**FOR HOUSING: W3 is not "port the J2D screen layer" — it is "load the donor's
own BLO screens and drive them".** Layouts, textures and demo variants are all
disc data already.

**NOT decoded: which `.blo` maps to which `mTextboxType`** (§7.7's per-message
field). That mapping lives in NONMATCHING `d_mesg.cpp`. **Flagged, not guessed.**

---

## 7.11 THE SCREENS DECODED — pane names, and four unnamed struct fields identified

13 `SCRN`/`blo1` screens across `msgres` / `dmsgres` / `tmsgres`. **Every block
walk self-checks (declared block count vs walked) and ALL PASS.** Pane tags are
at block offset **+0x0C**, established by scoring three candidate offsets across
122 pane blocks: **+0x0C printable in 100/122 (82%), +0x10 and +0x14 in 0/122.**

**THE REAL VALIDATION IS SEMANTIC, NOT STATISTICAL:** the extracted tags read
`ROOT`, `BACK`, `cur1`, `txt0` — **random bytes do not spell "ROOT".** The 22
non-printable tags are unnamed panes (J2D permits them), not decode failures.

### `hukidashi_00.blo` — the main textbox, 25 panes

```
PIC1:BACK
PAN1:ROOT
  PIC1:ms00                    the box graphic
  TBX1:txt0  tx01  tx02  tx03  <- FOUR TEXT LINES
  PIC1:yz00                    yazirushi = ARROW
  PIC1:dt00                    dot
  PIC1:mc00                    choice group, nesting:
    PIC1:mc01 .. mc09
    PIC1:cur1  cur2            cursors
  TBX1:txc0  txc1  txc2  tec3  choice text
```

### ✅ FOUR UNNAMED FIELDS IN `dMesg_screenData_c` ARE NOW IDENTIFIED

**`fopMsgM_pane_class field_0x88[4]` (§5) IS THE FOUR TEXT LINES** — and that is
corroborated by **three independent code sites** already in this spec:
`sub_mesg_class::text[4]` (§3) · `dMesg_tControl::mLineLength[4]` (§4) ·
`field_0x88[4]` itself. **Four in the struct, four in the file.**

| disc pane | code it drives |
|---|---|
| `TBX1 txt0/tx01/tx02/tx03` | `field_0x88[4]`, `text[4]`, `mLineLength[4]` |
| `PIC1 yz00` (*yazirushi* = arrow) | `arwAnimeInit()` / `arwAnime()` (§5) |
| `PIC1 dt00` (dot) | `dotAnimeInit()` / `dotAnime()` (§5) |
| `PIC1 mc00–mc09`, `cur1`, `cur2` | the SELECT states `SELECT_2/3/YOKO` (§2) |
| `TBX1 txc0/txc1/txc2/tec3` | choice text for those states |

**The `arwAnime` / `dotAnime` method names in §5 were opaque; the disc names
them.** `ring_loght.bti` in the same archive likewise answers
`dMesg_screenDataItem_c::ringMove()` / `lightMove()`.

### Variants

`hukidashi_05.blo` is `ROOT` + `tx80..tx83` only — **four text lines, no
decoration**: the minimal box. `hukidashi_02.blo` carries **eight** TBX panes
(`tx20…tx30`) with four arrows (`yz20…yz23`) and two dots — a multi-region
variant. `hukidashi_14_*` are PIC-only (no TBX): frame/decoration sets.

### ❌ TESTED AND FAILED — `mTextboxType` IS **NOT** THE `.blo` FILENAME NUMBER

The screens are named `hukidashi_00/02/05/07/08/09/14`, and `mTextboxType` is a
small per-message integer, so "type N selects `hukidashi_NN.blo`" is the obvious
hypothesis. **I tested it against all 4411 entries and it does not hold.**

```
mTextboxType observed:  0(3319) 1(38) 2(24) 5(22) 6(6) 7(27) 8(13)
                        9(193) 10(550) 11(181) 12(15) 13(14) 14(9)
msgres .blo numbers:    00  02  05  07  08  09  14
```
**Seven types have a same-numbered screen. SIX DO NOT — 1, 6, 10, 11, 12, 13 —
and types 10 (550 uses) and 11 (181) are the 2nd and 4th most common in the
game.** A rule that misses the second-most-used case is not the rule. Adding
`dmsgres` (`hukidashi_d00`, `d09`) and `tmsgres` (`baton_input`) supplies three
more screens, not six. **So the selector is genuinely indirect and remains in
NONMATCHING `d_mesg.cpp`. UNKNOWN — hypothesis tested, rejected, not massaged.**

### ✅ WHAT THE SWEEP DID ESTABLISH — the three presentation enums, bounded by data

| field | observed domain | note |
|---|---|---|
| `mTextboxType` @0x0C | **13 distinct**: 0,1,2,5,6,7,8,9,10,11,12,13,14 | type 0 is **75%** of all messages (3319/4411); **3 and 4 never occur** |
| `mDrawType` @0x0D | **3 values**: 0 (2717) · 1 (713) · 2 (981) | a 3-way enum, all live |
| `mTextboxPosition` @0x0E | **4 values**: 0 (2315) · 1 (121) · 2 (118) · 3 (1857) | 0 and 3 dominate; 1 and 2 are rare but real |

**These bound the enums empirically** — a fill can validate incoming values and
flag anything outside these sets as data it has never seen. **That is worth more
than a guessed mapping.**

**~~Observation, offered as a coincidence rather than a claim:~~ RESOLVED
2026-08-16 — IT WAS NOT CHANCE, AND `mTextboxType == 12` IS THE HYLIAN MARKER.**

I wrote that `mTextboxType 12` occurs exactly **15** times in `zel_00.bmg` while
`zel_01.bmg` holds exactly **15** messages, all type 12 — and declined to claim a
relationship. **The relationship is now measured, three ways, and all three sets
are IDENTICAL:**

| source | what it is | msgNos |
|---|---|---|
| donor `fopMsgM_hyrule_language_check` (`8002AD4C`) | CODE — which messages swap archive | 15 |
| `zel_01.bmg` INF1 (disc) | DATA — the Hylian variants | 15 |
| `zel_00.bmg` entries with `mTextboxType == 12` (disc) | DATA — the readable versions | 15 |

`0xD49 0xD4B 0xD4D 0xD4F 0xD51 0xD53 0xD54 0xD55 0xD57 0xD59 0x1178 0x117A
0x1389 0x138A 0x196E` — **set-equal across all three, verified against the user's
own disc (`res/Msg/bmgres.arc`, `bmgresh.arc`), not inferred.**

**SO THE FIFTEEN MESSAGES EXIST IN BOTH ARCHIVES**: the readable text in
`zel_00.bmg` and the Hylian text in `zel_01.bmg`, with
`dComIfGs_getClearCount()` selecting between them per playthrough.

**AND `mTextboxType 12` IS THEREFORE NOT AN UNEXPLAINED ENUM VALUE — it is the
data-side Hylian flag**, mirroring the code-side check. A fill can identify the
Hylian set from `zel_00.bmg` alone, without hardcoding fifteen message numbers.

*(Recorded because the discipline paid: the equal counts were noted and
explicitly NOT claimed. Had they been asserted as a mechanism when first seen,
the right answer would have arrived by luck and been indistinguishable from the
many times that guess is wrong.)*

---

## 7.12 ✅ [H2] THE TAG VOCABULARY — RECOVERED FROM DISC BYTES, NOT FROM THE INTERPRETER

**I had routed [H2] to Foundry as blocked on "what instrument verifies a
reconstruction". It did not need one.** The tag interpreter is NONMATCHING, but
**the tags themselves are DATA**, and data is settleable the same way INF1 was.

### The escape shape — confirmed, and it could have failed

```
0x1A  <totalLen:u8>  <group:u8>  <code:u16 BE>  [operand bytes…]
```
**Stepping by `totalLen` from every `0x1A` lands on text, newline, NUL or another
escape in 8888 of 8888 cases — 100.0%.** The probe was written to print
*"SHAPE NOT CONFIRMED"* and report no vocabulary below 90%.

Escape lengths observed: **5** (2251) · **6** (5232) · **7** (1405) — i.e. zero,
one or two operand bytes.

### The observed vocabulary — this is what WW's data actually uses

| group | count | reading |
|---|---|---|
| **0xFF** | 5376 | the dominant group; `code 0x0000` alone is **5232** |
| 0x00 | 3360 | system/prompt group, many codes |
| 0x01 / 0x02 / 0x03 | 100 / 35 / 17 | small special groups |

**⚠ NOTE THE GROUP BYTE IS 0xFF, WHICH VALIDATES §7.6's THIRD ENCODING:**
`getTagGroup` masks to 8 bits (`(tag >> 0x10) & 0xFF`). A full-`u16` group
assumption would still work here, but the mask is real and 0xFF is its top value.

### ✅ THE COLOUR TAG IS `group 0xFF · code 0x0000 · one operand byte`

Rendered in context, from real messages:

```
Tingle appeared on your <FF:0000:01>Game Boy Advance<FF:0000:00>!
Would you like to call Tingle?  <00:0008>Yes<FF:0000:02> No
```

**The operand is a CLT1 PALETTE INDEX (§7.9).** `:01` = `FF6400` orange on a
highlighted proper noun, `:00` = `FFFFFF` back to white, `:02` = `00FF00` green.
**That is the selector §7.9 was missing — the palette and its index tag are now
joined**, and both came off the user's own disc rather than out of NONMATCHING
code. **5232 uses makes it the single most common tag in the game.**

### Choice options are bracketed by `group 0x00`, codes `0x001E` / `0x001F`

```
Would you like to format?  <00:001E>Yes<00:001F>No
```
Seen across every Memory Card prompt. **These drive the `SELECT_2` / `SELECT_3`
states (§2) and the `txc0/txc1/txc2/tec3` choice panes (§7.11)** — the three
layers now line up: state machine, screen pane, and the byte in the text.

`group 0x00 code 0x0007` (1209) is the next most common and is **not identified**
— unknown, not guessed.

**WHAT THIS DOES AND DOES NOT ESTABLISH.** It establishes the **wire format and
the live vocabulary with counts** — a fill can parse every message in the game
from this. It does **not** establish what each unidentified `group 0x00` code
*does*; that remains interpreter behaviour. **But the two that matter most for
rendering — colour and choice — are now pinned to data.**

---

## 7.13 THE TAG SEMANTICS — identified from usage, with the confidence stated per row

Continuation of §7.12. The interpreter is NONMATCHING, so meaning is constrained
by **how the shipped script uses each tag**, across 4411 messages. **Operand-byte
counts are exact; readings are labelled CONFIRMED / PATTERN / UNNAMED and the
weak ones are not promoted.**

### CONFIRMED — usage is unambiguous across many messages

| tag | operands | reading | evidence |
|---|---|---|---|
| `00:0000` | **0** (336×) | **PLAYER NAME** | *"Well done, `<0000>`! With the shards…"* · *"That is our destination, `<0000>`…"* — always in direct address, never elsewhere |
| `00:0007` | **2** (1155×) | **WAIT / PAUSE, u16 operand** | *"…your shield…`<0007:0014>` Do you get it?"* · *"`<0007:000a>`That's worth 1 Rupee"* — mid-sentence, operand varies (0x14=20, 0x0A=10) |
| `00:000A` `000B` `000E` `000F` `0010` `0011` `0013` | **0** | **CONTROLLER BUTTON GLYPHS, inline** | *"Press `<000A>` to grab and lift things"* · *"press `<000A>` to climb in"* · *"`<0013>`Pick Warp `<000A>`Decide `<000B>`Cancel"* |

**⚠ `0010` / `0011` / `000F` ALWAYS APPEAR AS A SET OF THREE** — *"Set this to and
use it with `<0010>`, `<0011>`, or `<000F>`"*, in five separate item
descriptions. **That is WW's three item slots (X / Y / Z).** The specific
button-to-code mapping is **not** established here — only that the three are one
set and that the family is button glyphs.

### ✅ `00:0007` JOINS DIRECTLY TO A FIELD ALREADY IN THIS SPEC

`dMesg_tSequenceProcessor` carries **`mWaitRest` @0x090** with `resetWaitRest()`
/ `decWaitRest()` (§4) — a countdown drained one per call. **A wait tag with a
u16 operand and a per-frame countdown field are the two halves of one
mechanism**, reached from opposite directions: the field from a header, the tag
from the disc. **That is a code↔data join, not two guesses agreeing.**

### PATTERN — real and matched, but I am not naming it

`00:0001` (**304×**) and `00:0002` (**306×**), both zero-operand, appear as a
**matched pair bracketing the first line** of every item-get message:
```
<0001>You got a <FF:0000:02>Green Rupee<FF:0000:00>!<0002>
<0007:000a>That's worth <FF:0000:06>1 Rupee<FF:0000:00>!
```
**Counts within 2 of each other across the whole file is strong evidence of an
open/close span.** Whether it is a font-size span, a banner/box-type switch, or a
"headline line" marker **is not decidable from usage alone — UNNAMED.**

### `00:0008` is a choice marker, and it is NOT the same one as `001E`/`001F`

`00:0008` (213×, 0 operands) precedes the **first option** of an in-world
question — *"Would you like to call Tingle?\n`<0008>`Yes\nNo"*, *"Postage due is
201 Rupees. Will you pay?\n`<0008>`Yes\nNo"*. The Memory Card prompts instead use
`001E`/`001F` to bracket **both** options (§7.12). **Two distinct choice
mechanisms exist** — plausibly in-world dialogue vs. system/menu — and the port
must not collapse them.

### Still UNNAMED

`00:0016` (62) · `00:0004` (44) · `00:0039` (34) · `00:0012` `00:0013` `00:0015`
`00:000C` `00:000D` `00:0017` `00:001E`… and groups `0x01`/`0x02`/`0x03` (100 /
35 / 17 uses). **Reported as unknown rather than inferred.**

**WHAT A FILL CAN NOW DO:** parse every message, render colour correctly, insert
button glyphs, substitute the player name, honour waits, and drive both choice
mechanisms. **What it still cannot do is claim to know what the unnamed tags
mean.**

---

## 8. OPEN / UNKNOWN — the honest column

1. ~~**Flow is entirely unread** (§0). Structure only.~~ **SUPERSEDED — the
   MATCHED flow IS read** (§7.5 process shell · §7.6 resolution chain + BMG
   container · §7.8 processor contract · §7.9 MGCL). **WHAT REMAINS UNREAD IS
   THE NONMATCHING FLOW**, and that is a different claim: the tag interpreter
   (`processor.cpp`), the whole `d_*` message layer, and the 7,637-line
   `f_op_msg_mng.cpp`. **Reading those yields reconstructions, not donor
   algorithm** (§0.5) — so the remaining work needs verification against the
   binary, not more source reading. **§0's blanket warning no longer describes
   this document; §0.5's citable/reconstruction split does.**
2. **PARTLY RESOLVED, AND MY ORIGINAL WORDING CONFLATED TWO THINGS.** I called
   `mesg_header`/`mesg_info`/`mesg_data`/`JMSMesgEntry_c` "the BMG record layout".
   **They are not — they are WW's own GAME-SIDE wrappers** (`f_op_msg_mng.h`).
   **The actual BMG record layout is `JUTMesgInfo` / `JUTMesgIDData` / the `MESG`
   header, and those are now fully decoded from MATCHED source in §7.6.** The
   game-side `mesg_*` structs remain untranscribed and their definitions are not
   in the headers read so far.
3. ~~**`dMesg_tControl` offset 0x2C–0x3B is unaccounted** — `mRenderStack` size is
   not declared in `control.h`. **Measure before mirroring.**~~ **RESOLVED — THERE
   IS NO GAP.** `processor.h` (§7.8) declares `TProcessor::TStack_` as
   `const char* mStack[4]` (0x00–0x0F) + `u32 mNum` (0x10) = **0x14 bytes**.
   `TControl::mRenderStack` sits at 0x28, so `0x28 + 0x14 = 0x3C` — **exactly
   where `dMesg_tControl`'s own fields begin. The layout IS packed, and it is now
   derived from a declared size rather than assumed.** The warning stands only in
   the sense that it was right to refuse to assume it: the answer came from a
   different header, not from the one that looked authoritative.
4. **RESOLVED FOR THE ENGINE HALF (§7.6), STILL OPEN FOR THE GAME HALF.** The
   engine uses `(groupID << 16) | messageIndex` — MATCHED, settled. What consumes
   `getMesgNumber()`'s `(groupID << 8) | mResMsgNo` lives in NONMATCHING
   `f_op_msg_mng.cpp` and is **not guessed**. **And there is a THIRD encoding:
   tag group is masked to 8 bits (`(tag >> 0x10) & 0xFF`) while message groupID
   is a full `u16`.**
5. ~~**`field_0xf0` / `field_0xf4` on `msg_class`** are unnamed `u32`s sitting
   directly before `mStatus`. Unknown, not empty.~~ **RESOLVED §7.5 from MATCHED
   source: both are copied verbatim from the creation prm (`field_0x14` /
   `field_0x18`). Still unnamed; provenance pinned. A fill MUST carry them
   through — zeroing them drops caller-supplied data silently.**
6. **Six WW-exclusive states (§2)** have no receiver counterpart. Donor values.

**№31-C applies to every line of §8: these are UNKNOWN, not clean.**
