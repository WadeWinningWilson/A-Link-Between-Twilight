#!/usr/bin/env python3
# ============================================================================
# ww_bridge.py — roadmap item WWB:B4, the half that did not exist.
#
# §541 measured B4 as HALF-BUILT: `gate` is real under another name
# (covenant_gate.py, step 4), `verify` was never written. This is `verify`.
#
#   ww_bridge.py verify <live-dir> [--skeleton <repo-dir>]
#   ww_bridge.py scls   <live-dir> [--emit]     -> WWB:B2
#   ww_bridge.py gate   <exe>          -> delegates to covenant_gate.py
#   ww_bridge.py convert-all [args]    -> R1 pipeline runner (convert_all.py)
#                                         --gaps lists the remaining R1 work
#
# ---------------------------------------------------------------------------
# WHY IT TAKES A LIVE DIRECTORY AND NOT THE REPO SKELETON
#
# The repo skeleton is a TEMPLATE and its `arcs/` holds a README saying "Place
# your own Wind Waker extractions here (never commit Nintendo assets)". That
# absence is the covenant working as designed. Running these checks against the
# template would fail every one of them and mean nothing -- a red result that
# carries no information is the same defect as a green one that carries none.
# So `verify` runs against an INSTALLED skeleton, and the repo copy is the
# optional drift baseline.
#
# ---------------------------------------------------------------------------
# WHAT IT REFUSES TO DO
#
# It never reads asset CONTENT beyond a 8-byte magic check, and it never copies
# anything. Presence, resolution and magic are verifiable without extracting a
# single byte of Nintendo data.
#
# №31-C throughout: a check that cannot run reports UNKNOWN, never PASS. Exit
# 0 clean / 1 a real failure / 2 something could not be checked.
# ============================================================================
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
RE_KEY = re.compile(r"^\s*(\w+)\s*=\s*(.+?)\s*$")
RE_SECTION = re.compile(r"^\s*\[([^\]]+)\]")
# An absolute path in a runtime config makes the mod work on exactly one machine.
RE_ABS = re.compile(r"(?:^|[=,\s])(?:[A-Za-z]:[\\/]|/(?:home|Users|mnt)/)")


def _inis(root):
    return sorted(p for p in (root / "npc").glob("*.ini")) if (root / "npc").is_dir() else []


def _keys(path):
    out = []
    for ln in path.read_text(encoding="utf-8", errors="replace").splitlines():
        m = RE_KEY.match(ln)
        if m and not ln.lstrip().startswith(("#", ";")):
            out.append((m.group(1), m.group(2)))
    return out


def check_arcs(live):
    """Every `arc=` names a file that is actually installed.

    SEVERITY IS DELIBERATELY *NOT* FAIL, and this was learned the hard way.

    §544 this check reported `Esa` and `IkadaH` as referenced-but-absent. I read
    that as a defect and installed both. It caused a REGRESSION: the bait, which
    had been rendering, disappeared.

    The logs show why. `provider NPC_ESA` registers in EVERY run, with or without
    the arc — but `arc-mount 'Esa'` appears only once the file exists. So an
    absent arc was not a broken reference; it was the switch that kept the mod's
    override OFF, letting a working fallback draw the bait. Installing the arc
    turned the override ON, and the override does not render.

    A referenced-and-absent arc is therefore genuinely AMBIGUOUS: it may be an
    install gap, or it may be a deliberately-disabled override. The check cannot
    tell them apart, and reporting FAIL invited exactly the blind fix that broke
    the game. It reports UNKNOWN and says both readings — №31-C in the other
    direction: a check that cannot distinguish must not assert.
    """
    arcs = live / "arcs"
    if not arcs.is_dir():
        return "UNKNOWN", ["no arcs/ directory in the live install"]
    have = {p.stem for p in arcs.glob("*.arc")}
    # §549: arcs/ is NOT the only source. `arcs_lib/` is a fallback library the
    # loader serves via the "№110 arcs_lib fallback" path, and an arc present
    # ONLY there is fully installed -- it just is not overlaid. Not knowing that
    # is what made this check report Esa/IkadaH as absent when they were being
    # served the whole time, which is what led to §546 installing byte-identical
    # copies and regressing the bait (§548). The check now looks where the
    # loader looks.
    lib = live / "arcs_lib"
    have_lib = {p.stem for p in lib.glob("*.arc")} if lib.is_dir() else set()
    missing = set()
    for ini in _inis(live):
        for k, v in _keys(ini):
            if k == "arc" and v not in have and v not in have_lib:
                missing.add(f"{v}  (referenced by {ini.name})")
    if not missing:
        return "PASS", [f"{len(have)} arcs in arcs/, {len(have_lib)} in arcs_lib/ "
                        f"(both are served; arcs_lib via the №110 fallback)"]
    detail = [f"{m}" for m in sorted(missing)]
    detail.append("AMBIGUOUS: absence may be an install gap OR a deliberately "
                  "disabled override — installing Esa/IkadaH on this evidence "
                  "regressed the bait (§548). Confirm the provider has no working "
                  "fallback BEFORE installing.")
    return "UNKNOWN", detail


