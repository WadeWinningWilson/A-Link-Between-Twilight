# tale.stb cast-binding map — the Phase-2 forensics (Foundry §269)

**History's ask:** map which JStudio cast objects `tale.stb` waits on and how native
actors should bind in place of the mount demo-doubles.

## The cast, enumerated from the STB itself (2,684 bytes, Demo01.arc)

| JStudio object | type | donor binding (receipts below) | port status |
|---|---|---|---|
| `Ba1` | JACT | live actor by name (`fopAcM_searchFromName`) | ✅ native ba1 registers; found |
| `Link` | JACT | live player by name | ✅ |
| `camera` | JCMR | `mObject->createCamera()` | ✅ (worked in awake) |
| `d_act0` | JACT | **created ON DEMAND**: JSGFindObject's `d_act` branch → `fopAcM_fastCreate` → **DEMO00 puppet** (arg 0) → `setStageLayer` → `appendActor` | ❓ the freeze lives here |
| `d_act2`, `d_act3` | JACT | same, args 2/3 | ❓ |
| `message` / `jstudio`+`control` / `JSND` / `paticle` | JMSG/ctrl/JSND/JPTC | standard control objects | ✅ (JMSG rendered in mount era) |
| `fuku_model` | (string) | the clothes-prop resource name, supplied to a d_act via the STB actor-data channel | follows d_act |

## The donor mechanism (all decompiled source — receipts)

1. **`dDemo_system_c::JSGFindObject`** (donor d_demo.cpp:579): actor objects resolve
   `fopAcM_searchFromName(name)`; **not-found + name starts `d_act` →
   `fopAcM_fastCreate(name, 0)` + `fopAcM_setStageLayer(ac)`** → `appendActor`.
   **The port HAS this branch already** (src/d/d_demo.cpp:1035, §48-probed) — TP
   shipped the same mechanism.
2. **`d_act0..6` → `fpcNm_DEMO00_e` arg N** — donor d_stage.cpp:1196; **the port has
   the same rows** (src/d/d_stage.cpp:1388) and a registered DEMO00 profile + actor
   (`src/d/actor/d_a_demo00.cpp`, TP's twin).
3. **DEMO00's model/anims are NUMERIC-ID lookups into the DEMO ARCHIVE**
   (`getObjectIDRes(getDemoArcName(), mShapeID/mBckID/mBtpID…)` — donor
   d_a_demo00.cpp:169–374). Ids arrive via the STB actor-data channel at runtime.
4. No `d_act` placements exist in donor LinkRM (verified) — creation is always
   on-demand. `awake.stb` has NO d_act cast (verified) — **the tale is the port's
   FIRST exercise of the puppet-creation branch.**

## Data alibis (measured, rule these out)

- `Demo01.arc` staged mod-side, **all donor-named member ids donor-exact** (resid
  audit clean) — the numeric-id data demo00 needs is present and correctly numbered.
- `Ba.arc` likewise clean (§268). The freeze is BEHAVIORAL, not data.

## The discriminator — one repro run, existing log lines

Run the live repro, then grep the dusk log for `§48 JSGFindObject actor='d_act`:

| observation | meaning | fix target |
|---|---|---|
| **(a) no §48 line for d_act0** | the STB's object-open for d_act never reaches JSGFindObject — the port's JStage TCreateObject/actor-object path skips it | STB object-creation path in port d_demo.cpp (compare donor :620-635 factory setup) |
| **(b) §48 'not found' + 汎用くん生成失敗 (fastCreate NULL)** | DEMO00 create fails — prime suspect: TP demo00's create/heap expects TP conditions, or its numeric getIDRes into the demo arc fails (SAME ROOT as ferry blocker #1 — engine numeric path on mod arcs) | port d_a_demo00 create; if numeric lookups NULL, the §268 getIDRes probe localizes it |
| **(c) §48 FOUND/created but §52 still actor=NONE** | append/read-back mismatch — object appended to a different list than §52 reads | port appendActor/read-back plumbing |

## Fix-path B (native fallback, pre-approved shape)

If (b) shows TP's demo00 diverging behaviorally from what the WW STB drives: the donor
`d_a_demo00.cpp` is **fully decompiled (864 lines, 0 Nonmatching)** — a direct port of
the WW puppet via the standard kit is one actor-port of the easiest class (no cutscene
tier of its own, no placement, no event logic), and the `d_act*` OBJNAME rows can point
at it instead of TP's twin. That is the donor-verbatim end-state either way.

**Lane split:** History → run repro, read the discriminator, fix (a)/(c) plumbing or
integrate fix-path B; Foundry → on a (b) numeric-lookup result, the §268 getIDRes probe
is the next instrument; on a fix-path-B order, the demo00 kit generates in one command.
