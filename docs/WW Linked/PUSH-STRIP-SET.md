# Push strip set + gate (containment, §113/§120)

**Purpose:** the never-push covenant boundary after the dusk-API merge landed a push-capable mod
system next to WW content. Tree is **push-safe by PROCEDURE** — nothing here auto-pushes; the gate
below is the only path to a remote.

## PUSH GATE (mandatory sequence — no push without all four)
1. **rebuild** the merged exe
2. **M6 greplist CLEAN** — `ww_bridge gate <exe>` returns GATE CLEAN (not INCONCLUSIVE)
3. **Tier-3 empty** — no un-adjudicated WW markers (the §43 class-2 check, when built)
4. **user go** — explicit

Push is currently GATED regardless: the `Ivan` literal (`d_ww_itemmdl_pc.cpp:2007`) is still present
(deprioritized per user 2026-07-25 — "hit the bottom before the push comes").

## STRIP SET — covenant-risk surfaces (verify absent/clean before any push)
- **Mod folder** `%AppData%\...\model_replacements\WW-Crew-Restoration\` — physically outside the
  repo; cannot be pushed; local-only git repo of its own (no remote, ever). Donor bytes.
- **WW receiver source in-repo** (~25 files: `d_ext_*`, `src/d/ext_seq/*`, WW hooks in
  `custom_assets.cpp`) — fork-local; carries no donor bytes (verified §67 receiver-agnostic) but is
  WW-shaped. Stays fork-local until the receiver-as-code-mod extraction; not for upstream push.
- **Parked surfaces** — audio-shadow, ext-seq, bridge tooling — local-only.
- **The `Ivan` literal** — must be fixed before push (it is the one confirmed exe-level covenant
  marker; gate catches it if forgotten).
- **Temporary instrumentation (added §177 sweep, 2026-07-27)** — strip before push, none carry
  donor bytes: `include/dusk/fps_probe.h` + Ferry T buckets (m_Do_main/game_clock hooks) ·
  `run_fps_bisect_{palette,waves,btk,wavedraw}.bat` (repo root) · `DUSK_WW_FPS_BISECT` env branches
  · probe log lines: `[WwWind]` which-bank + W-LINE ARM/DISARM (keep the breach LOGIC, strip the
  chatter), `§161 flag-only` (keep — self-verifying acceptance), `§62 cutFx amb=` (strip after
  grass-VFX accepted), `§134 windProbe` (change-triggered after Ferry P #3).
- **Tracked game-derived binaries (flag for push review, §177):** `tools/font_extract/`
  (fontres.arc, .bfn, atlases — TP-side, legality hygiene not №31) · `tools/demo_cut_content/out/`
  tracked text reports are fine, packs/ correctly gitignored.

## Standing
- The **exe greplist gate (M6) is the real backstop** — it scans the built binary regardless of
  which source produced it, so a merge or a stray literal is caught at the exe.
- This file is the documented boundary; the gate is the enforcement. Housing owns both.
- Related: bus §67 (receiver-agnostic proof), §102 (Ivan), §113 (merge containment), §120.
