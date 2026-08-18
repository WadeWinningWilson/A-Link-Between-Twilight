# PR CORRECTION PACK — the four-version claim is a three-version fact

> ## ✅ STATUS 2026-08-17 — THE CODE FIX IS PUSHED; THE PR TEXT IS NOT
>
> **DONE, on the user's explicit go:** `MatchingFor("GZLJ01", "GZLE01", "GZLP01")`
> pushed to all three PR branches — `728923e0` (msdan2) · `c7eee79d` (msdan_sub2) ·
> `cae589fe` (hami2). Each commit message carries its own expected-vs-actual hash pair.
> Built in isolated worktrees so Decoder's live `main` was never touched.
>
> **⚠️ NOTE FOR THE USER:** those three commits carry a
> `Co-Authored-By: Claude Fable 5` trailer. That is honest attribution, but decomp
> projects sometimes have AI-contribution policies — **if you would rather it not be
> there, say so and I will amend and force-push all three.** Flagging rather than
> deciding for you.
>
> **STILL OPEN — needs `gh` or a token, which this machine does not have:** the three
> PR *descriptions* still read *"Matches … for all four versions."* `git push` cannot
> edit a description. Replacement text is below, ready to paste.
>
> **⚠️ CONTESTED:** Decoder's `98a75858` returned `main`'s three labels to bare
> `Matching`, citing a 19:52 D44J01 rebuild. That rebuild ran **inside** the 19:42
> exclusion window, and the TU's `.o` is unchanged throughout — so those RELs are
> passthrough originals, not compiled output. **`98a75858` is on `main` only and did
> NOT reach the PR branches**, which hold the conservative label. Falsifiable rebuild
> test filed to Decoder; see the CALLS row.

**Written 2026-08-17 by History/Bridge on the user's direct order, after independently
re-hashing every REL in all four manifests.** Sibling doc: [`decode-drafts/PR-STAGING.md`](decode-drafts/PR-STAGING.md),
which staged these PRs correctly and conservatively — the over-claim entered *after* it.

---

## WHAT "9/12" COUNTS

**3 translation units × 4 disc versions = 12 TU-version combinations.**

| | GZLE01 (US) | GZLJ01 (Japan) | GZLP01 (PAL/EU) | D44J01 (JP kiosk demo) |
|---|---|---|---|---|
| `d_a_obj_msdan2` | ✅ MATCH | ✅ MATCH | ✅ MATCH | ❌ **MISMATCH** |
| `d_a_obj_msdan_sub2` | ✅ MATCH | ✅ MATCH | ✅ MATCH | ❌ **MISMATCH** |
| `d_a_obj_hami2` | ✅ MATCH | ✅ MATCH | ✅ MATCH | ❌ **MISMATCH** |

**9 of those 12 cells are verified byte-identical. 3 are not.** Every ✅ above was
re-measured by hashing the built REL and comparing to that version's own unmodified
`build.sha1` — not by reading the manifest back.

### The three failing cells, expected vs actual

| TU | expected (`config/D44J01/build.sha1`) | actual (built REL) |
|---|---|---|
| `d_a_obj_msdan2` | `c343a7d7dad21e20e7d2f0fe196b76404fde8417` | `8a62d937efa23c72402e3c8ee32216de27d719a9` |
| `d_a_obj_msdan_sub2` | `1dc316e39d6c37b6e7453ab8b2da2b3c9031707f` | `57181aebb0e298e665daffe126fe190fbd0869f4` |
| `d_a_obj_hami2` | `cf947424afb7858141e39a6263aef29e94c2f6b9` | `c7b0876ceafa4620f26b6c97caa8a6449d3d1d2e` |

**The three values certified as "MATCH" in the twelve-of-twelve row are the EXPECTED
column, character for character.** The manifest was read back and reported as a
measurement of the artifact.

### Ruled out before filing

- **Not staleness** — the D44J01 RELs were built 08-17 08:46, *one minute after* the
  GZLP01 builds that genuinely matched; `hami2`'s source has not moved since 08-16 22:39.
- **Not a tampered oracle** — `git log origin/main..HEAD -- config/` is empty. All four
  manifests are upstream's own.
- **Not a code fault** — D44J01 diverging is the ordinary case. `configure.py` already
  carries **88 `ActorRel(MatchingFor("GZLJ01", "GZLE01", "GZLP01"), …)` lines** (141
  instances of that version triple across all object kinds). This is the house convention
  for *matches retail, differs on the kiosk demo*.
- **Confirmed independently by upstream CI** — `decomp-dev` reports GZLP01 / GZLJ01 /
  GZLE01 all matched with +0.03% on each PR, and **D44J01: "Report not found. Did the
  build succeed?"**

---

## THE CORRECTION — `configure.py`, one line per TU

Replace the bare `Matching` with the measured triple. Formatting copied from the 88
existing lines (note the aligned four-space run before the name):

```python
    ActorRel(MatchingFor("GZLJ01", "GZLE01", "GZLP01"),    "d_a_obj_msdan2"),
    ActorRel(MatchingFor("GZLJ01", "GZLE01", "GZLP01"),    "d_a_obj_msdan_sub2"),
    ActorRel(MatchingFor("GZLJ01", "GZLE01", "GZLP01"),    "d_a_obj_hami2"),
```

Current lines to replace — `configure.py:1777`, `:1779`, `:1755` respectively.

---

## REPLACEMENT PR BODIES

Each PR's current body opens *"Matches `<tu>` for all four versions"* and claims each
was *"verified locally against its own (unmodified) manifest."* That is true for three
versions and false for the fourth. Replacements below state exactly the evidence held.

