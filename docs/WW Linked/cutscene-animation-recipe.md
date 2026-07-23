# Cutscene Animation Recipe — making a ported cast actually perform

> ## READ FIRST — check the DONOR DECOMP, not just the receiver
>
> **Decomp source: `D:\XXXXXXX\WW DP\src`**  (arcs: `D:\XXXXXXX\Ex WW`)
>
> **This document exists because that rule was broken.** Every fault below was written down in
> `D:\XXXXXXX\WW DP\src\d\actor\d_a_npc_ls1.cpp` while it was chased through eight rounds of
> receiver-side instrumentation. The donor's `daNpc_Ls1_c::demo()` and `::setMtx()` are the
> reference implementation for this entire recipe — read them before theorising.
>
> Procedure: find the donor's equivalent actor → copy its call shape **verbatim** (flags,
> argument order, sequencing) → cite the donor function at the ported call site.

Companion to [cutscene-recipe.md](cutscene-recipe.md), which covers getting the scene to *run*.
This covers getting the **cast to move**. Ledger: №154, №163–№167, №173–№184.

---

## The chain, and how each link fails

A performing actor needs **all five**. Four of five produces an actor that looks completely
inert, and each failure mode is visually identical.

| # | Link | Fails as |
|---|---|---|
| 1 | Storyboard names it (`JACT` block id) | — |
| 2 | Name resolves to a live actor | "no demo performers" |
| 3 | Engine writes a `dDemo_actor_c` | nothing |
| 4 | **Something reads it back** (`dDemo_setDemoData`) | actor stands still |
| 5 | **`modelCalc()` after the matrix** | actor stands still **elsewhere** |

---

## 1. Binding: `OBJNAME` + the ARGUMENT identity

`JSGFindObject(ID)` → `fopAcM_searchFromName(name)` → `dStage_searchName` (OBJNAME table) →
`fopAcM_Search`, whose callback matches **proc AND argument**:

```c
prm->procname == fopAcM_GetProfName(actor) && prm->argument == actor->argument
```

Two consequences:

**(a) The census name needs an OBJNAME row.** `OBJNAME("Ls1", fpcNm_NPC_HENNA0_e, 5)`.

**(b) `parameters` and `argument` are DIFFERENT fields.** `argument` is `s8` at `0x498`;
`parameters` is separate. Passing the socket arg as `parameters` (manifest selection) while
leaving `argument` at `-1` makes the actor **unfindable by census name** — the storyboard
reports "no performer" for an actor standing in plain sight.

**Stamp the identity at the MOUNT, not the spawn.** Actors of the socket proc are also placed
by the stage's own ACTR data, where `argument` comes from the placement byte — that path never
touches your spawn helper. The mount is the one point every actor passes through.

> **Never widen the arg to `-1` to make a lookup succeed.** Every islander shares the
> `NPC_HENNA0` proc (Ls1=5, Ob1=7, Ko1=8); `-1` matches whichever is found first. That is the
> identity-swap class of bug (№126/№129) arriving through a different door. **If you are
> loosening a search key to make it match, that is the bug, not the fix.**

---

## 2. Animations come from the DEMO ARCHIVE by resource INDEX

The mount's manifest keys (`idle`, `talk1`) are irrelevant here. The demo path never consults
them:

```c
// d_demo.cpp:364-382
if (anmID & 0x10000) a_name = dStage_roomControl_c::getDemoArcName();  // the demo arc
else                 a_name = i_arcName;                               // the actor's own arc
i_key = dComIfG_getObjectIDRes(a_name, anmID & 0xffff);                // by INDEX
```

So "every manifest binds only idle/talk1" is a real constraint for **behaviour** and completely
**irrelevant to cutscenes**. Verify by decoding the `JACT` track and resolving its ids against
your ported arc:

```
python tools/ww_crew_restoration_skeleton/decode_stb.py <file.stb> --verbose
```

Worked example: `JACT 'Ls1'` → `00010043` / `00010045` → bit `0x10000` set, indices `0x43`/`0x45`
→ `47_ls_bwait_l.bck` / `47_ls_kyoro_l.bck`. **The ids landing on the right filenames proves the
port preserved donor resource ordering** — worth checking before blaming anything else.

---

## 3. The read-back — `dDemo_setDemoData`, with the DONOR's flags

Binding only gives the storyboard a record to write into. Something must read it back:

```c
// donor: daNpc_Ls1_c::demo()
dDemo_setDemoData(this, 106, mpMorf, mArcName, 0, NULL,
                  dBgS_GetGndMtrlSndId_Func(current.pos, 10.0f),
                  dComIfGp_getReverb(fopAcM_GetRoomNo(this)));
```

**`106` = `0x6A` = TRANS | ROTATE | ANM | ANM_FRAME.**

Do **not** invent a flag set. "We pass a morf so enable everything" (`0xEE`) adds two bits the
donor deliberately omits:

- **`ENABLE_SCALE_e` (4)** — assigns `scale = demo_actor->getScale()`. A storyboard that never
  authors a scale hands back **(0,0,0)**: the model is scaled out of existence while every
  position probe still reads perfect.
- **`ENABLE_ANM_TRANSITION_e` (128)** — writes `i_actor->gravity`.

The call returns 0 for non-performers, so it doubles as the test — no separate `demoActorID`
check to drift out of sync.

---

## 4. `modelCalc()` — the one that hides best

