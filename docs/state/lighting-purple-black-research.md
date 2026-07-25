# Restored-area purple/black lighting — research findings (data forensics)

**Pass type:** research (user-requested, 2026-07-24). Fix staged, not applied.

## ROOT CAUSE (byte-proven): botched Ba1_Get_Itm merge corrupted stage.dzs PAL0

The same 23:02 merge that dropped `event_list.dat` (№267) ALSO corrupted the
**PAL0 palette chunk** inside `F_DL01/STG_00.arc::stage.dzs`. №267 restored only
`event_list`; `stage.dzs` was never checked and stayed mangled.

**Evidence (F_DL01/STG_00.arc::stage.dzs, PAL0 @0xee0, 18×52B):**
- Backup PAL0 entries = uniform neutral runs: `998e99 998e99…`, `f7eee5 f7eee5…`,
  `ffffff ffffff…`.
- Live PAL0 = those runs replaced with shifted garbage incl. `414e97`/`0963e0`
  (blue-**purple**), `000000` (**black**), `cfd3df`, `7f8b98`.
- Lineage: `pre-ba1getitm-bak`, `pre165-bak`, `pre152-bak` ALL share the good
  PAL0 md5 `84e512ce`; only the live post-merge arc has `1d89baba`. All other
  arc members (colordata_tbl, pol_effcol/efftbl, Env0/Col0/VRB0, RTBL…) are
  byte-identical live-vs-backup — the damage is isolated to PAL0.

## Why purple AND black, and why it shifts at Grandma's door — ONE cause

The corrupt PAL0 holds BOTH blue-purple and pure-black entries. The env-light
chain (`Env0→Col0→PAL0→VRB0`, `d_ext_npc_mount.cpp:2503` `wwSkyFeedEnvFromStageVrbox`,
indexed by `UseCol`/room/time/weather) resolves a DIFFERENT palette index per
room and transition. So moving between rooms / through the door reads a
different corrupt entry — purple at one index, black at another. No separate
state bug required. `settingTevStruct(0)` then lands that env color on Link via
`dExtWw_applyPlayerDonorLook` (`d_ext_npc_mount.cpp:5937`, the `wwPlayerDonorLook`
setting) — which is why the setting toggle changes the shade.

## NOT part of this regression (pre-existing, benign)

The "menu renders the setting halfway inert (one effect off)" is the documented
coupling in `wwPlayerDonorLook`: `settingTevStruct(0)` (full tevStr repopulate)
vs the ambient pin to neutral 90 desync in menus (comment `d_ext_npc_mount.cpp:5947`).
It has always happened and is independent of the PAL0 corruption.

## Also confirmed clean

- The alink/env lighting CODE is not the cause — the palette DATA is corrupt.
- The `№133/№144` env-rebind logic (`wwSkyRebindStageLighting` :2465) is a real
  fragility (UseCol=room44 OOB, dm-default latch) but is NOT the recent
  regression — the recent break is the PAL0 bytes.

## FIX (data-only, no build, no code) — STAGED

The good arc is intact on disk; every member except `stage.dzs` already matches
the backup, and the backup's `event_list` is good too. One copy restores both:
```
cp F_DL01/STG_00.arc.pre-ba1getitm-bak F_DL01/STG_00.arc
```
Loses nothing real: the merge that "added" Ba1_Get_Itm actually failed (dropped
event_list + corrupted PAL0); that merge must be re-done properly regardless
(queued). After restore: verify PAL0 md5 == `84e512ce` and event_list present
(member listing — №267 law), wipe caches, retest lighting in restored areas.

## STANDING LAW reinforced (№267)

A botched arc write can damage MORE THAN ONE member. When restoring one, diff
ALL members against a known-good backup before declaring the arc fixed. №267
restored event_list and stopped; PAL0 rode broken for days. Full member-hash
diff is the verification, not the count.
