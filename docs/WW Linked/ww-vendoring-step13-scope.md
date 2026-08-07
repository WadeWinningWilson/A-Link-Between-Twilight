# Step 13 — vendoring rework to pinned reference: SCOPE

**Owner:** Engine + Bridge (both absent; **Housing Security covering as Engine**).
**Authorised by:** step 12, ruled 2026-08-06 — vendor by pinned reference, pin =
`zeldaret/tww@be8da688fcc755d77e2cdb7a69124297b01ff683`.
**Status:** MECHANISM C RULED (user, 2026-08-06) and BUILT — `tools/vendoring/ww_rebaseline.py`.
See §5 for what was built and how it differs from what §3 proposed.

---

## 1. The finding that reshapes the step

Step 13 reads "vendoring rework to pinned reference; stops forking zeldaret",
which implies one uniform conversion. **Measurement says the tree is two
populations, and only one of them can be vendored at all.**

Every `native-port` TU carrying a single `KIT-DONOR` was diffed against that
donor file at the pin, after normalising whitespace, comments and the port's own
namespace flattening (`JAudio1::` → `JASystem::` etc.) — because an exact-line
diff scores a systematic rename as total divergence and would have understated
every JAudio1 TU.

Two different questions, reported separately because they answer different things:

* **"how much of OUR file is donor text"** — decides whether the file can come
  from upstream instead of from us.
* **"how much of the DONOR file we cover"** — decides whether we took the whole
  TU or a slice of it.

### Tier V — vendorable (≥70% of our lines are donor text): 20 TUs

`d_a_kamome` 95.3% · `d_a_npc_ls1` 94.1% · `ja1_jasplayer` 92.5% ·
`ja1_jasregisterparam` 88.9% · `ja1_jasseqctrl` 87.2% · `ja1_jasbnkparser` 86.8% ·
`ja1_jastrackinterrupt` 86.4% · `d_a_obj_toripost` 86.2% · `ja1_jasouterparam`
84.3% · `ja1_jasnotemgr` 83.6% · `d_a_npc_zl1` 83.6% · `d_a_obj_mshokki` 80.6% ·
`ja1_jascalc` 80.1% · `d_door` 79.4% · `ja1_jastrackport` 78.4% ·
`d_a_ww_demo00` 72.4% · `d_a_esa` 70.4% (+3 borderline: `d_a_lamp` 69.0%,
`d_a_obj_otble` 67.4%, `d_a_spc_item01` 61.1%)

These are what the step was written for. Our edits are a thin delta over upstream
text, so "upstream at a pin, plus our patch" is both expressible and cheap, and a
pin bump becomes a patch re-application rather than a manual re-diff.

### Tier D — derived, NOT vendorable (<70%): 15 TUs

`d_ext_dmesg` 1.3% · `d_a_npc_ba1` 4.0% · `ww_jpa` 7.1% · `d_ext_tree` 10.2% ·
`d_a_ext_plank_span` 17.3% · `evt1_event_data` 22.1% · `d_kankyo_ww` 23.1% ·
`d_ext_save_flags` 25.4% · `evt1_event_manager` 30.4% · `mdoext1_3dline` 48.7% ·
`d_a_knob00` 50.7% …

**These are our own code that consulted a donor, not copies of it.** Expressing
them as "upstream + patch" would produce a patch larger than the file, which is
not vendoring — it is obfuscation with extra steps. They stay as our source. The
pin still serves them: it is the ref their donor facts were verified against
(E5), which is what a re-baseline needs.

## 2. Provenance findings that fell out of the measurement

Not the point of the exercise, but they are defects and they are recorded rather
than dropped. **Routed to Engine (banner accuracy), not folded into step 13.**

1. **`d_a_npc_ba1.cpp` is tagged `native-port` and is 4.0% donor text** (1,495 of
   our lines; covers 13.6% of the donor). Whatever that TU is, "a whole-file
   donor port" does not describe it. Same shape: `d_ext_dmesg` (1.3%),
   `d_ext_save_flags` (25.4%), `d_kankyo_ww` (23.1%).
2. **`evt1_event_data` / `evt1_event_manager` headers say "donor-verbatim"** and
   are 22.1% / 30.4% donor text, covering 5.3% / 4.3% of their donors. The claim
   is defensible for the *lines they took* — they are deliberate partial
   semantic-layer ports — but "donor-verbatim" invites reading them as whole-file
   ports, which the numbers do not support. Wording, not lineage.

**Stated limit:** the percentages are a FLOOR on verbatim-ness. Normalisation
handles whitespace, comments and the known namespace flattenings; it does not
handle per-symbol renames, reordering, or a port that reflowed argument lists. A
TU can be more donor-derived than its number says. It cannot be less.

