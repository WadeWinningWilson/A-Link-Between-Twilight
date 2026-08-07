# Declared WW→TP crossings — the register

**Owner: Housing Security** (ferried by HousingTemp, 2026-08-06).

## The doctrine this implements

> **"WW should never reach TP unless specifically stated (like in the case of WW
> Link transformation)"** — user, 2026-08-06.

Read as HousingTemp recorded it, and that reading is adopted here:

1. **Default is deny.** WW code reaching TP is a breach unless gated.
2. **Intentional crossings are allowed but must be DECLARED.** A crossing that is
   neither gated nor declared is the defect.

So the remedy at any site is *gate it* or *declare it*. This file is the second
half — without it, a sanctioned crossing is indistinguishable from a breach, and
the next audit re-flags it forever.

## What counts as a crossing

WW-owned code executing on behalf of a **receiver-owned** TU. The test is
LINEAGE, not roster membership and not file path:

* a WW-lineage TU calling WW code is **not** a crossing (`d_a_kb.cpp` calling the
  WW lighting path is correct — it is WW's pig);
* a receiver TU calling WW code **is** a crossing, even if the call sits inside a
  declared `KIT-DONOR-HUNK`. The hunk marker records where donor code *is*; it
  does not authorise the crossing.

## Register

| # | site | crossing | status | evidence |
|---|---|---|---|---|
| X-1 | `d_a_mirror.cpp:630` | TP receiver actor → WW lighting write path | **GATED §515** — `dExtWwSave_isWwHostStage`; TP's `g_env_light.setLightTevColorType` restored as default | donor has no `d_a_mirror.cpp`; file dates to the 2024 TP decomp history |
| X-2 | `d_a_swhit0.cpp:395-396` | TP receiver actor → WW lighting pair | **GATED §515** — TP's `settingTevStruct(16)` + `setLightTevColorType` restored as default | file dates to 2021-03-28 (`dol2asm`); `mixed` lineage = TP's file with donor spans |
| X-3 | `d_ext_npc_doors.cpp:1658` | WW cut vocabulary (`kWwLinkCuts`) → TP's Link (`Alink`) actor | **DECLARED — SANCTIONED** | receiver-authored host plumbing; the file's §379a note states the intent: WW event data carries a 'Link' staff and TP Link performs the cuts with its NATIVE acting |

### Not crossings — recorded so they are not re-flagged

| site | why not |
|---|---|
| `d_a_kb.cpp:542,556` | WW's pig. 92.1% donor text, donor owns `d/actor/d_a_kb.cpp`, entire dusklight history is WW commits. Was a PROVENANCE gap (no banner), fixed §515 — not a crossing. |
| `d_door.cpp:703,954` | WW door layer, `native-port`. Reached ONLY by `d_a_knob00.h` (WW's knob doors). TP's own door actors are separate files (`d_a_door_knob00.cpp`, `d_a_door_boss.cpp`, …). Contained. |
| the other 15 WW-lighting callers | all `native-port`/`mixed` WW-lineage TUs on the roster — WW code in WW files. |

## OPEN — "WW Link transformation" is NOT in this register, and should be

The doctrine names it as the precedent for a sanctioned crossing. **It could not
be located as implemented code.** A sweep for `WW_LINK` / `WwLink` / `wwLink` /
"toon link" / WW-link-transform across `src/` and `include/` returns exactly one
hit — `kWwLinkCuts` in `d_ext_npc_doors.cpp`, which is X-3 above (WW cut names
driving TP Link's acting) and is a different thing from *transforming* Link into
WW Link.

HousingTemp assumed it "presumably lives in code somewhere with nothing marking
it as a sanctioned crossing". On this evidence that assumption does not hold:
either it is a planned feature not yet built, or it exists under a name this
sweep did not anticipate. **User to confirm which** — recorded as UNKNOWN rather
than silently omitted, because a register that quietly drops the one precedent
the doctrine names is worse than no register.

## How to add an entry

A crossing enters here only with: the site (file:line), what crosses, the gate or
the declaration, and the EVIDENCE that settles the lineage of the receiving TU —
add date, donor-object presence, and measured donor-text share where relevant.
"Declared" without evidence is how a breach becomes permanent by assertion.
