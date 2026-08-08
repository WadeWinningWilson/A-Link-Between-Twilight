#!/usr/bin/env python3
"""R1 — `ww_bridge convert-all`: the one-command, resumable, checksum-short-circuited
pipeline runner. Bridge builds · Foundry specs · Housing audits.

WHAT LANDED HERE, AND WHAT DID NOT
----------------------------------
R1's spec assumes the ~20 conversion steps are knowable well enough to declare. Measured
against the tree, they mostly are not: of a 12-script sample of the pipeline's spine, only
THREE declare a usage contract (install_skeleton, restore_arc_resids, merge_event). The rest
carry no Usage line, no declared inputs, and no declared outputs. A declarative recipe
authored over that would be a GUESS wearing a manifest's clothes — and a runner that
confidently executes a guessed order is worse than twenty hand-run scripts, because the hand
runner knows they are being careful.

So this file lands the three things that are real, and measures the gap instead of papering
it:
  1. THE RUNNER — sequential, resumable, idempotent, per-step validated, checksum
     short-circuited (a step whose declared outputs already hash-match is SKIPPED, TTW's
     trick and the thing that makes re-running cheap).
  2. THE RECIPE FORMAT — declarative JSON, versioned from/to, per-step id/script/args/
     produces/tier. Steps carry the same TIER vocabulary as the R5 conversion DB, for the
     same reason: VERIFIED (contract read off the script) vs UNVERIFIED (order/IO inferred).
     The runner REFUSES to execute UNVERIFIED steps unless explicitly asked, so a partial
     recipe can never masquerade as a complete one.
  3. THE CONTRACT-GAP REPORT — `--gaps` lists every pipeline script with no declared usage.
     That list IS the remaining R1 work, and it is now a command instead of an estimate.

WHY THE REFUSAL MATTERS MORE THAN THE COVERAGE
----------------------------------------------
The mod folder is the project's least reproducible artifact. A runner that half-regenerates
it — silently skipping a step whose contract nobody wrote down — produces a folder that LOOKS
converted and is not. That is the §113-STASH failure class with a bigger blast radius. A
runner that stops and says "step 7 is UNVERIFIED, I will not guess" costs a turn; a runner
that guesses costs the folder.

USAGE
    python convert_all.py --gaps                 # what still needs a declared contract
    python convert_all.py --dry-run              # show the plan, run nothing
    python convert_all.py                        # run VERIFIED steps, skip up-to-date ones
    python convert_all.py --include-unverified   # explicit opt-in, never the default
    python convert_all.py --from-step <id>       # resume
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
import re
import subprocess
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
MOD = (Path(os.environ.get("APPDATA", "")) / "TwilitRealm" / "Dusklight"
       / "model_replacements" / "WW-Crew-Restoration")
RECIPE = HERE / "recipe.json"
STATE = MOD / ".convert_all_state.json"


# ---------------------------------------------------------------------------
# contract discovery — the honest half
# ---------------------------------------------------------------------------
# Accepts BOTH styles: docstring "Usage:" and comment "# Usage:". The first
# version missed every #-header script — under-reporting contracts, which for a
# gap report is the dangerous direction (it invents work and hides real coverage).
USAGE_RE = re.compile(r"^\s*#?\s*Usage[:\s]", re.I | re.M)

# Scripts that are instruments/probes, not pipeline steps: absence of a usage
# contract here is not a gap. Keeping this list explicit (rather than filtering
# by name shape) so a real step can never be silently excused.
NOT_PIPELINE = {
    "_dbg_dzr.py", "_list_arc.py", "chunk_census.py", "space_scan.py",
    "inspect_stg.py", "room_expect.py", "stb_timeline.py", "decode_stb.py",
    "jstudio_stb.py", "extract_amori_census.py", "dzx_offset_stable.py",
    "convert_all.py", "ww_bridge.py", "output_roster.py", "donor_roster.py",
    "covenant_gate.py", "ww_layer_manifest.py",
}


def pipeline_scripts():
    for p in sorted(HERE.glob("*.py")):
        if p.name in NOT_PIPELINE or p.name.startswith("__"):
            continue
        yield p


def has_contract(path: Path) -> bool:
    try:
        head = path.read_text(encoding="utf-8", errors="replace")[:4000]
    except OSError:
        return False
    return bool(USAGE_RE.search(head))


def cmd_gaps() -> int:
    missing, ok = [], []
    for p in pipeline_scripts():
        (ok if has_contract(p) else missing).append(p.name)
    print(f"pipeline scripts: {len(ok) + len(missing)}   with contract: {len(ok)}   "
          f"WITHOUT: {len(missing)}\n")
    print("DECLARED (recipe-authorable):")
    for n in ok:
        print(f"   {n}")
    print("\nNO DECLARED CONTRACT — each needs a Usage line stating inputs and outputs")
    print("before it can become a VERIFIED recipe step. This list is the remaining R1 work:")
    for n in missing:
        print(f"   {n}")
    print("\nWriting one Usage line per script is minutes of work for whoever knows the step;")
    print("inferring twenty of them from call sites is a day, and a guess. Route, do not infer.")
    return 0


# ---------------------------------------------------------------------------
# runner
# ---------------------------------------------------------------------------
def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def load_recipe() -> dict:
    if not RECIPE.exists():
        print(f"no recipe at {RECIPE}", file=sys.stderr)
        return {"version": 0, "steps": []}
    return json.loads(RECIPE.read_text(encoding="utf-8"))


def load_state() -> dict:
    if STATE.exists():
        try:
            return json.loads(STATE.read_text(encoding="utf-8"))
        except (OSError, ValueError):
            return {}
    return {}


def save_state(state: dict) -> None:
    try:
        STATE.parent.mkdir(parents=True, exist_ok=True)
        STATE.write_text(json.dumps(state, indent=2), encoding="utf-8")
    except OSError as e:
        print(f"  (state not saved: {e})")


def outputs_current(step: dict, state: dict) -> bool:
    """TTW's idempotence trick: a step whose declared outputs all exist AND still
    hash to what this runner recorded is already done."""
    produces = step.get("produces") or []
    if not produces:
        return False                      # nothing declared ⇒ cannot prove; always run
    recorded = state.get(step["id"], {}).get("hashes", {})
    if not recorded:
        return False
    for rel in produces:
        p = MOD / rel
        if not p.exists() or recorded.get(rel) != sha256(p):
            return False
    return True


def run_step(step: dict, state: dict, dry: bool) -> bool:
    sid = step["id"]
    script = HERE / step["script"]
    args = step.get("args", [])
    if dry:
        print(f"  would run: {script.name} {' '.join(args)}")
        return True
    if not script.exists():
        print(f"  MISSING SCRIPT {script.name} — stopping (a skipped step is a silent gap)")
        return False
    r = subprocess.run([sys.executable, str(script), *args], cwd=str(HERE))
    if r.returncode != 0:
        print(f"  FAILED rc={r.returncode} — stopping so the folder is not half-regenerated")
        return False
    hashes = {}
    for rel in step.get("produces") or []:
        p = MOD / rel
        if p.exists():
            hashes[rel] = sha256(p)
        else:
            print(f"  step declared output '{rel}' but it does not exist — NOT recording")
    state[sid] = {"hashes": hashes}
    save_state(state)
    return True


def cmd_run(dry: bool, include_unverified: bool, from_step: str | None) -> int:
    recipe = load_recipe()
    steps = recipe.get("steps", [])
    if not steps:
        print("recipe has no steps yet — run --gaps; the recipe is authored from declared")
        print("contracts, and inferring them is the one thing this runner will not do.")
        return 2

    state = load_state()
    started = from_step is None
    skipped = ran = 0
    for step in steps:
        sid = step["id"]
        if not started:
            if sid == from_step:
                started = True
            else:
                continue
        tier = step.get("tier", "UNVERIFIED")
        print(f"[{sid}] {step.get('desc', step['script'])}  ({tier})")
        if tier != "VERIFIED" and not include_unverified:
            print("  REFUSED: contract not verified. --include-unverified to force.")
            print("  (a guessed step that 'succeeds' yields a folder that looks converted)")
            return 3
        if outputs_current(step, state):
            print("  up to date — skipped")
            skipped += 1
            continue
        if not run_step(step, state, dry):
            return 1
        ran += 1

    print(f"\ndone: {ran} run, {skipped} skipped (already current)")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description="R1 pipeline runner")
    ap.add_argument("--gaps", action="store_true", help="list scripts with no declared contract")
    ap.add_argument("--dry-run", action="store_true")
    ap.add_argument("--include-unverified", action="store_true")
    ap.add_argument("--from-step")
    a = ap.parse_args()
    if a.gaps:
        return cmd_gaps()
    return cmd_run(a.dry_run, a.include_unverified, a.from_step)


if __name__ == "__main__":
    raise SystemExit(main())
