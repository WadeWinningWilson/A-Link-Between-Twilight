#!/usr/bin/env python3
# ============================================================================
# cargo_rank.py — WHICH WW ACTOR IS CHEAPEST TO MOVE PLUGIN-SIDE?
#
# The mechanism is closed: donor data serves on stock (145418), by-name hooks
# fire on stock (180329), and an above-enum profile index dispatches on stock
# (203926). "The road is built, paved and driven. What remains is cargo."
#
# Cargo means a REAL WW actor with real behaviour, living plugin-side. The
# plugin carries no receiver headers, so every receiver API an actor calls must
# be resolved BY NAME at runtime — one binding each. **So the migration cost of
# an actor is the number of DISTINCT receiver symbols it calls.** That is
# countable today, per actor, and it turns "which one first?" from a matter of
# taste into a ranking.
#
# WHAT THIS IS NOT: it does not say an actor is portable, only what it would
# cost. An actor with few bindings can still be blocked on a system (dPa_name),
# and a low score is a STARTING POINT for a preflight, never a substitute for
# one. Absent/unmeasurable is reported, never scored as zero (№31-C).
#
# Usage: cargo_rank.py [--top N]
# ============================================================================
import re
import subprocess
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
REG = REPO / "src" / "d" / "ext_plugin" / "ww_profile_register.cpp"
ACTORS = REPO / "src" / "d" / "actor"

# Receiver-API families an actor calls. These are the surfaces a plugin-side
# actor would have to bind by name; local statics and donor-internal helpers
# are not bindings and are excluded by the prefix filter.
RE_API = re.compile(
    r"\b((?:fopAc|fopAcM|fopMsgM|fopEv|fpc|dComIfG|dComIfGp|dComIfGd|dCc|dBgS|"
    r"dKy|dPa|dMsg|dStage|dEvt|dDemo|cM|cLib|mDoExt|mDoLib|mDoMtx|J3D)\w+)")


def strip_comments(text):
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    return re.sub(r'"(?:[^"\\]|\\.)*"', '""', text)


def registered_syms():
    """The WW_ROW(ix, sym) entries — the actors already in the exe."""
    if not REG.is_file():
        return []
    txt = REG.read_text(encoding="utf-8", errors="replace")
    return re.findall(r"WW_ROW\(0x([0-9A-Fa-f]+),\s*(\w+)\)", txt)


def tu_for(sym):
    """Map a profile symbol to its TU by locating g_profile_<sym>."""
    r = subprocess.run(["grep", "-rl", "-F", "g_profile_%s" % sym, str(ACTORS)],
                       capture_output=True, text=True)
    files = [f for f in r.stdout.splitlines() if f.endswith(".cpp")]
    return Path(files[0]) if files else None


def main():
    top = 999
    if "--top" in sys.argv:
        i = sys.argv.index("--top")
        if i + 1 < len(sys.argv):
            top = int(sys.argv[i + 1])

    rows = registered_syms()
    if not rows:
        print("UNRESOLVED: no WW_ROW entries found in %s (not proof of "
              "anything)" % REG)
        return 2

    print("CARGO RANK — migration cost of each registered WW actor, measured as")
    print("the number of DISTINCT receiver symbols it would have to bind by name")
    print("plugin-side. Lower = cheaper first cargo. %d registered actor(s).\n"
          % len(rows))

    measured, unresolved = [], []
    for ix, sym in rows:
        tu = tu_for(sym)
        if tu is None:
            unresolved.append((sym, "no TU defines g_profile_%s" % sym))
            continue
        src = strip_comments(tu.read_text(encoding="utf-8", errors="replace"))
        hits = set(RE_API.findall(src))
        # SPLIT CALLABLES FROM ENUM CONSTANTS — the first count overstated cost
        # about fivefold. Seven of TAG_SO's ten "bindings" were `_e` enum
        # constants (fopAc_ACTOR_e, fpcLy_CURRENT_e, ...), which are COMPILE-TIME
        # VALUES a plugin embeds directly. Only a callable needs a by-name
        # binding at runtime, and that is the number that prices the migration.
        # (An embedded enum value is not free either — it is a version
        # assumption rather than a binding — so both columns are reported.)
        enums = {a for a in hits if a.endswith("_e") or a.isupper()}
        calls = hits - enums
        measured.append((len(calls), sym, ix, tu.name, len(src.splitlines()),
                         calls, enums))

    measured.sort()
    print("  %-6s %-6s %-16s %-26s %6s %7s"
          % ("calls", "enums", "profile", "TU", "lines", "index"))
    for n, sym, ix, name, lines, _c, en in measured[:top]:
        print("  %-6d %-6d %-16s %-26s %6d   0x%s"
              % (n, len(en), sym, name, lines, ix))

    if unresolved:
        print("\n  UNRESOLVED (reported, never scored as zero — №31-C):")
        for sym, why in unresolved:
            print("    %-16s %s" % (sym, why))

    if measured:
        cheap = measured[0]
        print("\n  CHEAPEST CARGO: %s (0x%s, %s) — %d distinct receiver bindings."
              % (cheap[1], cheap[2], cheap[3], cheap[0]))
        print("  Its binding set:")
        for a in sorted(cheap[5]):
            print("      %s" % a)
        print("\n  LIMIT: a low score is a STARTING POINT for a preflight, not a")
        print("  verdict that the actor is portable. It can still be blocked on a")
        print("  system (e.g. dPa_name) that this count does not model.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