### PR #1173 — `d_a_obj_msdan2 OK`

```
Matches `d_a_obj_msdan2` for the three retail versions: GZLE01, GZLJ01 and GZLP01.

Decompiled from the split assembly. The REL builds SHA1-identical to
`config/<version>/build.sha1` for those three, each verified locally against its own
unmodified manifest. Status is set to
`MatchingFor("GZLJ01", "GZLE01", "GZLP01")` to match that evidence exactly.

D44J01 (the kiosk demo) does NOT match and is deliberately excluded — the built REL
hashes `8a62d937...` against the manifest's `c343a7d7...`. This mirrors the convention
already used by 88 other ActorRel entries in configure.py. CI's D44J01 report on this
PR is absent for the same reason.

Correcting this PR's original description, which claimed all four versions: the D44J01
column was asserted from the manifest's expected hashes rather than measured against
the built RELs. Three versions are measured; the fourth is now correctly excluded
rather than claimed.

Member names follow the `field_0xNNN` placeholder convention for unknowns; retained
names mirror the already-merged family siblings.
```

### PR #1174 — `d_a_obj_msdan_sub2 OK`

```
Matches `d_a_obj_msdan_sub2` for the three retail versions: GZLE01, GZLJ01 and GZLP01.

Decompiled from the split assembly. The REL builds SHA1-identical to
`config/<version>/build.sha1` for those three, each verified locally against its own
unmodified manifest. Status is set to
`MatchingFor("GZLJ01", "GZLE01", "GZLP01")` to match that evidence exactly.

D44J01 (the kiosk demo) does NOT match and is deliberately excluded — the built REL
hashes `57181aeb...` against the manifest's `1dc316e3...`. This mirrors the convention
already used by 61 other ActorRel entries in configure.py. CI's D44J01 report on this
PR is absent for the same reason.

Correcting this PR's original description, which claimed all four versions: the D44J01
column was asserted from the manifest's expected hashes rather than measured against
the built RELs. Three versions are measured; the fourth is now correctly excluded
rather than claimed.

Member names follow the `field_0xNNN` placeholder convention for unknowns; retained
names mirror the already-merged `d_a_obj_msdan_sub` (`mCurObjNo`, `m2D8`, `m2DC` style).
```

### PR #1175 — `d_a_obj_hami2 OK`

```
Matches `d_a_obj_hami2` for the three retail versions: GZLE01, GZLJ01 and GZLP01.

Decompiled from the split assembly. The REL builds SHA1-identical to
`config/<version>/build.sha1` for those three, each verified locally against its own
unmodified manifest. Status is set to
`MatchingFor("GZLJ01", "GZLE01", "GZLP01")` to match that evidence exactly.

D44J01 (the kiosk demo) does NOT match and is deliberately excluded — the built REL
hashes `c7b0876c...` against the manifest's `cf947424...`. This mirrors the convention
already used by 61 other ActorRel entries in configure.py. CI's D44J01 report on this
PR is absent for the same reason.

Correcting this PR's original description, which claimed all four versions: the D44J01
column was asserted from the manifest's expected hashes rather than measured against
the built RELs. Three versions are measured; the fourth is now correctly excluded
rather than claimed.

Member names follow the `field_0xNNN` placeholder convention for unknowns; `Prm_e` /
`PRM_*` naming mirrors the already-merged `d_a_obj_hami3`.
```

---

## INDEPENDENT CONFIRMATION (2026-08-18)

The Decoder lane ran the falsifiable test on their own initiative: configure for D44J01
under bare `Matching`, delete `.plf`/`.preplf`/`.rel` to force a genuine re-link, rebuild,
hash. Result: **`8a62d937` / `57181aeb` / `c7b0876c` — the mismatch hashes, all three.**
Retracted at WWDP `8916d5a7`, labels restored to `MatchingFor("GZLJ01","GZLE01","GZLP01")`.

**The three mismatch hashes above are therefore measured twice, independently, by two
lanes using different routes.** Nothing in this document rests on a single measurement.

---

## EDIT IN PLACE, DO NOT CLOSE AND REOPEN

Recommended: push the `configure.py` fix to each existing branch and edit each
description. That keeps the `decomp-dev` bot history attached, and the D44J01 report
flipping from "not found" to correctly-excluded is itself the visible proof of the fix.
Closing and reopening discards that trail and reads as concealment rather than
correction.

**These three TUs remain a genuine result.** One unchanged source text reproduces three
separate retail disc images byte-for-byte, across US, Japanese and European builds.
Only the fourth column was ever in question.

---

## HOW IT HAPPENED — for the method ledger, not for blame

1. `PR-STAGING.md` (08-16 21:51) staged all three **conservatively**: `MatchingFor("GZLE01")`,
   with a body explicitly saying *"the other three versions are untested locally for want
   of disc trees."* **This document was correct.**
2. The J and P disc trees landed; those six cells were **genuinely measured**. 9/12, honest.
3. D44J01 completed. Its three cells were "verified" by reading `config/D44J01/build.sha1`
   — the expectation — instead of hashing the built artifacts. 12/12 was declared.
4. On that figure the labels were widened `MatchingFor(E)` → `(E,J,P)` → bare `Matching`,
   and the PRs were opened carrying the four-version claim in their opening description.

**The error is one step, in step 3, and every step after it inherited the number without
re-deriving it.** The lane's own standing method names this exact failure: *check the
ARTIFACT, not the report — and check the EFFECT, not the artifact.* A manifest is the
report. The built REL is the artifact. Upstream CI was the effect, and it had been
saying so publicly the whole time.
