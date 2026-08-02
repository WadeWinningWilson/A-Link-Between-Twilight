# Native Rooms Campaign — bake the connective tissue before quests build on the bridge (Foundry §267)

**Greenlit (user, 2026-07-30):** quest one (awake) must not be built on the ini/CSV/region-
trigger bridge. Bake donor room-connective data into the already-native stage shells; retire
the ext transport/population machinery incrementally. Informed by History's §266-era interior
session: the three unfixed interior bugs are SYMPTOMS of the missing data below.

## The census (the campaign's foundation — measured, not assumed)

| chunk | donor LinkRM Room0 | mod R00_00 | donor sea Room44 | mod R44_00 | what its absence does |
|---|---|---|---|---|---|
| `PLYR` (spawn points) | **5** | 1 | **24** | 1 | **respawn-behind-door → abyss** (respawn targets a spawn id that doesn't exist); every arrival lands on the one authored point |
| `SCLS` (exits) | **1** | 0 | **10** | 0 | no native exits → ini region-trigger transport → **door-plays-but-Link-stays-outside** class |
| `TGDR` (door actors) | 0 | 0 | **5** | 0 | ext doors module instead of native door actors |
| `RCAM`/`RARO` (room cameras) | 1/1 | 0/0 | 5/5 | 5/5 | interior door/camera behavior missing (exterior already baked!) |
| `RPAT`/`RPPN` (paths) | 1/2 | 0 | **40/296** | 0 | **NO villager can walk a donor path** — dNpc_PathRun has no data; blocks History's villager AI at the first path-following NPC |
| `SOND` (sound env) | 1 | 0 | 2 | 0 | room sound environment silent |
| `ACTR`/layers + `SCOB` | 13+4 | 0 | 187 across Default+ACT0-b, 59+ SCOB | 0 | population is CSV-side (known); LAYER semantics unavailable natively — the questline blocker |
| `EVNT` (stage dzs) | 5 | merged ✅ (§265 via merge_event.py) | — | — | the one chunk family already handled |

Stage-side (STG_00.dzs) is already converted (Col0/Env0/PAL0/VRB0 = the lighting pipeline's
TP-format output) — the stage shells are healthy; the ROOM arcs are near-empty husks.

## Format law (the §267 receipt — why this is a TRANSLATOR, not a copier)

`stage_scls_info_class`: donor = 0xC bytes `{stage[8], start, room@0x9, wipe@0xA, pad}`;
receiver = `{mStage[8], mStart, mRoom@0x9, 0xA, 0xB, mWipe@0xC}` — **wipe MOVED, entry GREW.**
A raw chunk copy corrupts silently. Every baked chunk type needs its own field map, receipted
against both `d_stage.h` headers (same method as this SCLS pair). The event container was the
lucky exception (§149: byte-identical structs); assume nothing else is.

## Tool: `bake_room_chunks.py` (merge_event.py's sibling)

- Donor-verbatim entries, per-chunk field translation (receipts in-tool per §267 format law).
- **Wave 1 (quest-critical): `PLYR` + `SCLS` + `TGDR`** — kills the respawn-abyss and
  door-strand class natively; acceptance = warp/door/respawn arrivals all land on donor spawn
  points, exits work without region triggers.
- **Wave 2 (villager-critical): `RPAT`/`RPPN` + `RCAM`/`RARO` + `SOND`.**
- **Wave 3 (questline): layer-aware `ACTR`/`SCOB` with the registered-proc filter** (bake only
  procs that exist natively — l_objectName→fpcNm map from the rosters; the rest stay CSV until
  ported; idempotent re-runs shrink the CSV as actors land). Acceptance instrument = the
  layer-attribution tap (built alongside).
- Dry-run default; `--write` only after History's in-flight interior fixes land (no arc
  collisions); every write gets the standard `.pre<N>-bak`.
- DN-1 fence: this campaign is DATA chunks (spawns/exits/paths/cameras) — BG/collision
  registration is NOT touched. (History's ladder fix + the lava-sink/BG-attribute cluster stay
  in the Engine lane.)

## Sequencing

1. History's interior pass lands (in flight — ladder clear + mount-trigger disable already in).
2. Foundry builds bake tool Wave 1 + runs dry-run receipts (can start NOW, no arc writes).
3. Wave-1 write + History retests the interior (their Grandma testing needs it anyway).
4. Waves 2–3 + layer tap; then quest authoring proceeds on native ground.

## Note for History's CURRENT session (ferry with this doc)

The census explains two of your three untouched bugs without any code archaeology:
respawn-abyss = missing PLYR entries; door-strand = missing SCLS/PLYR pair (the ext transport
has no native target to hand Link to). The lava-sink-on-furniture is the one true
collision-attribute issue (BG poly attributes — Engine lane, DN-1). Your warp-arrival ladder
clear remains right regardless — arrival-state clears and native spawn data are complementary.
