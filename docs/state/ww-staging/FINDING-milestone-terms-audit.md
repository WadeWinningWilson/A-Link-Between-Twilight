# FINDING — the three milestone terms, re-derived: term 2's evidence was vacuous, term 3's discriminators were not

era: era-independent
<!-- era rationale: evidence audit of a published milestone; method + measurement | Housing/Engine, 2026-08-21 -->

**Purpose.** The 2026-08-21 era boundary published **two of three hard terms as
VERIFIED**. This file re-derives all three from the artifacts on disk. **Term 1
holds. Term 2's CLAIM holds but its EVIDENCE could not have detected a
violation. Term 3 is not verifiable from anything currently on this machine,
and the discriminators proposed for it were never validated — two of them are
contaminated.** Plus one thing nobody had checked: **the milestone artifact was
built from uncommitted source and cannot be rebuilt from any commit.**

Every row below was measured today against the files, not carried forward.

---

## ① Source genuinely stock — **HOLDS, re-derived independently**

`%USERPROFILE%/Documents/dusklight-main`: `HEAD = c880d46fb5`,
`git rev-list --left-right --count origin/main...HEAD` = **0 0**,
`git status --porcelain -- src include sdk` = **empty**. Unchanged from the claim.

---

## ② Zero-bake — **CLAIM SURVIVES A CORRECTED TEST; THE PUBLISHED TEST WAS VACUOUS**

### Defect A — the sweep ran against a DEFLATE container

`ww_donor_disc.dusk` is **a ZIP archive**, not a flat blob:
`50 4b 03 04 … method=deflate`, 4 members (`mod.json` 568 B, `lib/`,
`lib/windows-amd64/`, `lib/windows-amd64/mod.dll` 784,384 B → 784,952 B
uncompressed from 237,981 B shipped).

A raw `grep` over it **cannot see any member's bytes**. The published reading —
*"zero `RARC`/`Yaz0`/`J3D2`/`bdl4`/`TIMG`/`dzb`"* — is what that grep returns for
**every** string, including strings that are certainly inside.

**Positive control, run on the same command that produced the published zero:**

| needle | hits on the shipped `.dusk` | truth |
|---|---|---|
| `WwRegistry` | **0** | present 205× in the payload |
| `ww_donor_disc` | **0** | present |
| `dusk` | **0** | present |

***A test that returns zero for things that are there cannot have its zero read
as absence.*** This is the ledger's recurring shape — the grep-against-a-binary-
that-could-not-contain-it class — reappearing on the milestone's own evidence.

### Defect B — on the real payload, the magics ARE present

Extracted and swept `mod.json` + `mod.dll`:

| magic | occurrences | what they actually are |
|---|---|---|
| `RARC` | 4 | 1 code immediate (`cmp`/`jne`), 3 JSON log format strings |
| `Yaz0` | 3 | 1 code immediate, 2 log format strings |
| `J3D2` | 5 | **all 5** code immediates in `.text` |
| `bdl4` | 2 | both code immediates |
| `bmd3` | 2 | both code immediates |
| `INF1` | 3 | 1 code immediate, 2 log strings |
| `dzb`  | 8 | 7 `[WwRegistry]` log keys + the literal filename `alwd.dzb` |
| `TIMG` · `MAT3` · `SHP1` · `BMD3` · `bti1` | 0 | — |

**Every one of the 27 is a parser constant or a log literal — which is exactly
what a zero-bake plugin MUST contain**, because it recognises those formats in
the user's own ISO at runtime. **No donor payload ships.** The claim is true.

**A PRESENCE-SHAPED CHECK NOW STANDS BESIDE THIS ABSENCE-SHAPED ONE** (Librarian,
reproduced independently the same day, and better than mine on this point): **PE
section sizes.** `.text` **555 KB of 766 KB**, `.data` 2.5 KB, `.rsrc` 1.5 KB,
`modmeta` 8 KB — **there is nowhere for a donor arc to hide.** That argument
survives every magic-scan on this machine being broken, which an absence never does.
*(Their sweep found 24 to my 27; the delta is exactly `INF1`×3, which they had not
searched for. Both sets agree on the classification.)*

