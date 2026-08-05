# Foundry intake ledger — external resources (2026-07-28 sweep)

> Foundry's evaluation queue for outside-the-tree methods/tools. Status enum:
> ADOPT-NOW · EVALUATE · REFERENCE · REJECTED. Housing may audit any adopted
> instrument. Context for this sweep: Engine + Bridge (Cursor) usage-gated —
> priority went to tools a Claude lane can operate directly (Python, offline).

## Tier 1 — ADOPTED 2026-07-28 (user-ratified; pilot PASSED)

**Runtime ruling:** gclib/wwrando run in the pinned **Python 3.12 venv at
`D:\XXXXXXX\foundry-py312`** (uv-managed; invoke via
`D:\XXXXXXX\foundry-py312\Scripts\python.exe`). gclib is **incompatible with
Python 3.14** (PEP 649 lazy annotations break `bunfoe`/`jpa`/`j3d` in ways that can
silently drop fields); a hand-patched 3.14 install was tried, deemed untrustworthy for
a verdict-producing instrument, and **uninstalled** — do not reinstall gclib under 3.14.

**Proof-of-worth pilot (PASSED 8/8):** `tools/foundry/jpc_crosscheck.py` regenerated
Bridge's §192 facts from donor bytes, independently: common.jpc = 193 particles
incl. 0x0031 + 0x03DA/0x03DB/0x03DC; Pscene011.jpc = exactly {0x8315, 0x833D};
0x89D6/0x89D7 absent (TP-lineage confirmation). Offline JPC censusing is now a
Claude-lane capability.

| Resource | What it gives us | Notes |
|---|---|---|
| **gclib** (LagoLunatic, MIT, pip) — github.com/LagoLunatic/gclib | Python parsers: GCM, DOL, REL, RARC, Yaz0/Yay0, BTI, J3D (partial), **JPC (partial)**, BMG, BFN | `pip install "gclib @ git+https://github.com/LagoLunatic/gclib.git"`. Offline arc/particle/image decoding WITHOUT Bridge. JPC support = the §191 "decode JPA1 BSP1 colors offline" class of question becomes Foundry-answerable. |
| **wwrando source** (LagoLunatic) — github.com/LagoLunatic/wwrando | `wwlib/` DZx (DZR/DZS) parser, event-list handling, item tables, `tweaks.py` = worked examples of WW data surgery in Python | The randomizer team solved "machine-read every stage/room/actor placement in the game" years ago — this is P6's census engine, ready-made. GZLE01-only (matches us). |
| **zeldaret/tww refresh cadence** — github.com/zeldaret/tww | The decomp is now **72.13% decompiled / 58.73% linked** and moving (per decomp.dev) | `D:\XXXXXXX\WW DP` is a real git clone — **pulled 2026-07-28** 61a3a0a→be8da68 (189 files, ~9k insertions in 9 days). Standing cadence: `git pull --ff-only` weekly-ish; tree was clean. |

**Adoption footprint (2026-07-28):** wwrando shallow-cloned to `D:\XXXXXXX\wwrando`
(wwlib: dzx.py, events.py, dzb.py, charts.py, stage_searcher.py — P6 census engine,
not yet piloted); gclib in the 3.12 venv above; pilot tool `tools/foundry/jpc_crosscheck.py`.

## Tier 2 — EVALUATE (instrument successors; F1 result decides some)

| Resource | What it gives us | Decision hook |
|---|---|---|
| **Felk's Dolphin fork (Python scripting)** — github.com/Felk/dolphin | Embedded Python: `await event.frameadvance()`, memory r/w, savestates, controller input, `--script x.py` CLI. An **mcp-dolphin** wrapper exists (dmang-dev/mcp-dolphin) | Potential DuskTap successor: scripted taps without C++ rebuilds, and programmatic capture-session driving (P5 harness). Cost: separate fork — evaluate side-by-side (portable), don't merge blindly. |
| **Lobsterzelda's lua-support build** — github.com/Lobsterzelda/lobster-zelda-dolphin (branch `lua-support`) | TAS community's claim: "there shouldn't be any desyncs for Wind Waker on this build"; the 4.0-Lua lineage is what WW TASers actually trust | **If F1 fails on our master build**, this is the known cure for reels: record/replay on this build, keep DuskTap build for tap missions from save states. |
| **py-dolphin-memory-engine** (pip) + Dolphin Memory Engine GUI | Hook a RUNNING Dolphin from Python; per-frame RAM polling, no patches | Complements DuskTap (call-site events) with state sampling — NPC pathing/AI state timelines for P6 fact sheets. Works against stock or our build. |

## Tier 3 — REFERENCE (inspection estate + specs)

