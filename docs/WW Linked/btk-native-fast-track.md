# BTK native rendering — the fast-track design (Foundry, bus §215)

> User order: a cleaner way to render BTKs in the port, generalizing the grass/water
> work. Interrelated with P14 lighting (both are the visual-parity stack).

## Where it actually stands (evidence, not memory)

- The receiver plays BTK **natively and everywhere** for TP content:
  `mDoExt_btkAnm` (m_Do_ext), 100+ actor draw paths bind it (vexp).
- WW face panes already ride BTK ✓ (`face-expression-recipe.md` §b).
- **Outset `model1.btk` playback is VERIFIED ACTIVE in Engine** (§128 Housing;
  Bridge's `model1_btk_motion.md` is the motion/compositing spec — 100-frame loop,
  16 material×texGen bindings, full per-track key tables + CSVs).
- What's NOT general: the wiring is per-case, and the color axis (§112 konst/tev)
  went through Bridge-side BAKING. `d_ww_itemmdl_pc` and `d_a_ext_vegetation`
  have no BTK binding at all (grepped).

## The design: a generic BTK auto-binder socket (Engine lands; agnostic naming)

One mechanism instead of per-model wiring, at the port's model-mount choke points:

1. **Discovery:** when a mod-folder arc model is mounted (itemmdl path, ext-actor
   model path, scene/room model path), probe the same arc dir for a sibling
   `<model>.btk` (donor convention: `model.bdl`+`model.btk`, `model1.bmd`+
   `model1.btk`).
2. **Bind:** `mDoExt_btkAnm::init(modelData, btkRes, loop-per-TTK1, speed 1.0)` —
   the exact primitive TP actors use; nothing new is written, the existing player
   is reused.
3. **Advance+entry:** owner's execute/draw calls play()+entry(modelData) — the
   TP idiom (see any of the 100+ native call sites). Scene models advance on the
   stage draw tick.
4. **Covenant:** socket is donor-agnostic ("sibling texture-SRT anim auto-bind");
   donor files stay mod-folder-side; loop/duration/matrix-mode come from the
   donor TTK1 verbatim (Maya matrix-mode flag included — Bridge's report flags
   `matrix_mode 1=Maya per WW`: **[INFERENCE-NEEDED] verify the receiver
   J3D build honors the Maya texmtx convention before trusting visuals** — this
   is the classic source of "scrolls the wrong way/anchor" bugs).
5. **Retirement path:** where auto-bind covers a model, the baked-motion variant
   retires (colors/§112 remain a separate axis until measured equal).

## The verification harness (Foundry's lane)

- **Offline expected-values:** Bridge's decoder already emits per-key CSVs
  (`model1_btk_tracks.csv` etc.) — the donor law. No duplicate tool.
- **Receiver tap:** one DuskLog line (emitter-tap pattern, toggle
  `DUSK_BTK_TAP=1`): at btk entry, log model, material, texGen, frame, and the
  effective texmtx SRT — then `probe_differ` census/seq vs offline-evaluated
  expected SRT at the same frames. UV motion becomes a computed verdict
  (the windline-density precedent, applied to texture animation).
- **Donor runtime cross-check (optional):** DuskTap on the donor's
  J3DTexMtx calc site during an Outset capture — settles Hermite-interpolation
  disputes with measured donor values.

## Outside tools (intake candidates, T3 reference unless promoted)

- **J3DUltra** (Sage-of-Mirrors) — modern C++ J3D lib with BTK support; candidate
  offline evaluator for expected-SRT generation if Bridge's CSVs need
  interpolation (Hermite eval) rather than key tables.
- **noclip.website** J3D/BTK TypeScript — reference-implementation tier (standing
  caution applies); good for cross-checking Maya-mode matrix composition.
- **CloudModding TTK1 docs** — already Bridge's decode source.
- **gclib J3D (partial)** — in-house Python already adopted; check TTK1 coverage
  before adding a new dependency.

## Ferries

- **ENGINE:** the auto-binder socket (steps 1-3; smallest version = itemmdl +
  ext-actor paths, scene models second) + the Maya-mode verification.
  **— §218 STATUS (History covering Engine):**
  - **EXT-ACTOR PATH — LANDED + BUILT.** `d_ext_npc_mount.cpp` `tryBindModel`: at
    model bind, when there is no explicit manifest `btk=`, derive the sibling
    `<model>.btk` (donor convention `model.bdl`+`model.btk`), and if the arc
    carries it, `init` it on the body `mDoExt_btkAnm` with `EMode_LOOP` and
    `play()` it every frame (new `mColorBtkPlay` flag; entry site already existed).
    Reuses the existing `mpColorBtk` slot — manifest `btk=` stays the STATIC
    color-select (Vlupy, `EMode_NONE`, `setFrame`); the sibling probe is the new
    ANIMATED case. Donor-agnostic, no-op for models without a sibling btk. Logs
    `[ExtNpcMount] §218 btk auto-bind '<name>' → <proc> (loop)`. **Build CLEAN.**
    This covers **every socketed WW model** (flowers, ship, furniture, props).
  - **MAYA-MODE — VERIFIED, [INFERENCE-NEEDED] CLOSED.** The receiver J3D honors
    the Maya texmtx convention: `J3DShapeMtx.cpp` cases 5/11 read the matrix_mode
    bit `mInfo & 0x80` and route to `J3DGetTextureMtxMaya`/`…MayaOld` when set
    (basic composer otherwise). BTK animates the SRT; the mode is the material's,
    so a donor-authored Maya material composes correctly — no scroll/anchor bug.
  - **ITEMMDL PATH — SCOPED, not yet done.** `d_ww_itemmdl_pc` is a specialized
    subsystem (cached/shared `J3DModelData`, per-item bake-once draw like
    `dWwItemmdl_drawWwBowModel`); the auto-bind needs an owned btk player keyed to
    the cached model + play/entry in its bespoke draw. Follow-up (lower priority —
    the ext-actor path is the broad win). Scene models = the "second" tier, still.
  - **NOTE on the flowers:** `yaflw00.bdl` carries **numbered** btks
    (`yaflw00_01/_02.btk`), not a sibling `yaflw00.btk`, so the auto-binder does
    NOT cover them — the white/pink flower is a separate color-frame case (the
    "more work with the flowers"), not this generic socket.
- **BRIDGE (on return):** confirm which models currently ship baked motion (the
  retirement worklist) + Hermite-evaluated expected-SRT CSVs if key tables
  aren't enough for the differ.
- **FOUNDRY:** lands the `DUSK_BTK_TAP` line when the socket exists (or earlier
  on the §128 model1 wiring to baseline it); runs the verdicts.