**The correct test is not magic-absence — it is payload-absence.** Magic-string
absence would fail on any *honest* zero-bake plugin and pass on a baked one that
compressed its cargo. Stated so the next instance does not re-run the wrong one:
*classify every occurrence by context; a zero here is a smell, not a pass.*

---

## ③ Actually plugin-only — **✅ VERIFIED 2026-08-21** (was: not verifiable from what existed)

### ✅ CLOSED 2026-08-21 — VERIFIED on `dusklight-20260821-143653.log`

The missing input arrived. `term3_verify.py` → **VERIFIED (exit 0)**, and four
further facts back it, so the verdict does not rest on one instrument:

| # | fact | value |
|---|---|---|
| 1 | run's own `Build:` | `v1.4.1-145-dirty (rev c880d46fb57ec…)` — the stock rev |
| 2 | install **named in the log**, not assumed | `~\Documents\dusklight-main\…\mods\ww_donor_disc.dusk` + 4 sibling mods (**5 mods; the fork install loads 2**) |
| 3 | markers proven to fire | `[ExtWw]` **0** · `[ExtNpcMount]` **0** (4,068 / 92 on the fork control) |
| 4 | **module attribution over all 4,870 records** | **no fork-only module emitted anything** |
| 5 | coverage — the run DID the work | `R44_00` room 44 worldize (8+8+1) · `bdl_consume_gate mode=finish` · 11,700 `bg_draw` · 19,811 draw entries · 109,800 ground queries / 28,416 hits · clean shutdown |

**FACT 4 IS THE ONE THAT NEARLY WENT WRONG, AND IT IS A LESSON ABOUT MY OWN METHOD.**
A string-in-binary sweep over the run's 15 distinct bracket tags flagged **7 as
attributable to neither the stock exe nor the plugin — including `[Demo]`, which
IS in the fork exe.** Read as a marker, that is a falsifier. **Read by emitting
module — which the log prints on every line — all 7 clear instantly:** `[Demo]` is a
mod's *display name* (`"[Demo] Ambient Occlusion"`, from `ao_mod`), and
`[CURRENT]`/`[PAUSE]`/`[STBWAIT]` are **ZEV event names interpolated into a stock
`frameInterp` message** — data, not literals. *The binary-substring instrument that
proved term 2's evidence vacuous was itself about to manufacture a false falsifier.
Attribute by emitter, not by substring.*

**FACT 5 IS THE ONE A GREEN RESULT HIDES:** a 42-second boot-and-die reads zero fork
markers too. The absence only means something because the same run loaded room 44 and
drew 19,811 entries.

**WHAT IT DOES NOT SETTLE — AND WHO SETTLED IT.** The log fixes *which binary ran*; it
can never fix *what the frame looked like*, because `3eed7c2` turns a fatal into a silent
non-draw. **The user closed that half the same day: Outset drew and looked right, matching
this lane's written progress.** *The only claim in this audit that no instrument could
have produced came from the person looking at the screen* — worth stating plainly in a
file otherwise built entirely out of measurements. **Open, in their scope:** unported
items / TP-native placement (**grass**) still to be replaced.

### The blocker as it stood before that run

**There was no vanilla run log on this machine.** `%AppData%\TwilitRealm\Dusklight\logs`
holds 10 `dusklight-*.log`; **all 10 are fork builds** (`efa49c5c06` ×6,
`40cbc5febb` ×4). **`c880d46fb5` appears in none of them.** No archived copy
exists in-tree either. Term 3 needs a **fresh run**; nothing on disk can settle it.

### Static half — DONE, and it is sound

Fork exe (`dusklight/build/.../dusklight.exe`, 39,390,720 B, 2026-08-21 13:25) vs
vanilla exe (`dusklight-main/build/.../dusklight.exe`, 45,248,512 B, 2026-08-17 20:02):

**10 of 10 fork-only markers present in the fork exe, 0 of 10 in the vanilla exe.**
The set discriminates, and it is non-vacuous because the same command finds them
next door.

### The part that had NOT been done: purity, and two markers fail it

