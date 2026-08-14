#!/usr/bin/env python3
# ===========================================================================
# l2c_equiv.py — L2c: runtime BDL-adaptation equivalence, proven not assumed.
#
# THE RISK IT RETIRES (ferry §762, the board's highest-risk item): the disc
# serves vanilla BDL4 verbatim; the runtime adapts at load (custom_assets
# path); the offline pipeline (adapt_bdl_arcs, now quarantined) adapted at
# bake. If the two disagree, models are silently wrong — this class does not
# crash. The test: HASH the runtime's adapted output against the offline-
# staged bytes, member by member.
#
# TWO HALVES:
#   ENGINE (tale §792 spec): DUSK_L2C_DUMP=1 writes each adapted model's raw
#     bytes to %APPDATA%/TwilitRealm/Dusklight/l2c_dump/<Arc>__<member>.bdl
#   THIS HARNESS: for each dump, find the staged counterpart (the offline-
#     adapted reference: arcs_retired_row21/ first — including lib_* — then
#     arcs_lib/, then live arcs/), extract the same-named member, sha256 both.
#
# VERDICTS (the §615/HT-29 discipline baked in):
#   EQUIV         bytes identical — the runtime path matches the offline one
#   DIFFER        mismatch — named, first-divergence offset printed
#   NO-REFERENCE  dumped model has no staged counterpart — reported, not skipped
#   and an empty dump dir is exit 2 UNTESTED, never a pass.
#
# WHY IT MATTERS THREE WAYS: (1) closes L2c itself; (2) per-class EQUIV
# coverage IS the resolver-generality receipt holding the 538-arc retirement;
# (3) the interior packs' disc conversion runs through exactly this proof.
#
# Usage:
#   l2c_equiv.py                      compare the dump dir against staged refs
#   l2c_equiv.py --dump <dir>         explicit dump location
#   l2c_equiv.py --selftest           synthesizes arc+dump, proves all verdicts
# ===========================================================================
import hashlib
import io
import os
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from ww_disc import yaz0_decompress, rarc_list

MOD = Path(os.path.expandvars(r"%APPDATA%\TwilitRealm\Dusklight")) / \
      "model_replacements" / "WW-Crew-Restoration"
DUMP = Path(os.path.expandvars(r"%APPDATA%\TwilitRealm\Dusklight")) / "l2c_dump"
REF_ROOTS = ("arcs_retired_row21", "arcs_lib", "arcs")


# ============================================================================
# v2 (tale §807): THE CANONICAL REFERENCE IS THE DISC, adapted on compare.
# The staged arcs proved to be bake-era receipts, not truth (the Ls pair's
# "divergence" was a 4-byte telescope-era EDIT in the STAGED copy; the runtime
# output was the pristine donor). Reference now = disc bdl4 run through the
# SAME MDL3->bmd3 transform the §792 dump hook applies — imported from the
# quarantined baker (import-safe by the gate's design; the OFFLINE transform
# is the shared truth, run here at COMPARE time, writing nothing).
# With this, §798's Class B collapses (transform==transform), NO-REF collapses
# (every room/stage model has a disc counterpart), and the 538 hold has ONE
# comparator. Staged refs remain a FALLBACK, loudly labeled.
# ============================================================================
import json as _json
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "ww_crew_restoration_skeleton"))
import struct as _st


