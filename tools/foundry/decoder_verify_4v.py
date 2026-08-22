#!/usr/bin/env python3
"""Routine four-version byte verification of Decoder's TUs.

Standing order (user, 2026-08-21): "routinely verify Decoder work across the
four versions to ensure vanilla byte verification."

USES THE PROJECT'S OWN INSTRUMENT, not an invented one:
`config/<VERSION>/build.sha1` is what configure.py itself checks
(`config.check_sha_path`). Do NOT compare a built .rel against
`orig/<v>/files/rels/*.rel` -- the three RETAIL discs ship RELs Yaz0-COMPRESSED
(magic 59617a30) while the kiosk demo D44J01 ships them RAW, so that comparison
reports 3 false mismatches and 1 true match every time.

A `NonMatching` TU's hash agreeing proves NOTHING: NonMatching objects are not
linked, so the build emits the ORIGINAL rel and the hash matches vacuously.
Certification only means something for Matching / MatchingFor / Equivalent.
"""
import hashlib, re, sys
from pathlib import Path

ROOT = Path(r"D:/XXXXXXX/WW DP")
VERSIONS = ["GZLE01", "GZLJ01", "GZLP01", "D44J01"]

def statuses():
    """TU -> declared status, from configure.py (the certification surface)."""
    out, txt = {}, (ROOT / "configure.py").read_text(encoding="utf-8", errors="replace")
    for m in re.finditer(r'ActorRel\(\s*(\w+)[^,]*,\s*"([^"]+)"', txt):
        out[m.group(2)] = m.group(1)
    return out

def manifest(v):
    f = ROOT / "config" / v / "build.sha1"
    d = {}
    if f.exists():
        for line in f.read_text(encoding="utf-8", errors="replace").splitlines():
            p = line.split(None, 1)
            if len(p) == 2:
                d[p[1].strip()] = p[0].strip()
    return d

def sha1(p):
    r"""hashlib, NOT `sha1sum`.

    TRAP: GNU sha1sum ESCAPES output when the path contains a backslash -- it
    prefixes the whole line with `\` and doubles the separators, so
    `.split()[0]` yields "\6422280d..." instead of "6422280d...". Every cell
    then reads MISMATCH while the underlying bytes are perfect. Cost one false
    all-red board on a Windows path.
    """
    try:
        h = hashlib.sha1()
        with open(p, "rb") as f:
            for chunk in iter(lambda: f.read(1 << 20), b""):
                h.update(chunk)
        return h.hexdigest()
    except OSError:
        return None

def main(tus):
    st, mans = statuses(), {v: manifest(v) for v in VERSIONS}
    print(f"{'TU':<22}{'status':<14}" + "".join(f"{v:<11}" for v in VERSIONS))
    print("-" * 80)
    rc = 0
    for tu in tus:
        s = st.get(tu, "?")
        vacuous = s.startswith("NonMatching")
        cells = []
        for v in VERSIONS:
            rel = f"build/{v}/{tu}/{tu}.rel"
            exp, got = mans[v].get(rel), sha1(ROOT / rel)
            if got is None:      cells.append("unbuilt")
            elif exp is None:    cells.append("no-entry")
            elif exp == got:     cells.append("vacuous" if vacuous else "MATCH")
            else:                cells.append("MISMATCH"); rc = 1
        print(f"{tu:<22}{s:<14}" + "".join(f"{c:<11}" for c in cells))
    print("\nMATCH = byte-verified against the project's manifest.")
    print("vacuous = NonMatching TU; the original rel is linked, so agreement proves nothing.")
    return rc

if __name__ == "__main__":
    args = sys.argv[1:] or ["d_a_npc_kamome", "d_a_npc_ko1", "d_a_npc_ob1",
                            "d_a_npc_p2", "d_a_npc_ym1", "d_a_npc_yw1"]
    sys.exit(main(args))