- **Winditor** (LordNed/Winditor) — GUI map + text + event editor; visual cross-check
  for censuses. **WindEditor**, **Wind_Waker_Event_Editor** (event_list.dat), and
  **WindWakerCollisionEditor** (DZB) by Sage-of-Mirrors — per-domain editors/viewers.
- **TASVideos DTM format doc** (tasvideos.org/EmulatorResources/Dolphin/DTM) — header
  spec for reel-library tooling (from-savestate flag, controller config, etc.).
- **CloudModding TWW wiki** — format documentation (UNVERIFIED this sweep; check on use).

## Intelligence that changes the program (not a tool)

**WW is a known Dolphin movie-desync offender.** TASVideos, across a decade and as
recently as 2025: no published WW TAS exists; master-line builds desync WW movies
("within minutes" era reports; newer builds still desync-prone), and the community
fell back to a 4.0-Lua lineage + Lobsterzelda's build. **P1 consequence (design
correction, ratified into the state file):**

1. Reels are demoted from "guaranteed replay" to **validated-per-reel** — the F1
   record→replay→tap-diff gate is load-bearing, not a formality.
2. **Save states are promoted to the primary re-harvest asset.** States on a pinned
   build have no replay-determinism problem at all. Dense per-beat state coverage +
   short from-state segments (short movies desync far less than long boot-anchored
   ones) beats one long fragile movie.
3. Capture-maximal sessions remain the hedge: with the full tap roster live, the
   first play yields most data even if its movie never syncs again.

## §423 Sky-campaign law for the kits (2026-08-04, Housing Security)
From commit 9c62ae05a0 (§407-§420, user-verified) — encode into actor/staging kits:
1. LIGHTING CONTRACT: every WW actor draw routes dKyWw_settingTevStruct (donor-authored
   TEV type) → dKyWw_setLightTevColorType; never hand-write C0/K0. The §406 canary logs
   any unfed tevstr ("UNFED tevstr" = wiring miss).
2. CUSTOM-ARC MODELS: custom-mounted arcs serve RAW J3D bytes — NEVER feed
   dComIfG_getObjectRes results to mDoExt_J3DModel__create; go through
   dExtNpcMount_acquireModelData (parse-once cache; two symbolicated crashes prove it).
3. NATIVE-ACTOR PATTERN (vrbox precedent): TP same-lineage actor + #if TARGET_PC WW leg
   (PC-append fields, guarded sizeof asserts), donor register law from extraction,
   spawn from a resource-proven site, handshake flag to retire any interim draw.
4. STATUS BIT 1 = "stage has a sky" (daVrbox_Create only) — any future host surface
   that suppresses a TP subsystem must assert the contract it displaced.
5. WIND: hosts arm donor wind = tact-default (1,0,0) + authored FILI level (receipts
   in §416); never invent ambient constants.

## §426 RULING: lineage tags RATIFIED (user, 2026-08-04) + law-1 finding closed
1. The lineage declaration is APPROVED as proposed: one machine-readable line per
   ported TU. Suggested concrete shape (adjust if the lint prefers another spelling,
   but keep it grep-exact, never inferred from prose):
       // KIT-LINEAGE: native-port | bridge-owed[:ledger-ref] | host-plumbing
   Semantics as you specified: native-port = all five laws scored; bridge-owed =
   scored but reported OWED against the ledger entry (standing directive: bridges
   labeled, never silently passed); host-plumbing = donor-draw laws N/A with lineage
   as the stated reason. Missing tag = UNKNOWN. Foundry executes the mass-tagging
   and emits the tag from actor_kit for every new port.
2. GREEN-LIT: wire kit_laws.py as the blocking pre-flight on actor_kit AFTER the
   tags land (so N/A semantics are truthful from the first gated run).
3. Your law-1 finding is CLOSED by Housing (§425, exe 18:05:12): d_a_ext_plank_span
   now routes dKyWw_settingTevStruct(TEV_TYPE_BG0) → dKyWw_setLightTevColorType,
   donor receipt WW d_a_bridge.cpp:1244. Next sweep should report 0 VIOLATION;
   plank_span classifies native-port (declared not-ported list stays on the ledger,
   which does not demote a port to a bridge — imitation machinery does).
4. For your two law-5 UNKNOWNs: the donor wind authority is §416 on the WW tale bus
   (tact-default (1,0,0) + FILI level, Winditor receipt sea/Room44=0→0.3) — cite it.

### §423 correction (Housing, 2026-08-04): law 2 symbol spelling
Foundry's catch confirmed: the exported symbol follows the established
dExtNpcMount_ prefix. Code renamed at source (mount TU + both vrbox TUs);
law text above corrected in place. Lint should key on dExtNpcMount_acquireModelData.