def strip_mdl3(buf):
    """The §792 dump hook's ACTUAL transform — MDL3 strip + retag ONLY, no
    normalizers. DISCRIMINATED against the live corpus (Mshokki koppu:
    strip-only == runtime dump byte-for-byte; full adapt_bdl != — its
    litmask/tevregs normalizers are OFFLINE-bake extras the hook does not
    apply). Mirrors adapt_bdl_arcs.py:165-192 minus the normalize calls.
    RULED (tale §809): both normalizers were crutches for then-unported donor
    lighting systems, retired BY the §687/§694 ports — not consume-path gaps.
    Tripwire: any half-bright / channel-blacked WW model reopens this as a
    LIGHTING-PORT gap, never a re-bake. This comparator matches the hook."""
    if bytes(buf[:8]) != b"J3D2bdl4":
        return len(buf)
    seccnt = _st.unpack_from(">I", buf, 12)[0]
    off = 0x20
    mo = ms = None
    for _ in range(seccnt):
        tag = bytes(buf[off:off + 4])
        ss = _st.unpack_from(">I", buf, off + 4)[0]
        if tag == b"MDL3":
            mo, ms = off, ss
            break
        off += ss
    if mo is None:
        buf[0:8] = b"J3D2bmd3"
        return len(buf)
    tail = buf[mo + ms:]
    new = len(buf) - ms
    buf[mo:mo + len(tail)] = tail
    del buf[new:]
    buf[0:8] = b"J3D2bmd3"
    _st.pack_into(">I", buf, 8, new)
    _st.pack_into(">I", buf, 12, seccnt - 1)
    return new

_DISC = None


def _disc():
    global _DISC
    if _DISC is None:
        pth = os.environ.get("WW_ISO", "")
        if not pth:
            try:
                cfg = os.path.expandvars(r"%APPDATA%\TwilitRealm\Dusklight\config.json")
                pth = _json.load(open(cfg, encoding="utf-8")).get("backend.extraIsoPath", "")
            except OSError:
                pth = ""
        _DISC = False
        if pth and Path(pth).is_file():
            import ww_disc
            f, boot = ww_disc.iso_open(pth, ww_disc.WW_IDS)
            idx = {}
            for fp, off, ln in ww_disc.fst_walk(f, boot):
                idx.setdefault(fp.rsplit("/", 1)[-1].lower(), []).append((fp, off, ln))
            _DISC = (f, idx)
    return _DISC or None


def disc_adapted_member(arc_name, member, stage_scope=None):
    """(adapted_bytes, disc_path) — every disc arc candidate named <arc>.arc,
    member extracted, run through the dump hook's own transform. Ambiguous
    names (Room0.arc exists per stage) return ALL candidates; the caller
    matches by bytes — identity beats guessing (the §772 lesson)."""
    d = _disc()
    if not d:
        return []
    f, idx = d
    out = []
    import ww_disc
    # receiver->donor arc-name alias (rooms/stages dump under receiver names).
    # Serve-name aliases = the mod's kTpCollisionAlias roster (donor arcs whose
    # names collide with load-bearing TP arcs, remounted under Ww* — bytes
    # verbatim): WwAlways/WwKkiba00 (tale §803-era), WwLink (tale §810 Ruling 2,
    # Aryll's telescope re-expression).
    SERVE_ALIASES = {"wwalways": "Always", "wwkkiba00": "Kkiba_00", "wwlink": "Link"}
    names = [arc_name + ".arc"]
    sa = SERVE_ALIASES.get(arc_name.lower())
    if sa:
        names.append(sa + ".arc")
    m = re.match(r"R(\d+)_00$", arc_name)
    if m:
        names.append("Room%d.arc" % int(m.group(1)))
    if arc_name.lower() in ("stg_00", "stage"):
        names.append("Stage.arc")
    cands_list = []
    for nm in names:
        cands_list += idx.get(nm.lower(), [])
    if stage_scope:
        cands_list = [c for c in cands_list
                      if c[0].startswith("res/Stage/%s/" % stage_scope)]
    for fp, off, ln in cands_list:
        f.seek(off)
        raw = f.read(ln)
        try:
            # §839 Finding 2 closure: by-index dumps (the §811 acquirer names
            # members "idx#N") have no member NAME to match. Identity beats
            # guessing (§772): compare against EVERY J3D member of the arc and
            # let the bytes name the match — the report carries which member.
            by_index = member.startswith("idx#")
            for nm, data in ww_disc.rarc_list(raw):
                if by_index or nm.lower() == member.lower():
                    if data[:4] == b"Yaz0":
                        data = ww_disc.yaz0_decompress(data)
                    if by_index and bytes(data[:4]) != b"J3D2":
                        continue
                    buf = bytearray(data)
                    strip_mdl3(buf)
                    out.append((bytes(buf), "%s:%s" % (fp, nm) if by_index else fp))
        except ValueError:
            continue
    return out


