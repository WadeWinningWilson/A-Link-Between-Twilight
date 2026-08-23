# Drive / FPS oracle — live state

> **Field RelWithDebInfo poison (~33 / ~100–120)?** That is **not** this file.
> Go to **[build-fps.md](build-fps.md)** (`tools\_factory_recover.bat`). This page is the **capped ~144 drive oracle** only.

| Field | Value |
|-------|--------|
| **status** | Tracks B–J complete; mean ~143.8–144 on good sessions; intermittent ~105 cliffs (~17%) on WIP **and** v1.4.1 |
| **owner_impl** | Build-analysis / drive chat |
| **next** | Profiler on hitch frame; use labeled `load_save` drives for regression — not shield-mission FPS |
| **do_not** | Use `DUSK_DRIVE=armogohma_shield` as FPS baseline; leave `DUSK_DRIVE*` set during manual play; commit drive/`local_dev_backup/`; start with `reconfigure_build.bat` for routine checks; revert HUD/FA/parry to “fix” FPS |
| **golden_sha** | `76b715abc6` (user-confirmed 144) |
| **updated** | 2026-08-23 (pointer to build-fps factory restore) |
| **detail** | [performance-handoff.md](../performance-handoff.md) (drive protocol + hitch research) |
| **factory** | [build-fps.md](build-fps.md) — RelWithDebInfo `/O2` poison restore |
| **build_map** | [build-fps-guidelines.md](../build-fps-guidelines.md) |
| **leaning** | [future-performance-leaning.md](../future-performance-leaning.md) |

---

## Ideal multi-agent loop (async — needs a runner)

Drive is **optional** inside an [Interconnected Run](../Interconnected%20Chats/INTERCONNECTED-RUNS.md) — only when the user asks. Deliberation (plans → critique → dual sign-off) happens first; drive validates after execute.

```text
User brief + Start (run-control)
        ↓
Plans → critical responses → unified plan (both APPROVED)
        ↓
Impl agent(s)  →  update docs/state/<feature>.md + build_run.bat
        ↓
(optional) Drive  →  labeled load_save (this file’s protocol)
        ↓
PASS / FAIL written back; user Pause/Stop freezes everyone
```

Agents do **not** wake each other; the **user** (or later script) starts turns. Pause/Stop via [run-control.md](run-control.md).

## When to drive vs manual

| Drives | Manual play |
|--------|-------------|
| Regression after known-good build | “Does it feel like 144?” |
| Repeatable `F_SP121` gate | Saved AppData, no drive env |
| Labeled A/B | Final sign-off |

## Canonical drive (short)

- Overlay drive code lives under `local_dev_backup/` — **not** shipping `src/`  
- Runner example: `local_dev_backup/session/run_load_save_drive.ps1` (or Track G user-natural)  
- Env: `DUSK_DRIVE_USER_CONFIG=1`, `DUSK_DRIVE_SKIP_MENU_GATE=1`, `DUSK_DRIVE=armogohma|hyrule_field`  
- **PASS:** gate_min ≥ 127, play_avg ~140–144 on field after real save load  
- After batch: clear `DUSK_DRIVE*`, restore config, one manual field check, no drive commits  

Full protocol: [performance-handoff.md § Drive session](../performance-handoff.md#addendum-drive-session-protocol-2026-06-25).

## Scenes

- Field: `F_SP121` r0 p0  
- Armogohma observe: `D_MN06A` (hitch research — not shield mission)