```c
// donor: daNpc_Ls1_c::setMtx()
mpMorf->getModel()->setBaseScale(scale);
mDoMtx_stack_c::transS(current.pos);
mDoMtx_stack_c::ZXYrotM(mAngle);
mpMorf->getModel()->setBaseTRMtx(mDoMtx_stack_c::get());
mpMorf->calc();                                    // ← REQUIRED, every frame
```

**`setBaseTRMtx` only records where the model should be. `modelCalc()` recomputes the
joint/world matrices the renderer actually draws from.** Skip it and the model renders with
**stale joints — exactly where it was last calculated**, while position, scale, base matrix and
draw-entry all report correct.

This is the failure that survived four rounds of probes, because every instrument measured
**intent** and the joints were describing the old pose.

> **If you early-out of a native update path, mirror the donor's whole call SEQUENCE.**
> Suppressing competing writers is legitimate; silently dropping the bookkeeping the engine
> still needs from you is not. The same `return` statement caused this twice — once by skipping
> `setBaseTRMtx`, once by skipping `modelCalc()`.

---

## 5. Still to port (donor `demo()` does more than we do)

- **Face / texture-pattern animation (BTP/BTK)** — comes through the **prm / SetData channel**,
  NOT `JSGSetTextureAnimation`. There are two texture-anim channels on `dDemo_actor_c` and it is
  easy to port the wrong one (we did, №186–№187):

  | Channel | Gate | Fed by | Read via |
  |---|---|---|---|
  | `mTexAnm` | `ENABLE_TEX_ANM` (bit 8) | actor op 76 | `getTexAnmId()` |
  | **prm / SetData** | **`ENABLE_UNK_e` (bit 0)** | `JSGSetData` | **`getDemoIDData()`** |

  The donor's `daNpc_Ls1_c::demo()` uses `getP_BtpData(mArcName)`, a wrapper over the **prm**
  channel. The canonical receiver pattern (proven in `d_a_alink_demo.inc`, `d_a_horse.cpp`,
  `d_a_demo00.cpp`, `d_a_midna.cpp`):

  ```c
  if (demo_actor->checkEnable(dDemo_actor_c::ENABLE_UNK_e)) {
      int arg0, arg1, arg2; u16 resID;
      while (demo_actor->getDemoIDData(&arg0, &arg1, &arg2, &resID, NULL)) {
          // dispatch by the (arg0,arg1,arg2) triple to BCK / BTP / BTK
      }
  }
  ```

  `getDemoIDData` decodes each 32-bit prm entry: `arg0 = v>>0x1E` (2-bit type), `arg1 =
  (v>>0x18)&0xF`, `arg2 = (v>>0x10)&0xF`, `resID = v&0xFFFF`. **Each actor picks its own
  triple** — there is no universal "BTP means (x,y,z)". Find the actor's triple by logging
  `getDemoIDData`'s raw output for one run and matching it against the known resource index
  (Ls1's BTP is `0x39`). The donor's `getP_BtpData` is nonmatching, so the filter is not
  readable from source — a decode probe is the way in.
- **Texture SRT (BTK)** — same shape via `getP_BtkData(mArcName)`.
- **Held props via `ENABLE_SHAPE_e`** — the donor toggles `mTelescopeScale` on
  `demo_actor_p->getShapeId() == 1`, and `setMtx` parents the telescope to the right-hand joint
  (`getAnmMtx(m_hnd_R_jnt_num)`) with an explicit offset/rotation. **A missing held item is
  this, not a model problem.**

---

## 6. Debugging order (cheapest first)

1. `JSGFindObject … -> FOUND`? → binding (§1)
2. `demoActorID != 0` and `checkEnable(mask)` non-zero? → read-back reached (§3)
3. Actor position tracking `demo_actor->getTrans()`? → read-back working
4. **Model where the actor is?** → `modelCalc()` (§4)
5. Animating but wrong pose/face/props? → BTP/BTK/SHAPE (§5)

> **Probe honesty.** `getBaseTRMtx()[*][3]` is the **translation column** — unaffected by scale
> *and* by stale joints, so it proves POSITION, never DRAWN. A log at the top of a draw function
> proves draw was **entered**, not that the model was submitted. Both are true statements about
> the wrong quantity. **A probe that measures intent cannot falsify a claim about output** —
> and when a probe and the user's eyes disagree, the eyes are the instrument that is working.

---

## Face materials — one BTP, five planes

WW faces are texture planes, and (verified on `ls.bdl`) there are **five separate face
materials**:

| Plane | Materials |
|---|---|
| Eyes | `SC_eyeL`, `SC_eyeR` |
| Eyebrows (*mayu*) | `SC_mayuL`, `SC_mayuR` |
| Mouth (*kuchi*) | `SC_kuchi` |

**A single BTP animates all five at once.** Every expression file (`maba`=blink,
`warai`=smile, `okori`=angry, `kizuku`=notice, …) is a whole-face keyframe set targeting all
five materials — not one BTP per plane. So binding one `mDoExt_btpAnm` drives eyes, eyebrows
and mouth together; there is nothing separate to wire for the mouth or brow.

Blink (`maba.btp`) is looping (loopMode 2); expressions like `warai` are once-and-hold
(loopMode 0). The idle blink (№188) holds frame 0 (eyes open) on a random 60–90 frame timer,
then plays through and re-arms — the donor's `play_btp_anm` index-1 branch.

