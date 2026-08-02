# BDL/BMD parse-timing (DN-3)

The single most expensive crash class in the demo work: **when** a model buffer is parsed, not whether.

## ① What the donor does / the port rule

WW/TP J3D model data is **pointer-fixed after parse**: parsing rewrites internal offsets to absolute
pointers into the arc's buffer. Consequences that are law here:

- **A fixed buffer must never be re-parsed.** Parsing an already-fixed buffer corrupts it.
- **An arc must not be freed while its parsed data is still cached.** The cached pointers dangle.

## ② Traps (DN-3 — HARD STOP)

- **TRAP — parsing BDL models at arc-mount time.** The §180 attempt added `'BDL '/'BDLL'/…` resType
  cases so models parsed when the arc mounted. But ExtNpcMount already parses demo models at
  **consume-time**, so this caused a **double-parse** → the Outset black-screen regression. **Never
  parse BDL/BMD at mount; parse at consume-time only.** See [../../DO-NOT.md](../../DO-NOT.md) DN-3.
- **TRAP — `fuku.bdl` is actually `J3D2bmd3`.** The Hero's Clothes demo prop crashed in
  `mDoExt_bckAnmRemove` because `fuku.bdl` was left **unparsed** (the missing BDL parsers) — and its
  magic is BMD, not BDL. The crash was only found by **symbolication** (see
  [../debugging-methods/symbolication.md](../debugging-methods/symbolication.md)), after several wrong
  hypotheses (create-order, resource ids, NULL guards).
- **TRAP — freeing an arc while parsed data is cached.** Caused the sumo BMT crash and room-lane mesh
  corruption elsewhere. Audit any session-lived cache when adding a new lane.

## ③ Correct method — the consume-time resolver

Resolve + parse the demo model **when it is consumed**, not when the arc mounts:
`dExtNpcMount_acquireDemoModel` (global scope in `src/d/d_ext_npc_mount.cpp`) →
`acquireBgModel(arc, "demo_shape_%u", res)`, with the model cache purged per-arc by
`purgeModelCacheForArc` (**erase-only** — the arc owns the buffer, so erase the cache entry, never free
the buffer). SCOB scale byte = `scale/10` (byte 10 → float 1.0).

## ④ Imperfect / watch

- The consume-time resolver is specific to the demo-prop path; other model lanes have their own timing —
  do not generalize without checking.
- Any new session-lived model cache must be audited against the "never free an arc while parsed data is
  cached" rule before it ships.