**A marker the PLUGIN can emit is not a fork discriminator** — the plugin loads on
both binaries. Swept `mod.dll`:

| marker | fork exe | vanilla exe | mod.dll | verdict |
|---|---|---|---|---|
| `[daBg]` · `898-P1` · `898-P2` · `257 skip` · `BgW REGISTERED` | ✓ | 0 | 0 | **VALID** |
| `[ExtWw]` *(bracketed)* | ✓ | 0 | 0 | **VALID** |
| `ExtNpcMount` | ✓ | 0 | 0 | **VALID** |
| `checkDekuLeafGlide` | ✓ | 0 | 0 | **VALID** |
| `ExtWw` *(bare substring)* | ✓ | 0 | **7** | **CONTAMINATED — bracket it** |
| `dExtWwSave_isWwHostStage` | ✓ | 0 | **1** | **CONTAMINATED — plugin hooks it by name** |

### And the runtime positive control the handoff demanded — half of it already exists

The handoff named `§898-P1` / `§757` / `№257` and flagged proving-they-fire as
*the step most likely to be skipped*. **It would have been skipped into a wall:
`[daBg]` fires 0 times on every fork log on disk** — the whole family is unproven
at runtime, on the fork, today.

**Two markers are already proven to fire**, on fork run `dusklight-20260821-114625.log`:

| marker | lines on a fork Outset run | source |
|---|---|---|
| `[ExtWw]` | **4,068** | `src/d/d_ext_npc_mount.cpp` (fork `src/`) |
| `[ExtNpcMount]` | **92** | fork `src/` |

**Use those as the discriminator.** The absence test on vanilla then means
something, because the same two markers are loud on the fork, absent from the
vanilla exe's bytes, and absent from the plugin that loads on both.

### The instrument

`tools/foundry/term3_verify.py` encodes all of the above: purity by construction,
**a refusal to answer without a `--control` log in which a marker actually fires**,
and identity (`Build:` line + on-disk `.dusk` md5) before any VERIFIED. Exercised on
all three exit paths — fork log judged → **FALSIFIED**; no control → **INDETERMINATE**;
green unreachable without the vanilla log. *A check that can only come back green is
not a check.*

---

## ⚠ UNRECORDED UNTIL NOW — the milestone artifact is not reproducible from any commit

The shipped `.dusk` is **byte-identical in both installs** (md5 `cc00092ef79ca5e6b5820adf67e2c75d`,
237,981 B, 2026-08-19 17:12:36) — that part of term 2's identity claim is good.

**But it was built from uncommitted source.** These strings live in `mod.dll` and
**do not exist in `HEAD:mods-src/ww_donor_disc/registry.cpp`** — only in the
worktree (198 insertions, uncommitted):

| string | in shipped `mod.dll` | in `HEAD` | in worktree |
|---|---|---|---|
| `WW_BDL_CONSUME` | 4 | **0** | 7 |
| `bdl_consume_gate` | 1 | **0** | 1 |
| `lwood_zero_shareddl` | 1 | **0** | 1 |
| `lwood_hide_shapepackets` | 1 | **0** | 1 |
| `WW_LWOOD_SKIP_PATCH` | 1 | **0** | 1 |

**Two consequences, and the second is the one that matters.**
1. The handoff's open item — *"do not let the probe edits ride into a milestone
   build"* — is **late: they already did.**
2. **They are not probes.** `WW_BDL_CONSUME` is the **finish-consume that the
   write-up credits with closing the stretch bug** — the milestone's load-bearing
   fix. Reverting the diff to tidy the tree would **delete the fix**.

**And the builder is untracked too:** `mods-src/ww_donor_disc/standalone/build_install.bat`
— the script the write-up names as the build path — is `??` in `git status`, as are
`_research/` and the two `_research_alwd_*.py` decoders the Alwd bisect reasons from.
`WW_BDL_CONSUME` at `HEAD` survives in exactly one place tree-wide: a **mention in
`CALLS.md`**. The board records the fix; the repo does not contain it.

**The tree cannot rebuild the artifact it shipped.** Committing `registry.cpp` is
the user's call and is not taken here; the gap is recorded so nobody reverts a
milestone by housekeeping.