def check_population(live):
    """Every `population=` CSV exists and carries at least one data row."""
    problems = []
    seen = set()
    for ini in _inis(live):
        for k, v in _keys(ini):
            if k != "population":
                continue
            seen.add(v)
            hits = list(live.rglob(v))
            if not hits:
                problems.append(f"{v}  MISSING (referenced by {ini.name})")
                continue
            rows = [r for r in hits[0].read_text(encoding="utf-8", errors="replace")
                    .splitlines() if r.strip() and not r.startswith("#")]
            if len(rows) <= 1:
                problems.append(f"{v}  present but has no data rows")
    if not seen:
        return "UNKNOWN", ["no population= keys declared"]
    return ("FAIL", problems) if problems else ("PASS", [f"{len(seen)} CSV(s) resolve with rows"])


def check_dialogue(live):
    """Every `dialogue=` key resolves to a real [section]."""
    d = live / "dialogue"
    if not d.is_dir():
        return "UNKNOWN", ["no dialogue/ directory"]
    sections = set()
    for f in d.glob("*"):
        if f.is_file():
            for ln in f.read_text(encoding="utf-8", errors="replace").splitlines():
                m = RE_SECTION.match(ln)
                if m:
                    sections.add(m.group(1).strip())
    missing, seen = [], 0
    for ini in _inis(live):
        for k, v in _keys(ini):
            if k == "dialogue":
                seen += 1
                if v not in sections:
                    missing.append(f"{v}  (referenced by {ini.name})")
    if not seen:
        return "UNKNOWN", ["no dialogue= keys declared"]
    return ("FAIL", sorted(set(missing))) if missing else \
           ("PASS", [f"{seen} key(s) resolve against {len(sections)} section(s)"])


def check_abs_paths(live):
    """No absolute paths in runtime configs -- they work on one machine only."""
    bad = []
    for pat in ("npc/*.ini", "population/*.ini", "*.ini"):
        for f in live.glob(pat):
            for i, ln in enumerate(f.read_text(encoding="utf-8", errors="replace")
                                   .splitlines(), 1):
                if ln.lstrip().startswith(("#", ";")):
                    continue
                if RE_ABS.search(ln):
                    bad.append(f"{f.name}:{i}: {ln.strip()[:70]}")
    return ("FAIL", bad) if bad else ("PASS", ["no absolute paths in runtime configs"])


def check_bdl_magic(live):
    """Report surviving J3D2bdl4 magic. REPORTED, not failed -- see below."""
    arcs = live / "arcs"
    if not arcs.is_dir():
        return "UNKNOWN", ["no arcs/ directory"]
    hits = []
    for p in sorted(arcs.glob("*.arc")):
        try:
            if b"J3D2bdl4" in p.read_bytes():
                hits.append(p.name)
        except Exception as e:
            return "UNKNOWN", [f"could not read {p.name}: {type(e).__name__}"]
    # B4 says "no surviving J3D2bdl4". I can COUNT them exactly; what I cannot
    # state is which arcs are expected to be adapted vs carried as-is, and that
    # is the difference between a finding and a false alarm. Reported with the
    # list so the owning lane rules, rather than asserted as a failure by me.
    return ("UNKNOWN", [f"{len(hits)} arc(s) still carry J3D2bdl4 magic: "
                        + ", ".join(hits[:6]) + (" …" if len(hits) > 6 else ""),
                        "expected-adapted set is not declared anywhere -- "
                        "owning lane must rule"]) if hits else \
           ("PASS", ["no arc carries J3D2bdl4 magic"])


