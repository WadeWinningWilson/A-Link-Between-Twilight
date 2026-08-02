# DuskTap capture library — INDEX (raw traces live OUTSIDE the repo)

**Raw traces are housed at `D:\Dolpheen Plz\captures\traces\`** (moved out of the repo 2026-07-30,
bus §204/§205). They are bulk donor-derived data — register dumps, world coordinates, donor
placement names — so they live with the donor material, not in the shipped tree. Derived
*analysis* stays in docs; this file is the pointer.

Full session logs: `D:\Dolpheen Plz\captures\dolphin-capture-*.log`.

| trace | size | what it is | consumed by |
|---|---|---|---|
| `dolphin-captures-tale-trace-20260728.txt` | 1.2MB | Grandma tale scene: JStudio actor/camera/message commands + dataID payloads, timestamped | History — W2 diff-target |
| `dolphin-captures-awake-aryll-trace-20260728.txt` | 19KB | Aryll/awake window (51 actor cmds) | History — W3 binding ground truth |
| `dolphin-captures-storybook-trace-20260728.txt` | 29KB | opening storybook window (92 actor cmds) | History |
| `dolphin-captures-roster-20260728.txt` | 85KB | 897 placement spawns, 132 unique donor names + params + positions | History (ambient census), Housing (build list) |
| `dolphin-captures-buildlist-placements-20260728.txt` | 5KB | the §199 four targets' donor sites (ikada_h, NpcSo, woodb/woodbx, flower/tree packets) | whoever ports them |
| `dolphin-captures-bgm-notes-20260728.txt` | 306KB | 8,045 donor note events (intro medley → Outset theme) | audio board unpark / Foundry P2 differ |

**Reproducing:** launch `D:\Dolpheen Plz\Local Dolphin\build\Binaries\Dolphin.exe` (the tapped
build — boot log must read `DuskTap: N tap(s) loaded`), taps configured in
`…\Binaries\User\Config\DuskTap.ini`. See bus §189-§199 for tap addresses and their provenance.
