# Face & Expression Recipe — how WW faces work, and porting them

> ## READ FIRST — check the DONOR DECOMP, not just the receiver
>
> **Decomp source: `D:\XXXXXXX\WW DP\src`**  (arcs: `D:\XXXXXXX\Ex WW`)
>
> Every claim in this doc was cross-checked against `d_a_npc_ls1.cpp` and our own ported
> `ls.bdl`. Where a claim comes from an external write-up and is **verified in the decomp**, it
> is marked ✓. Where it is **external and not yet confirmed here**, it is marked ⚠.

**Sources.** The architecture description is from Nathan Gordon's Wind Waker graphics analysis,
"Link's Expressions" ([medium.com/@gordonnl](https://medium.com/@gordonnl/links-expressions-eb7beae2c62c),
mirror [windwaker.graphics](https://windwaker.graphics/links-expressions.html)), and the receiver
(TP) equivalent is from Ben Jones' "Twilight Princess Eyes Breakdown"
([benjones.us](https://www.benjones.us/twilight-princess-eyes-breakdown/)). Everything below is
reconciled with the decomp and our shipped assets.

Companion to [cutscene-animation-recipe.md](cutscene-animation-recipe.md) (§ face materials).
Ledger: №188–№190.

---

## 1. The face is texture planes, not geometry

A WW face has **no sculpted eyes/brows/mouth**. They are flat textured planes raised slightly
off the head, and every expression is a **texture swap** on those planes. This is why the whole
system is so cheap and so expressive.

**Verified plane/material layout of `ls.bdl` (Aryll):**

| Feature | Material(s) | Texture | Draw mode (`ls.bdl`, verified) | ✓ |
|---|---|---|---|---|
| Eyes (the white/shape — also the **mask**) | `SC_eyeL`, `SC_eyeR` | `ls_eye` (IA4, has alpha) | **BLEND/xlu** — floating alpha plane | ✓ |
| Pupil (iris) | pupil material | `ls_hitomi` (*hitomi* = pupil) | (behind eye, masked) | ✓ |
| Eyebrows | `SC_mayuL`, `SC_mayuR` | `ls_mayu` (*mayu* = eyebrow) | **OPAQUE** (alpha-test cutout) | ✓ |
| Mouth | `SC_kuchi` | `ls_kuchi` (*kuchi* = mouth) | **OPAQUE — on the head, NOT a floating plane** | ✓ |

Left and right eyes/brows are **separate materials** — this matters for §5.

> **Not all five are floating alpha planes.** Verified on `ls.bdl`: only the **eyes** are
> BLEND/xlu (true floating alpha planes). The **mouth is OPAQUE and painted on the head mesh**,
> not a raised plane — corroborated by direct emulator observation (see
> [reference-ww-rendering-observations.md](reference-ww-rendering-observations.md)). Aryll's
> **eyebrows are OPAQUE too** (alpha-test cutout that still draws over hair). BTP still swaps the
> mouth/brow textures for expressions — a material being opaque doesn't stop texture-pattern
> animation; it only changes how it composites. **The "visible pane" adaptation bug (§2, №190)
> can therefore only affect the BLEND eye planes, never the opaque mouth.**

**Texture counts** (article, for Link): 7 eye shapes, 6 eyebrows, 9 mouths, 1 pupil. ⚠ our
`Ls.arc` carries its own set (11 expression BTPs); exact per-feature counts not tallied.

---

## 2. Pupil masking — done in the SHADER, not by geometry

The naïve assumption is that the pupil sits *behind* the eye and is occluded. **It does not.**
The eye texture is used as a **mask in the TEV/shader**, so the pupil is visible **only in the
white areas** of the eye image (article). The pupil plane is raised **slightly off** the eye
plane to avoid z-fighting (article).

**Receiver (TP) does the analogous thing differently** (Ben Jones): two geometry layers —
eyelid flush with the face, iris just underneath — and **the iris is masked by the eyelid's
alpha channel**, with the iris UVs scaled up for resolution.

> **This is the crux of the "visible eye pane" bug (№190).** The mask is a MATERIAL/TEV state,
> not part of the mesh. When a WW model is adapted into the receiver's pipeline (BDL→BMD3), if
> that eye material's blend / alpha-compare / TEV-mask state does not survive, the raised eye
> quad renders **opaque** — a visible rectangle over the face. A working eye plane is invisible
> *because of its material*, so a visible one is always a material-adaptation failure, never a
> mesh problem.

---

## 3. Three animation channels, one face

The face is driven by **three independent systems** (all confirmed in `daNpc_Ls1_c`):

### a. BTP — expression + blink (texture-pattern swap) ✓
`mBtpAnm` (`J3DAnmTexPattern`) swaps which texture frame each face material shows. **One BTP
targets all five face materials at once** (verified: `maba`, `warai`, `okori`, `kizuku`, `nozoku`,
`fuan` each list all five). So one BTP = a whole-face expression.

- **Blink** = `maba.btp` (*mabataki* = blink), resID index 1, looped by `play_btp_anm` on a
  **random 60–90 frame** hold-then-play timer. Ported in №188.
- **Expressions** = `warai` (smile), `okori` (angry), `kizuku` (notice), `nozoku` (peek /
  telescope-squint), `fuan` (unease), etc. Coupled to BCK animations via `setAnm_NUM` →
  `init_texPttrnAnm` (the `a_anm_prm_tbl` table pairs a facial resIndex with each animation). ⚠
  not yet ported for our mount.

### b. BTK — texture SRT ✓
`mBtkAnm` (`J3DAnmTextureSRTKey`) scrolls/scales the face texture coordinates. Paired with the
BTP per expression (`init_texPttrnAnm` sets both).

### c. Pupil UV offset — "looking around" ✓
`eye_ctrl()` drives two `daNpc_Ls1_matAnm_c` material animators (`mpMatAnms[0]` = one eye,
`mpMatAnms[1]` = the other). Their `calc(J3DMaterial*)` writes
**`J3DTexMtx::getTextureSRT().mTranslationX/Y`** — i.e. it **slides the pupil texture's UVs** to
aim the gaze, exactly as the article describes ("slide the UV's … to look in different
directions"). The offsets are eased with `cLib_addCalc` and clamped to ±1. The two eyes get
**mirrored X** (`fVar2 *= -1` for the second) so both pupils converge.

---

## 4. Why the blink drives the whole face (and expressions will too)

Because a single BTP addresses all five materials, binding one `mDoExt_btpAnm` animates eyes,
brows and mouth together — there is nothing separate to wire per plane. The idle blink already
does this. Porting expressions is the same channel with different resIndices, selected by the
BCK↔BTP coupling table.

---

## 5. One-eye-closed / asymmetric expressions (wink, telescope-squint)

The eyes are **separate L/R materials** (`SC_eyeL`, `SC_eyeR`). A BTP frame can therefore bind an
**open** eye texture to one and a **closed** one to the other — that is how a wink or a
telescope-squint (`nozoku`) is authored. It is not a special code path: it is a single BTP frame
with different texture indices per eye material, held for the duration of its animation.

So "make one eye close and hold" = play the expression BTP whose frame closes one eye
(candidate: `nozoku`, the peek/look-through pose) via the §3a coupling, and let its frame hold.
⚠ which resIndex closes which eye is unverified — confirm by binding each expression BTP and
observing, or by decoding the per-material texture indices in the `.btp`.

---

## 6. Porting checklist

- [ ] Model carries the face materials (`*_eye`, `*_hitomi`, `*_mayu`, `*_kuchi`) — verify per model
- [ ] **Eye material's mask/blend survived adaptation** — else the pane renders opaque (§2, №190)
- [ ] Idle blink bound (`blink_btp=`, №188 mechanism)
- [ ] Pupil UV animator (`eye_ctrl` / matAnm) — ⚠ not yet ported; gaze is currently fixed
- [ ] Expression BTP/BTK coupled to animations (`setAnm_NUM` table) — ⚠ not yet ported
- [ ] Asymmetric expressions come free once expressions are wired (§5)

---

## 7. Per-model status

| Model | Face materials | Blink | Pane renders correctly |
|---|---|---|---|
| `ls.bdl` (Aryll / NPC_LS) | ✓ 5 planes + pupil | ✓ (№188) | ✓ (invisible pane, composites) |
| `zl.bdl` (Tetra / NPC_ZL) | ✓ `zelda_eye`, `zelda_mayu` | ⚠ not bound | **✗ pane visible — §2 material bug (№190)** |
