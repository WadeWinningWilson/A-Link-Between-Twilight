# PR staging — three (A)-clean TUs for zeldaret/tww (DECODER)

> Status: AUTHORIZED by the user (informed, identity questions answered, on the
> board in their words). Execution: by any session holding DIRECT in-chat
> authorization — Foundry's session qualifies; the Decoder lane activates on
> the user's word in its own chat. Checklist source: Foundry's authorization
> row + hold-clearance row. ONE TU PER PR. File each PR URL back to CALLS.md.

## Common facts (all three PRs)

- Base repo: `zeldaret/tww`, donor tree at `D:\XXXXXXX\WW DP` (junction `D:\XXXXXXX\WWDP`).
- All work verified byte-true for GZLE01 against upstream's own untouched
  `config/GZLE01/build.sha1`; independent certifications s2f8ac419 / s8c02df15 /
  s6a91c73f (History/Bridge), self-enrolling re-cert sweep green.
- Rename sweep DONE: invented `mEventId`/`mState` → `field_0x298`/`field_0x29c`;
  all retained names are upstream-attested (merged sibling vocabulary:
  msdan_sub's `mPhs`/`mCurObjNo`/`M_arcname`/`M_tmp_mtx`; hami3's `Prm_e`/`PRM_*`).
  RELs re-hashed clean after the sweep.
- configure.py lines use `MatchingFor("GZLE01")` — exactly the local evidence.
  (Upstream may prefer bare `Matching` if their CI proves the other three; say so.)
- Working tree also contains d_a_obj_msdan (99.95%, NOT for PR) — exclude its
  files from every branch.

## PR 1 — title: `d_a_obj_msdan2 OK`

Files: `configure.py` (only the `d_a_obj_msdan2` line), `include/d/actor/d_a_obj_msdan2.h`, `src/d/actor/d_a_obj_msdan2.cpp`
REL SHA1: `416e1c4b893d5986f42b67246700df8266f1407d` == build.sha1

Body:
```
Matches d_a_obj_msdan2 for GZLE01 (REL SHA1-identical to config/GZLE01/build.sha1,
verified locally). The other three versions are untested locally for want of
disc trees; the TU contains no #if VERSION guards, so CI is expected to pass —
flagged here rather than discovered. Status set MatchingFor("GZLE01");
maintainers may prefer Matching if CI proves all four.
Member names follow the field_0xNNN placeholder convention for unknowns.
```

## PR 2 — title: `d_a_obj_msdan_sub2 OK`

Files: `configure.py` (only its line), `include/d/actor/d_a_obj_msdan_sub2.h`, `src/d/actor/d_a_obj_msdan_sub2.cpp`
REL SHA1: `78b670a883570475fae7ed68f8fb58921464c885` == build.sha1

Body: as PR 1, adjusted; note member names mirror the already-merged
`d_a_obj_msdan_sub` (`mCurObjNo`, `m2D8`, `m2DC` style).

## PR 3 — title: `d_a_obj_hami2 OK`

Files: `configure.py` (only its line), `include/d/actor/d_a_obj_hami2.h`, `src/d/actor/d_a_obj_hami2.cpp`
REL SHA1: `de364dda26847a39e8570d030efb587284e70c7e` == build.sha1

Body: as PR 1, adjusted; note `Prm_e`/`PRM_*` naming mirrors merged
`d_a_obj_hami3`, members use `field_0xNNN`.

## Branch surgery (whoever submits)

From clean upstream base: per PR, apply ONLY that TU's three files (take the
header+source from the working tree; edit configure.py to flip ONLY that TU's
line to `MatchingFor("GZLE01")`). Do NOT carry the msdan files or the other
TUs' configure lines. Verify per branch: build the REL, SHA1-compare, then
`ninja changes` == that one unit.

## Explicitly NOT authorized

The decomp.me scratch (d_a_lod_bg::loadModelData) — separate yes/no pending
with the user. Authorization is per-action; an adjacent yes is not this yes.