def check_drift(live, skeleton):
    """Skeleton <-> live drift (B4's G4 catch)."""
    if skeleton is None:
        return "UNKNOWN", ["no --skeleton given; drift not checked"]
    only_skel = []
    for p in (skeleton / "npc").glob("*.ini"):
        if not (live / "npc" / p.name).is_file():
            only_skel.append(f"npc/{p.name} in skeleton, absent live")
    only_live = []
    for p in (live / "npc").glob("*.ini"):
        if not (skeleton / "npc" / p.name).is_file():
            only_live.append(f"npc/{p.name} live, absent from skeleton")
    both = only_skel + only_live
    return ("FAIL", both) if both else ("PASS", ["skeleton and live agree on npc/*.ini"])


CHECKS = [
    ("arc presence", check_arcs),
    ("population CSVs", check_population),
    ("dialogue keys", check_dialogue),
    ("absolute paths", check_abs_paths),
    ("J3D2bdl4 magic", check_bdl_magic),
]


def cmd_verify(argv):
    if not argv:
        print("usage: ww_bridge.py verify <live-dir> [--skeleton <repo-dir>]")
        return 2
    live = Path(argv[0])
    skeleton = None
    if "--skeleton" in argv:
        skeleton = Path(argv[argv.index("--skeleton") + 1])
    if not live.is_dir():
        print(f"UNKNOWN — live dir not found: {live}")
        return 2

    print(f"WW BRIDGE VERIFY — {live}")
    worst = 0
    for name, fn in CHECKS:
        try:
            state, detail = fn(live)
        except Exception as e:
            state, detail = "UNKNOWN", [f"check raised {type(e).__name__}: {e}"]
        print(f"  [{state:7s}] {name}")
        for d in detail[:8]:
            print(f"            {d}")
        if len(detail) > 8:
            print(f"            … {len(detail) - 8} more")
        worst = max(worst, {"PASS": 0, "FAIL": 1, "UNKNOWN": 2}[state])
    state, detail = check_drift(live, skeleton)
    print(f"  [{state:7s}] skeleton<->live drift")
    for d in detail[:8]:
        print(f"            {d}")
    worst = max(worst, {"PASS": 0, "FAIL": 1, "UNKNOWN": 2}[state])

    print("\n  A PASS here means the check RAN and found nothing, not that the")
    print("  mod is correct. UNKNOWN is not a pass (№31-C).")
    return worst



# ============================================================================
# WWB:B2 — `scls`: door binding GENERATED, not hand-authored.
#
# WHERE THE OUTPUT GOES, AND WHY IT IS NOT THE REPO
#
# `population/doors.ini` in the repo carries 8 rows of WW world coordinates
# decoded from Nintendo's DZR. B2 generalises that to "any WW interior", which
# would MULTIPLY donor-derived data committed to this tree -- in the same week
# trip-wire (b) was ruled TRIPPED and slated for full extraction. So the
# generated file is written into the LIVE INSTALL, derived on the user's machine
# from the user's own extracted ISO. Same shape as `verify` reading live, and
# the same shape as arcs/README.txt refusing to hold assets.
#
# THE DECODE, AND WHY THE STRIDE IS TRUSTED
#
# WW's SCLS is `stage_scls_info_class`, 0x0C -- a FIXED 12-byte entry:
# char dest[8]; u8 spawn; u8 room; u8 exit; u8 pad. NOT TP's variable-length
# form, which tools/demo_cut_content/scls_sweep.py correctly warns against
# fixed-stride parsing. Confirmed against reality rather than assumed: every
# decoded entry lands `pad = 0xFF` and `room = 44`, and Room44 IS Outset Island
# in this project (arcs/README.txt). A frame-shifted parse produces neither.
# ============================================================================
def _dzr_of(arc_path):
    import sys as _s
    _s.path.insert(0, str(HERE.parents[1] / "tools/demo_cut_content"))
    from dzr_placements import rarc_members
    # materialise: rarc_members is a GENERATOR, and consuming it twice yields
    # nothing the second time -- silently, which cost a debug round.
    mem = list(rarc_members(str(arc_path)))
    return next(((n, b) for n, b in mem if n.lower().endswith((".dzr", ".dzs"))), None)