# Ls regress row (tale §807): the staged copy carries a known 4-byte
# telescope-era edit vs the disc — tracked here every run until History rules
# re-express-or-retire. GONE would also be a finding.
# Ls regress row RETIRED per History's ruling (tale §810, Ruling 1): the staged
# 4-byte ls.bdl edit (and lshand.bdl @0x1BF2, same class) was the offline
# normalize bake — no donor behavior to re-express, staged copies superseded.
# The dict + REGRESS machinery stay for future known-edit entries.
KNOWN_EDITS = {}


def find_staged_arc(arc_name, mod=None):
    """Offline-adapted reference arc, retirement-aware (lib_ prefix included)."""
    mod = mod or MOD
    for root in REF_ROOTS:
        for cand in (arc_name + ".arc", "lib_" + arc_name + ".arc"):
            p = mod / root / cand
            if p.is_file():
                return p
    return None


def member_bytes(arc_path, member):
    raw = arc_path.read_bytes()
    for nm, data in rarc_list(raw):
        if nm.lower() == member.lower():
            if data[:4] == b"Yaz0":
                data = yaz0_decompress(data)
            return data
    return None


def first_divergence(a, b):
    n = min(len(a), len(b))
    for i in range(n):
        if a[i] != b[i]:
            return i
    return n if len(a) != len(b) else -1


def run(dump_dir, mod=None):
    dump_dir = Path(dump_dir)
    dumps = sorted(dump_dir.glob("*__*")) if dump_dir.is_dir() else []
    if not dumps:
        print("L2C: dump dir empty or absent (%s)" % dump_dir)
        print("  UNTESTED is not green: run the game with DUSK_L2C_DUMP=1 first")
        print("  (Engine hook spec: tale §792). Exit 2.")
        return 2

    equiv, differ, noref = [], [], []
    print("L2C equivalence — runtime dump vs offline-staged reference\n")
    for d in dumps:
        # §839 Finding 1: positional dump names collide across stages (Ojhous
        # Room1 vs Ojhous2 Room1 both dumped as R01_00__*). The hook's queued
        # fix stage-qualifies positional arcs: <Stage>__<Arc>__<member>. Both
        # shapes accepted; a stage prefix scopes the disc lookup directly
        # instead of identity-scanning every same-named candidate.
        stage_scope = None
        arc, _, member = d.name.partition("__")
        if "__" in member:
            maybe_arc, _, maybe_member = member.partition("__")
            if re.match(r"R\d+_00$|Stg_00$", maybe_arc):
                stage_scope, arc, member = arc, maybe_arc, maybe_member
        got = d.read_bytes()
        # v2: DISC-FIRST. Any byte-identical adapted disc candidate = EQUIV.
        cands = disc_adapted_member(arc, member, stage_scope)
        hit = next((fp for want, fp in cands
                    if hashlib.sha256(got).digest() == hashlib.sha256(want).digest()), None)
        if hit:
            equiv.append(d.name)
            print("  [EQUIV  ] %-40s %d bytes == disc %s (adapted)" % (d.name, len(got), hit))
            continue
        if cands:
            want, fp = min(cands, key=lambda c: abs(len(c[0]) - len(got)))
            off = first_divergence(got, want)
            differ.append(d.name)
            print("  [DIFFER ] %-40s vs disc %s (adapted): runtime %d B, ref %d B — first @0x%X"
                  % (d.name, fp, len(got), len(want), off))
            continue
        # fallback: bake-era staged reference, LOUDLY labeled as receipts-not-truth
        ref_arc = find_staged_arc(arc, mod)
        want = member_bytes(ref_arc, member) if ref_arc else None
        if want is None:
            noref.append(d.name)
            print("  [NO-REF ] %-40s not on disc, no staged fallback" % d.name)
            continue
        if hashlib.sha256(got).digest() == hashlib.sha256(want).digest():
            equiv.append(d.name)
            print("  [EQUIV* ] %-40s %d bytes == STAGED %s (bake-era receipt, not disc truth)"
                  % (d.name, len(got), ref_arc.parent.name))
        else:
            off = first_divergence(got, want)
            differ.append(d.name)
            print("  [DIFFER*] %-40s vs STAGED (receipt): %d B vs %d B — first @0x%X"
                  % (d.name, len(got), len(want), off))

    print("\n  %d EQUIV · %d DIFFER · %d NO-REFERENCE of %d dumped"
          % (len(equiv), len(differ), len(noref), len(dumps)))
    for (arcn, member), why in KNOWN_EDITS.items():
        sa = find_staged_arc(arcn, mod)
        cands = disc_adapted_member(arcn, member)
        if sa and cands:
            staged = member_bytes(sa, member)
            n_diff = sum(1 for x, y in zip(staged or b"", cands[0][0]) if x != y)
            if staged and n_diff:
                print("  [REGRESS] %s/%s: KNOWN-EDIT present in staged copy (%d bytes) — %s"
                      % (arcn, member, n_diff, why))
            else:
                print("  [REGRESS] %s/%s: KNOWN-EDIT GONE — closes only by History's ruling"
                      % (arcn, member))
    if differ:
        print("  DIFFER list is the defect worklist — the runtime path does NOT")
        print("  reproduce the offline adaptation for these. Exit 1.")
        return 1
    if noref and not equiv:
        print("  nothing compared — coverage is zero, not clean. Exit 2.")
        return 2
    print("  EQUIV set = per-arc generality evidence toward releasing the 538 hold.")
    return 0


