# PLAYTEST CARD — one run, every lane's pending checks (§695 #1)

> Assembled by the integrator before each user run. `log_check.py` reads THIS
> file as its marker registry — the card and the registry are one artifact.
> Line format:  `- CHECK <id> | <lane> | <where / what to do> | MARKER: <literal>`
> Optional:     `| FAIL: <literal>` — if the FAIL literal appears, verdict is
> FAIL even when the MARKER also appears. A check whose MARKER never appears
> is UNTESTED — never green.

> **Marker rule (§697/§698, integrator shift):** a MARKER must be the STATIC part
> of the source format string. Never bake a runtime value (`room=44` is fine —
> it is the value under test; `5 record(s)` is not — a legitimate change of that
> count silently flips the check to UNTESTED, which is indistinguishable from
> "never exercised").

## Current card (seeded 2026-08-09 · markers synced against build 2026-08-11 13:49)

- CHECK seam-r44 | PATH:disc | Foundry | enter Outset host room 44 | MARKER: [WwRoomSeam] 3b: stage=
- CHECK fili-r44 | PATH:disc | Foundry | same visit — FILI translation fires | MARKER: [WwRoomSeam] FILI room 44
- CHECK rcam-recv | PATH:mount | Foundry | same visit — RCAM self-detects receiver-format. CONTRADICTS rcam-donor; see §698 — needs the RCAM owner's ruling, NOT a marker edit | MARKER: record(s) already receiver-format
- CHECK scls-translated | PATH:mount | Foundry | R_DL01 SCLS data translated at room load (LOAD-time fact only) | MARKER: [WwRoomSeam] SCLS[0] stage='
- CHECK! roundtrip-ojhous | PATH:disc | Housing/Engine | ORCA'S HOUSE ROUND TRIP — enter and walk back out. PASS requires the HOST room to reload AFTER the interior (the return leg, proven in run 164618). This is the one REQUIRED interior check: it is keyed to Ojhous because Ojhous is the proven route, and that is a stated limitation, not an oversight | MARKER: 3b: stage='sea' room=44 | AFTER: 3b: stage='Ojhous'
- CHECK entry-linkrm | PATH:disc | History | Grandma's house loads after the host room | MARKER: 3b: stage='LinkRM' | AFTER: 3b: stage='sea' room=44
- CHECK entry-ojhous2 | PATH:disc | History | Sturgeon's (Ojhous2) — room 1 proven in 162549; enter to re-exercise | MARKER: 3b: stage='Ojhous2' | AFTER: 3b: stage='sea' room=44 | EXPECT: UNTESTED
- CHECK entry-onobuta | PATH:disc | History | ROSE'S HOUSE — never room-loaded in any session on record. UNTESTED here IS the finding until the port lands (tale §800) | MARKER: 3b: stage='Onobuta' | EXPECT: UNTESTED
- CHECK entry-omasao | PATH:disc | History | Mesa & Abe's house — never room-loaded; UNTESTED is the finding | MARKER: 3b: stage='Omasao' | EXPECT: UNTESTED
- CHECK entry-pjavdou | PATH:disc | History | Jabun's cavern — never room-loaded; UNTESTED is the finding | MARKER: 3b: stage='Pjavdou' | EXPECT: UNTESTED
- CHECK entry-linkug | PATH:disc | History | LinkUG — never room-loaded; donor DZR has NO actors, so an empty room is the EXPECTED result, not a fault | MARKER: 3b: stage='LinkUG' | EXPECT: UNTESTED
- CHECK rdl02-entry | PATH:mount | Housing/Engine | R_DL02 has NO entry path yet — this line stays UNTESTED until one lands, and that UNTESTED is the finding | MARKER: [WwRoomSeam] 3b: stage='R_DL02' | EXPECT: UNTESTED
- CHECK rcam-donor | PATH:disc | Foundry | donor-stride RCAM translation — card said "none staged"; it FIRES today (5 records, room 44). Count de-baked | MARKER: donor-stride record(s) translated for room 44 | EXPECT: UNTESTED
- CHECK! crash-sweep | all | any run — a crashed session must NOT report green (HT-29, field-proven §711) | MARKER: [WwRoomSeam] | FAIL: DUSKLIGHT CRASHED
- CHECK seam-error-sweep | PATH:any | all | any run | MARKER: [WwRoomSeam] | FAIL: [WwRoomSeam] SCLS: roomNo

> **PATH dimension (row 22, Foundry):**  = served from the donor
> image (stage  naming);  = the staged/mounted package
> (/ naming);  = path-independent. Different paths
> are DIFFERENT TESTS — a disc-path PASS says nothing about mount and vice
> versa. Per row 21 the mount is RETIRING: once the overlay unloads,
> mount-path checks flip to RETIRED (kept for the reference package, never
> counted green). Labels assigned by Foundry from stage-naming evidence —
> INTEGRATOR RE-VERIFY at next marker sync, since which path a marker
> exercises is exactly the ambiguity row 20 measured (153 mount overlays
> shadowing 103 disc files until row 21 lands).

> **Pass-9 lesson (Foundry note):** a marker measures a LOG LINE, not the
> behaviour it stands for. Checks whose intent is a TRANSITION must use
> AFTER: (sequence) — and checks whose intent is an INTERACTION HAPPENING AT
> ALL (door opens, actor responds) cannot be expressed as log greps unless the
> behaviour logs; those need a behavioural probe (duskStateTap class), routed
> as an instrument request, not faked with a nearby marker.