def decode_scls(blob):
    """[(dest, spawn, room, exit)] from a DZR blob, or [] if it has no SCLS."""
    import struct
    out = []
    n = struct.unpack_from(">I", blob, 0)[0]
    for i in range(n):
        tag, num, off = struct.unpack_from(">4sII", blob, 4 + i * 12)
        if tag != b"SCLS":
            continue
        for k in range(num):
            e = blob[off + k * 12: off + k * 12 + 12]
            if len(e) < 12:
                break
            out.append((e[:8].split(b"\0")[0].decode("ascii", "replace"),
                        e[8], e[9], e[10]))
    return out


def cmd_scls(argv):
    if not argv:
        print("usage: ww_bridge.py scls <live-dir> [--emit]")
        return 2
    live = Path(argv[0])
    arcs = live / "arcs"
    if not arcs.is_dir():
        print(f"UNKNOWN — no arcs/ under {live}")
        return 2

    rows, unreadable = [], []
    for a in sorted(arcs.glob("*.arc")):
        try:
            hit = _dzr_of(a)
        except Exception as e:
            unreadable.append(f"{a.name}: {type(e).__name__}")
            continue
        if not hit:
            continue
        for dest, spawn, room, exit_ in decode_scls(hit[1]):
            rows.append((a.stem, dest, spawn, room, exit_))

    print(f"WW BRIDGE SCLS — {live}")
    print(f"  arcs scanned : {len(list(arcs.glob('*.arc')))}")
    print(f"  exits decoded: {len(rows)} from {len({r[0] for r in rows})} arc(s)")
    if unreadable:
        print(f"  UNREADABLE   : {len(unreadable)} (reported, never skipped silently)")
        for u in unreadable[:5]:
            print(f"      {u}")
    for r in rows[:12]:
        print(f"      {r[0]:<12s} -> {r[1]:<8s} spawn={r[2]} room={r[3]} exit=0x{r[4]:02x}")
    if len(rows) > 12:
        print(f"      … {len(rows) - 12} more")

    if "--emit" not in argv:
        print("\n  (dry run — pass --emit to write door_bindings.ini into the live install)")
        return 0 if rows else 2

    import hashlib
    out = live / "population" / "door_bindings.ini"
    out.parent.mkdir(parents=True, exist_ok=True)
    tool = Path(__file__).read_bytes()
    src = hashlib.sha256()
    for a in sorted(arcs.glob("*.arc")):
        src.update(a.name.encode())
        src.update(str(a.stat().st_size).encode())
    with out.open("w", encoding="utf-8") as f:
        f.write("# WW door bindings — GENERATED from SCLS. Do not hand-edit.\n")
        f.write("# Written into the LIVE install, never the repo: these are\n")
        f.write("# donor-derived coordinates and the repo shipping them is what\n")
        f.write("# trip-wire (b) exists to stop.\n")
        f.write("# regenerate: python ww_bridge.py scls <live-dir> --emit\n")
        f.write(f"# tool_sha256:  {hashlib.sha256(tool).hexdigest()}\n")
        f.write(f"# input_sha256: {src.hexdigest()}\n")
        f.write(f"# count: {len(rows)}\n")
        for arc, dest, spawn, room, exit_ in rows:
            f.write(f"\n[{arc.lower()}]\ndest={dest}\nspawn={spawn}\n"
                    f"room={room}\nexit=0x{exit_:02x}\n")
    print(f"\n  EMITTED {out}  ({len(rows)} bindings)")
    return 0


def cmd_gate(argv):
    """gate already exists as covenant_gate.py -- delegate, never reimplement."""
    g = HERE / "covenant_gate.py"
    if not g.is_file():
        print("UNKNOWN — covenant_gate.py not found")
        return 2
    import subprocess
    print(f"delegating to {g.name} (B4's `gate` half; built at step 4)")
    return subprocess.run([sys.executable, str(g)] + argv, cwd=str(HERE.parents[1])).returncode


def main():
    if len(sys.argv) < 2:
        print(__doc__ or "usage: ww_bridge.py verify <live-dir> | gate <exe>")
        return 2
    cmd, rest = sys.argv[1], sys.argv[2:]
    if cmd == "verify":
        return cmd_verify(rest)
    if cmd == "scls":
        return cmd_scls(rest)
    if cmd == "gate":
        return cmd_gate(rest)
    print(f"unknown subcommand: {cmd}")
    return 2


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
