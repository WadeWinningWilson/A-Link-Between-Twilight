#!/usr/bin/env python3
# ============================================================================
# winditor_oracle.py — §394: adapter over the Winditor resource databases, used
# as an INDEPENDENT ORACLE for the things Foundry has been hand-decoding.
#
# WHY THIS EXISTS (lane correction, user's call): Foundry's job is instruments,
# but the instruments had become hand-rolled reimplementations of formats that
# already have mature implementations — and three published claims this session
# (§386, §392, §393) were all a hand-rolled decoder getting a field offset or an
# extent wrong. The fix is not "be more careful"; it is to stop being the only
# implementation. Winditor (LordNed, open source) ships:
#
#   ActorDatabase.json          actor name -> class, archive, models, and the
#                               vanilla LOCATIONS the actor appears in
#   ActorResourceDatabase.json  actor -> the resources it loads
#   EventDefinitionDatabase.json  event/staff definitions (the §298 tale.stb lane)
#   BGM*/WaveBanks/SongNames    the audio-identification lane (§376)
#   templates/*.json            per-actor PARAMETER BIT LAYOUTS (the §312 lane)
#
# These are community-authored databases, not donor bytes: reading them is safe
# under №31, and nothing from them is copied into the repo — this adapter reads
# them in place from the Winditor install.
#
# FILE QUIRK, handled rather than assumed: these .json files carry TRAILING
# COMMAS before closing brackets, which strict json.loads rejects. They are
# stripped here; if a file still fails to parse the tool says UNKNOWN rather
# than returning a partial view.
#
# Usage:
#   winditor_oracle.py actor <name> [<name> ...]     lookup by actor name
#   winditor_oracle.py where <substring>             actors whose Locations match
#   winditor_oracle.py params <template>             a parameter template
#   winditor_oracle.py list-templates
# Read-only.
# ============================================================================
import json
import re
import sys
from pathlib import Path

ROOT = Path(r"D:\XXXXXXX\Winditor\_release\app\resources")
# §394b — the PER-ACTOR parameter templates (421 of them: bit masks, enums and
# tooltips for each actor's params word) live in the SOURCE tree, not in the
# shipped resources folder. This is the oracle for the work §312 did by hand.
TEMPLATES = Path(r"D:\XXXXXXX\Winditor\Editor\resources\templates\MapEntityData")


def load(name):
    p = ROOT / name
    if not p.exists():
        print(f"UNKNOWN — {p} not present (is Winditor installed?)")
        return None
    txt = p.read_text(encoding="utf-8-sig")
    # trailing commas before ] or }
    txt = re.sub(r",(\s*[\]\}])", r"\1", txt)
    try:
        return json.loads(txt)
    except json.JSONDecodeError as e:
        print(f"UNKNOWN — {name} did not parse after comma repair: {e}")
        return None


def actor_index(db):
    out = {}
    for e in db:
        n = e.get("Actor Name")
        if n:
            out.setdefault(n, []).append(e)
    return out


def show(e):
    loc = e.get("Locations") or []
    print(f"  Actor Name      : {e.get('Actor Name')}")
    print(f"  ActorClassType  : {e.get('ActorClassType')}")
    print(f"  English Name    : {e.get('English Name')}")
    print(f"  Archive Name    : {e.get('Archive Name')}")
    print(f"  Main Model      : {e.get('Main Model')}")
    sec = e.get("Secondary Models")
    if sec:
        print(f"  Secondary Models: {sec}")
    wait = e.get("Wait Animations")
    if wait:
        print(f"  Wait Animations : {wait}")
    if e.get("Explanation"):
        print(f"  Explanation     : {e['Explanation']}")
    print(f"  Locations ({len(loc)}) : {', '.join(loc) if loc else '(none listed)'}")


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__ or "see header")
    cmd = sys.argv[1]

    if cmd == "list-templates":
        for p in sorted(TEMPLATES.glob("*.json")):
            print(f"  {p.stem}")
        return 0

    if cmd == "params":
        p = TEMPLATES / (sys.argv[2] + ".json")
        if not p.exists():
            cand = [q.stem for q in TEMPLATES.glob("*.json")
                    if sys.argv[2].lower() in q.stem.lower()]
            print(f"no template '{sys.argv[2]}'." +
                  (f" close: {cand}" if cand else ""))
            return 2
        print(p.read_text(encoding="utf-8-sig"))
        return 0

    db = load("ActorDatabase.json")
    if db is None:
        return 2
    idx = actor_index(db)

    if cmd == "actor":
        for want in sys.argv[2:]:
            hits = idx.get(want) or [e for n, es in idx.items()
                                     if want.lower() == n.lower() for e in es]
            if not hits:
                near = sorted(n for n in idx if want.lower() in n.lower())[:8]
                print(f"\n=== {want}: NOT FOUND" +
                      (f"  (near: {near})" if near else ""))
                continue
            print(f"\n=== {want}  ({len(hits)} entry/entries)")
            for e in hits:
                show(e)
        return 0

    if cmd == "where":
        pat = sys.argv[2].lower()
        rows = [e for e in db
                if any(pat in (l or "").lower() for l in (e.get("Locations") or []))]
        print(f"{len(rows)} actors list a location matching '{sys.argv[2]}':\n")
        print(f"{'actor':22s} {'class':22s} {'archive':16s} main model")
        for e in sorted(rows, key=lambda x: (x.get("Actor Name") or "")):
            print(f"{str(e.get('Actor Name')):22s} "
                  f"{str(e.get('ActorClassType')):22s} "
                  f"{str(e.get('Archive Name')):16s} {e.get('Main Model')}")
        return 0

    print(f"unknown command '{cmd}'")
    return 2


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