def selftest():
    # §695 #4 law: prove EQUIV, DIFFER, NO-REF and the empty-dir contract.
    import struct
    import tempfile
    d = Path(tempfile.mkdtemp())
    (d / "arcs").mkdir(parents=True)
    (d / "dump").mkdir()

    # Synthetic RARC built to ww_disc.rarc_list's EXACT read offsets:
    #   data_off = u32@0xC + 0x20 · (_,_,num_ents,ent_tbl) = ">IIII"@0x20
    #   str_tbl = u32@0x34 + 0x20 · entries at ent_tbl+0x20, stride 0x14
    #   entry: etype=e[4] (0x02=dir, skipped) · name_off=u16@6 · d_o,d_l=u32@8,12
    payload = b"MODELDATA"
    arc = bytearray(0x60)
    arc[0:4] = b"RARC"
    arc[0xC:0x10] = struct.pack(">I", 0x40)      # data at 0x40+0x20 = 0x60
    arc[0x20:0x30] = struct.pack(">IIII", 0, 0, 1, 0x20)   # 1 entry @0x20+0x20=0x40
    arc[0x34:0x38] = struct.pack(">I", 0x34)     # strings at 0x34+0x20 = 0x54
    e = bytearray(0x14)
    e[4] = 0x11                                   # file, not directory(0x02)
    e[6:8] = struct.pack(">H", 0)                 # name_off 0
    e[8:16] = struct.pack(">II", 0, len(payload)) # d_o, d_l
    arc[0x40:0x54] = e
    arc[0x54:0x5A] = b"m.bdl\x00"
    arc = bytes(arc) + payload
    (d / "arcs" / "TestA.arc").write_bytes(arc)

    ok = True
    # EQUIV
    (d / "dump" / "TestA__m.bdl").write_bytes(payload)
    ok &= run(d / "dump", mod=d) == 0
    # DIFFER
    (d / "dump" / "TestA__m.bdl").write_bytes(b"MODELDXTA")
    ok &= run(d / "dump", mod=d) == 1
    # NO-REF
    (d / "dump" / "TestA__m.bdl").unlink()
    (d / "dump" / "Nope__x.bdl").write_bytes(b"z")
    ok &= run(d / "dump", mod=d) == 2
    # empty
    (d / "dump" / "Nope__x.bdl").unlink()
    ok &= run(d / "dump", mod=d) == 2
    print("\nselftest:", "OK" if ok else "BROKEN")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    if "--selftest" in sys.argv:
        raise SystemExit(selftest())
    dump = DUMP
    if "--dump" in sys.argv:
        dump = Path(sys.argv[sys.argv.index("--dump") + 1])
    raise SystemExit(run(dump))
