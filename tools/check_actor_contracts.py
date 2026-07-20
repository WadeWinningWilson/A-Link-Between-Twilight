#!/usr/bin/env python3
"""№133: static gate for census-spawnable actor contracts.

WHY THIS EXISTS, AND WHY IT IS NOT A RUNTIME GUARD
--------------------------------------------------
The first attempt at protecting these contracts was a runtime guard that
inspected the engine's actor lifecycle — asking whether an actor was alive,
reasoning about culling, layers and saves. That was misdirected. Those meanings
belong to the original engine, and every time the guard learned one it was
another chance to get the original's semantics wrong. A guard that has to fight
the engine to work is in the wrong place.

The actual failure mode is not the engine misbehaving. It is a NEW ACTOR being
written — by a person or, far more often here, by an AI instance that did not
read the docs — that silently breaks a contract the spawner depends on. Those
breaks are invisible at runtime (the symptom lands on OTHER actors, and dialogue
keeps working, so it reads as "something got scrambled" rather than "this file
is wrong"). Each one cost a full playtest cycle to find.

So the check belongs HERE: in the repo, before the code ever runs, naming the
offending file and line. The engine is left alone to be itself.

CONTRACTS CHECKED (each one is a bug that actually shipped)
----------------------------------------------------------
C1  An actor reachable from the census spawner must CONSUME its pending-spawn
    entry (dExtNpcMount_takePendingSpawn) in create. The spawner pushes one for
    every census row; leaking it hands this actor's identity to a LATER actor.

C2  ...and must never return cPhs_ERROR_e from create. fopAcM_create has already
    returned a valid id by then, so the spawner cannot discard the entry. Fail
    inert instead: return cPhs_COMPLEATE_e having built nothing.

C3  Any custom GX vertex-descriptor block must restore state afterwards with
    J3DShape::resetVcdVatCache(). Omitting it makes every model drawn AFTER this
    actor render as nothing — the whole cast disappears while still spawning.

Reachability is not guessed: the census-spawnable proc list is parsed out of
dExtNpcMount_socketActorId(), which is the single place that maps a manifest
socket name to an actor id. An actor that is not reachable that way is skipped.

Usage:  python tools/check_actor_contracts.py            (exit 1 on violation)
        python tools/check_actor_contracts.py --list     (show what is checked)
"""
from __future__ import annotations

import io
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
MOUNT = ROOT / "src" / "d" / "d_ext_npc_mount.cpp"
ACTOR_DIR = ROOT / "src" / "d" / "actor"


def strip_comments(text: str) -> str:
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    return re.sub(r"//[^\n]*", "", text)


def census_reachable_procs() -> set[str]:
    """Parse socketActorId() — the one place a socket name becomes an actor id."""
    if not MOUNT.is_file():
        return set()
    src = strip_comments(io.open(MOUNT, encoding="utf-8", errors="replace").read())
    start = src.find("dExtNpcMount_socketActorId")
    if start < 0:
        return set()
    body = src[start : start + 4000]
    return set(re.findall(r'strcmp\(socketName,\s*"([^"]+)"\)', body))


def profile_procs(text: str) -> set[str]:
    """fpcNm_*_e names this translation unit registers a profile for."""
    return set(re.findall(r"/\*\s*Proc Name\s*\*/\s*(fpcNm_\w+)", text))


def main() -> int:
    reachable = census_reachable_procs()
    # fpcNm_FOO_e  <->  socket name FOO / NPC_FOO
    reach_ids = set()
    for name in reachable:
        reach_ids.add(f"fpcNm_{name}_e")
        reach_ids.add(f"fpcNm_NPC_{name}_e")
        if name.startswith("NPC_"):
            reach_ids.add(f"fpcNm_{name[4:]}_e")

    if "--list" in sys.argv:
        print("census-reachable sockets:", ", ".join(sorted(reachable)) or "(none)")

    violations = []
    checked = []
    for path in sorted(ACTOR_DIR.glob("*.cpp")):
        raw = io.open(path, encoding="utf-8", errors="replace").read()
        text = strip_comments(raw)
        # NB: parse the profile block from RAW text. The "/* Proc Name */" marker
        # is itself a comment, so matching on the stripped text finds nothing and
        # the gate silently passes everything — which it did on first run.
        procs = profile_procs(raw)
        if not procs:
            continue

        rel = path.relative_to(ROOT).as_posix()
        is_reachable = bool(procs & reach_ids)

        # C3 applies to ANY actor that touches vertex descriptors, reachable or not.
        if "GXSetVtxDescv" in text and "resetVcdVatCache" not in text:
            line = next(
                (i for i, l in enumerate(raw.splitlines(), 1) if "GXSetVtxDescv" in l), 0
            )
            violations.append(
                (rel, line, "C3", "custom GX vertex block without J3DShape::resetVcdVatCache()"
                 " — everything drawn after this actor will render as nothing")
            )

        if not is_reachable:
            continue
        checked.append((rel, sorted(procs & reach_ids)))

        if "dExtNpcMount_takePendingSpawn" not in text:
            violations.append(
                (rel, 0, "C1", "census-spawnable actor never calls "
                 "dExtNpcMount_takePendingSpawn() — its identity entry leaks to a later actor")
            )

        for i, line in enumerate(raw.splitlines(), 1):
            if "cPhs_ERROR_e" in line and not line.strip().startswith(("//", "*")):
                violations.append(
                    (rel, i, "C2", "returns cPhs_ERROR_e from a census-spawnable actor — "
                     "fopAcM_create already returned a valid id; fail inert with "
                     "cPhs_COMPLEATE_e instead")
                )

    if "--list" in sys.argv:
        for rel, procs in checked:
            print(f"  checked {rel}  ({', '.join(procs)})")

    if violations:
        print(f"\nACTOR CONTRACT VIOLATIONS ({len(violations)}):\n")
        for rel, line, code, msg in violations:
            where = f"{rel}:{line}" if line else rel
            print(f"  [{code}] {where}\n        {msg}\n")
        print("These are silent at runtime and land on OTHER actors. Fix before shipping.")
        return 1

    print(f"actor contracts OK — {len(checked)} census-spawnable actor(s) checked, 0 violations")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
