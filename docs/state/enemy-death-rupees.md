# Enemy Death Rupees — live state

| Field | Value |
|-------|--------|
| **status** | Ending Blow + wolf-bite / custom-death grants landed; build green |
| **setting** | `game.enemyDeathRupees` (ALBW Settings) |
| **owner_impl** | Cursor |
| **owner_review** | — |
| **next** | Playtest matrix below; then audit any remaining custom deaths if playtester finds gaps |
| **do_not** | Suppress vanilla ground drops; grant bosses via field kill table (use fight-victory path); skip hooks without `#if TARGET_PC` |
| **updated** | 2026-07-04 |
| **detail** | [albw-port.md](../albw-port.md) (overview + file map) |

---

## Current design (short)

- **Main path:** `dAlbwEnemyRupees_tryKillAfterDamage()` in `cc_at_check` when HP hits ≤0 with nonzero attack power.
- **Ending Blow:** central grant in `procCutDownLandInit()` after `onCutDownHitFlg()` (`d_a_alink_cut.inc`) — covers all downable enemies; idempotent with per-actor hooks.
- **Manual path:** `dAlbwEnemyRupees_onEnemyKill()` on custom deaths (wolf bite, throw kill, Amo delete, etc.).
- **Idempotent** per actor ID via `wasKillGranted()`.

## Key files

| Area | Path |
|------|------|
| Module | `src/d/d_albw_enemy_rupee.cpp`, `include/d/d_albw_enemy_rupee.h` |
| `cc_at_check` | `src/d/d_cc_uty.cpp` |
| Ending Blow | `src/d/actor/d_a_alink_cut.inc` (`procCutDownLandInit`) |
| Twilight Kargarok wolf bite | `src/d/actor/d_a_e_yc.cpp` |
| Other wolf / custom hooks | `d_a_e_s1`, `e_po`, `e_dn`, `e_mf`, `e_ba`, `e_gi`, `e_ms`, `e_yk`, `e_yg`, `e_ai`, `e_oc` (slingshot), plus earlier `e_db` / `e_st` / … |

## Playtest matrix (next)

1. Ending Blow Bokoblin / field Kargarok → +5 / +15 with setting **On**
2. Wolf hang-bite Twilit Carrier Kargarok (`E_YC`) → +15
3. Normal sword/arrow kill field Kargarok → still +15
4. Wolf throw Keese (`E_BA`) → +1
5. Setting **Off** → no wallet credit from these paths

## Known gaps / watch

- Bosses / mid-bosses: fight-victory grants, not field kill table.
- Further custom-death actors may still bypass `cc_at_check` — fix with `onEnemyKill` at lethal site, or extend central patterns when found.
