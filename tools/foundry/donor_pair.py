#!/usr/bin/env python3
# ============================================================================
# donor_pair.py - PAIR EACH PLUGIN FUNCTION WITH ITS DONOR COUNTERPART.
#
# WHY THIS EXISTS: the plugin's KIT-DONOR-STATUS fields mostly read UNKNOWN,
# and the obvious next step - "byte-compare against the donor" - IS NOT
# AVAILABLE AND NEVER WILL BE. "Matching" in decomp means *compiling the
# source reproduces the original binary's bytes*. The donor is PowerPC
# GameCube code; the plugin compiles to x64 for dusklight. Those two artifacts
# cannot be byte-equal, so a "Matching" verdict on plugin CODE would be
# reporting something that cannot exist. (Generated DATA TABLES are the
# exception - those genuinely can be byte-compared, and ww_cam_data.cpp's
# `d_cam_type2.cpp Matching` is a real claim of that kind.)
#
# What IS available for code is a SYMBOL-LEVEL SOURCE DIFF: for each ported
# function, line our implementation up against the donor's. This tool does the
# part that is mechanical - working out WHICH donor function each plugin
# function corresponds to - so the reviewer's judgement is spent on the
# comparison rather than on the archaeology.
#
# THE BUG IN v1, KEPT HERE BECAUSE THE NUMBER IT PRODUCED WAS DANGEROUS:
# single-key normalisation paired 8 of 171 functions. `dKyWw_wind_set` did not
# match donor `dKyw_wind_set`, because the plugin infixes its `Ww` marker INTO
# the donor's prefix and one normalised key cannot see through that. **8/171
# reads as "these files are almost entirely reimplementation" - a damaging
# conclusion drawn from a normaliser bug.** Multi-key matching gives 49.
#
# So: OVER-MATCHING IS THE SAFER ERROR HERE. A false pair costs one line of a
# reviewer's attention. A false MISS silently reclassifies ported code as
# original, which is the exact judgement this tool exists to inform.
#
# Usage:
#   donor_pair.py [file.cpp ...]     default: the UNKNOWN-status ported files
# Reads KIT-DONOR headers for the donor units; requires the donor tree.
# ============================================================================
"""Symbol pairing -- multi-key matching.

v1 produced 8 pairs out of 171 and that number was WRONG, not informative:
`dKyWw_wind_set` failed to pair with donor `dKyw_wind_set` because the
plugin's `Ww` marker is infixed into the donor prefix, and single-key
normalisation cannot see through that. A pairing tool that under-matches
tells the reader "this file is a reimplementation" -- a damaging and false
conclusion drawn from a normaliser bug.

v2 generates SEVERAL candidate keys per symbol and pairs on any overlap:
  · text after the first underscore   (dKyWw_wind_set -> wind_set)
  · lowercase with 'ww' removed and non-alnum stripped
  · camel/underscore tail with known lineage prefixes removed
Over-matching is the safer error here: a false pair costs one line of a
reviewer's attention, a false MISS silently reclassifies ported code.
"""
import io
import re
import sys
from pathlib import Path

PLUG = Path(r"%USERPROFILE%\Documents\A Link Between Twilight WW")
DONOR = Path(r"<decomp-root>\WW DP\src")

FUNC = re.compile(
    r'^[A-Za-z_][\w:<>,\s\*&]*?([A-Za-z_]\w*(?:::[A-Za-z_~]\w*)?)\s*\([^;]*?\)\s*(?:const\s*)?\{',
    re.M)
KEYWORDS = {"if", "for", "while", "switch", "return", "else", "do", "catch"}
PREFIXES = ("dkyww", "dkyw", "dky", "dscnky", "dakankyo", "dkankyo", "wwvrkumo",
            "wwvrbox", "wwkankyo", "wwshore", "wwwave", "wwsky", "wwcam", "ww",
            "dasea", "da", "d")


def funcs(path):
    try:
        txt = io.open(path, encoding="utf-8", errors="replace").read()
    except OSError:
        return []
    txt = re.sub(r'/\*.*?\*/', '', txt, flags=re.S)
    txt = re.sub(r'//[^\n]*', '', txt)
    out, seen = [], set()
    for m in FUNC.finditer(txt):
        n = m.group(1)
        if n in seen or n.split("::")[-1] in KEYWORDS:
            continue
        seen.add(n)
        out.append(n)
    return out


def keys(name):
    """Several candidate keys; pair on ANY overlap."""
    base = name.split("::")[-1]
    ks = set()
    low = base.lower()
    if "_" in base:                      # dKyWw_wind_set -> wind_set
        ks.add(base.split("_", 1)[1].lower().replace("_", ""))
    flat = re.sub(r'[^a-z0-9]', '', low.replace("ww", ""))
    ks.add(flat)
    stripped = re.sub(r'[^a-z0-9]', '', low)
    for p in PREFIXES:
        if stripped.startswith(p) and len(stripped) > len(p) + 2:
            ks.add(stripped[len(p):])
            break
    ks.add(stripped)
    return {k for k in ks if len(k) > 3}


def main():
    targets = sys.argv[1:] or [
        "ww_wave.cpp", "ww_shore.cpp", "ww_sky.cpp", "ww_kankyo.cpp",
        "ww_kankyo_wind.cpp", "ww_vrbox.cpp", "ww_vrkumo.cpp"]
    print("SYMBOL-PAIRING WORKSHEET v2   plugin <-> donor")
    tot_p = tot_u = 0
    for name in targets:
        p = PLUG / name
        if not p.is_file():
            continue
        hdr = io.open(p, encoding="utf-8", errors="replace").read()[:4000]
        units = re.findall(r'^// KIT-DONOR:\s*(\S+\.cpp)', hdr, re.M)
        dmap = {}
        for u in units:
            for fn in funcs(DONOR / u):
                for k in keys(fn):
                    dmap.setdefault(k, set()).add("%s::%s" % (Path(u).name, fn))
        pf = funcs(p)
        paired, unp = [], []
        for fn in pf:
            hits = set()
            for k in keys(fn):
                hits |= dmap.get(k, set())
            (paired if hits else unp).append((fn, sorted(hits)))
        tot_p += len(paired); tot_u += len(unp)
        pct = (100.0 * len(paired) / len(pf)) if pf else 0
        print("=" * 72)
        print("%-20s %d fns | PAIRED %d (%.0f%%) | plugin-only %d"
              % (name, len(pf), len(paired), pct, len(unp)))
        for fn, hits in paired[:10]:
            print("   PAIR %-26s <-> %s" % (fn, hits[0]))
        if len(paired) > 10:
            print("   ... %d more" % (len(paired) - 10))
        if unp:
            print("   PLUGIN-ONLY: " + ", ".join(f for f, _ in unp[:10]))
    print("=" * 72)
    print("TOTAL paired %d | plugin-only %d  (v1 said 8 paired -- that was a bug)"
          % (tot_p, tot_u))


main()
