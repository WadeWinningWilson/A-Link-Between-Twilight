# Tale/awake STB scene scripts — distilled (jstudio_stb.py, 2026-07-27)

Exact, frame-accurate decodes via Housing's `tools/ww_crew_restoration_skeleton/jstudio_stb.py` (bus
§172), a faithful mirror of the decomp `JSystem/JStudio`. Raw JSON dumps live in scratch; this is the
distilled cast + spawns + SHAPE(clothes) + fades + camera. **All values are game data, not estimates.**
(`get_shield` has no STB — it's an event-pack, StartCode 201.)

## `tale.stb` (TALE_DEMO) — the give · 11 blocks · ends @710
| actor | spawn (LinkRM) | rotY | notes |
|---|---|---|---|
| `Link` | `(-341, 375, 250)` | 180° | SHAPE `1→1→1→0→0` @{0,100,150,**385**,618}; ANIM 4,2,512… |
| `Ba1` (Grandma) | `(-341, 375, -3)` | 0° | ANIM 65552→65563 (present/talk bcks) |
| `d_act0` | `(-341, 375, -3)` | 0° | **SHAPE 65548 (`fuku_model` = the clothes prop)**; ends @255 |
| `d_act3` | — | — | **FADE-OUT 20f (black) @frame 680** (`id 9 [dir0,0x14]`) |
| `d_act2` | — | — | ends @676 |
| `camera` | — | — | 31 events, FVR-indexed to 8 FVB curves (eye/target/FOV, ranges 3.33/3.5s) |

## `tale_2.stb` (TALE_DEMO2) — Link wearing · 9 blocks · ends @710
| actor | spawn | rotY | notes |
|---|---|---|---|
| `Link` | `(-341, 375, 250)` | 180° | **SHAPE 1 (hero) held** — Link is now in the clothes |
| `Ba1` | `(-341, 375, -3)` | 0° | same present/react ANIMs |
| `d_act3` | — | — | **FADE-OUT 20f (black) @frame 680** |
| `d_act2` | — | — | ends @676 |
| `camera` | — | — | 31 events / 8 FVB curves |

**Fade fact (authoritative):** both tale demos end with `FADE-OUT 20f (black) @680`, fired by the
`d_act3` demo00 double — the demo *exit* (tale→tale_2 covers the outfit change; tale_2→gameplay).
Rides the demo00 execution. The **commencement** fade (gameplay→cutscene, pre-`tale.stb`) is NOT in
these STBs — `tale.stb` opens un-faded @0 — it's `d_a_npc_ba1::event_actionInit` (STUB); 20f is the
near-certain standard since every STB fade = 20f.

**SHAPE/clothes note:** SHAPE = clothes state (`d_a_player_main` getShapeId: 0=casual, 1=hero). tale
Link toggles at **@385**; tale_2 holds 1. The exact give-direction (which toggle IS the "put on")
is Engine's to reconcile against the player texture path — but the *timing* is now exact.

## `awake.stb` (the opening, F_DL01) — W3 camera reference · 7 blocks · ends @2659
| object | detail |
|---|---|
| `camera` | **101 events → 179 FVB `list_parameter` curves** — the full eye/target/FOV path, ranges 0.67…8.33s (4.667s = 140f, matches its own WAIT 140) |
| `Link` | spawn `(24601, 1654, -6388)` rotY -60° (Outset lookout); SHAPE 1; ANIM over 0–1835+ |
| `Ls1` (Aryll) | SHAPE/ANIM over 0–1149; ends @2637 — **the W3 binding target** |

This is the complete camera + cast reference for W3 (opening-cutscene binding) in one decode.

## Usage
`python jstudio_stb.py <file.stb|.arc> [member] [--json[=out.json]]` — raw STB or Yaz0'd RARC.
Known limits (Housing): camera vv6–vv9 (roll/fov/near/far family) unlabeled pending one decomp read;
binary dataID keys print as hex.