## 3. Mechanism options for Tier V — a user call

| | What | Cost | Gets us |
|---|---|---|---|
| **A** | **Git submodule** at the pin + build-time patch application | Highest — build integration, submodule ergonomics on Windows, every contributor clones the donor | True vendoring; upstream is never copied into our tree |
| **B** | **Fetch script** at the pin (no submodule) + patch application | Medium | Same result, no submodule friction; the donor tree stays where it already is |
| **C** | **Keep the copies; build a re-baseline tool** that regenerates each Tier-V TU from `pin + stored patch`, and reports conflicts on a pin bump | Lowest | Stops the *fork* (the stated goal) without changing how the build consumes anything |

**Recommendation: C, then reassess.** The step's stated purpose is "stops forking
zeldaret" — C achieves exactly that: when zeldaret improves a TU, bump the pin,
re-run, resolve. A and B additionally remove donor text from our tree, which is a
*containment* win — but containment is what the plugin boundary (§4 primitive 4)
and the donor-array extraction already own, and both are already scheduled. Doing
it here would duplicate that work in a second place.

C is also the only option that is reversible and that does not touch the build,
which matters because the build is currently green and nothing else in flight
depends on it.

**What C actually is, concretely:** for each of the 20 Tier-V TUs, store the diff
against the pinned donor file; a tool re-applies it after a pin bump and reports
per-TU conflicts. Roughly the shape of `bmd_reskin.py` — mechanical, testable,
and it fails loudly rather than silently.

## 4. Not in scope

* Steps 15–19 (hook ABI, plugin loading, leg migration) — gated on step 8, separate.
* The donor-array extraction (trip-wire (b)) — accepted and deferred by the user
  2026-08-06; it is a *containment* action, not a vendoring one.
* Tier D TUs — they stay as our source, by measurement, not by preference.


---

## 5. What was built (mechanism C), and one deliberate change from §3

`tools/vendoring/ww_rebaseline.py`. Modes: `--status`, `--tier`, `--selftest`,
`--plan <sha>`. **It never writes a TU** — every mode reports what a bump would
do. A tool that rewrites 20 ported TUs in place on a bad pin is not recoverable
from by inspection; landing a re-baseline stays a deliberate, reviewed act.

### No patches are stored — §3 said "stored patch", and that was wrong

A unified diff carries donor source in its **context and removed lines**. Storing
donor→ours patches would therefore add donor text to this tree, in a project
whose standing constraint is that dusklight ships no WW code. It would also
create a second artifact to keep in sync with the file it describes — census
spec §1's drift failure in miniature.

Nothing needs storing. Our file is already in the tree and the donor at the pin
is a public reproducible commit, so the delta is computable on demand and a
re-baseline is exactly a **three-way merge**: base = donor@pin, ours = our file,
theirs = donor@newpin. `git merge-file` does this natively and marks conflicts
rather than guessing. The only persistent state is the pin, which E5 already put
in all 72 banners.

### Negative control

`--selftest` re-baselines every Tier-V TU onto the SAME pin. base == theirs means
the merge has nothing to apply, so the result must be our file byte-identically;
any difference is the tool being wrong, not the code. **Result: PASS, 20/20
byte-identical.** It earned its keep immediately — it failed first, on four TUs
whose code was fine, because the merge was round-tripping through the locale
codec (cp1252) while donor sources carry Shift-JIS comments. A tool whose job is
exact reproduction must not decode to do it; the merge path is bytes end to end.

### Rehearsal

`--plan HEAD~10` (backwards, so no network and no change to the donor tree):
**12 unchanged upstream · 2 merge clean · 6 need hand review.** The heaviest is
`d_a_npc_bm1.cpp` at 88 conflicts — and the target commit is literally
"d_a_npc_bm1 100% Matching Retail (#1107)". That is the value proposition
demonstrated rather than asserted: when upstream finishes a TU, our port needs
re-derivation, and this quantifies it per TU before anyone commits to the work.

## 6. PREREQUISITE surfaced — the donor checkout is SHALLOW

`D:/XXXXXXX/WW DP` is a shallow clone: **14 commits of history**. `--plan`
against anything older resolves nothing, and the tool reports UNKNOWN and
refuses rather than guessing (verified: `--plan HEAD~300` → "cannot resolve …
this tool will not guess").

Real re-baselining needs history, so `git fetch --unshallow` is a prerequisite.
**Not done here** — it is a network operation against the user's own donor
checkout and its size is unknown; that is the user's call, not a tool's.
