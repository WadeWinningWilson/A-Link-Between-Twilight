# ============================================================================
# catchall_procmap.py — FOUNDRY, phase 2 of the catch-all census.
#
# Phase 1 measured: 87 distinct names in Outset, only 16 with a same-named arc.
# That looked like an 85% failure. This phase tests whether that framing is
# even the right one, by asking the donor's OWN dispatch table what a name is:
#
#   dStage_objectNameInf = { char name[8]; s16 procname; s8 argument; s8 gbaName }
#
# There is no arc field — a name maps to a PROCNAME. And procnames are SHARED
# (kusax1/kusax7/flower/swood/... are all one fpcNm_GRASS_e). So the port cost
# scales with DISTINCT PROCS, not with names, and the 87 may collapse hard.
#
# Reads the table from donor source (the artifact), never a remembered list.
# ============================================================================
import re
import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path

sys.stdout.reconfigure(encoding="utf-8", errors="replace")
HERE = Path(__file__).parent
sys.path.insert(0, str(HERE))
import scls_scan as S

SRC = Path(r"D:/XXXXXXX/WW DP/src")
OBJ = Path(r"D:/XXXXXXX/Ex WW/files/res/Object")

# ---- 1. the 87 names, re-derived from the room (same method as phase 1) ----
raw = S.yaz0((S.ROOT / "sea" / "Room44.arc").read_bytes())
dzr = next(b for nm, b in S.rarc_members(raw) if nm.lower() == "room.dzr")

names = Counter()
nchunk = struct.unpack_from(">I", dzr, 0)[0]
for c in range(nchunk):
    tag = dzr[4 + c * 12: 8 + c * 12].decode("ascii", "replace")
    cnt = struct.unpack_from(">I", dzr, 8 + c * 12)[0]
    off = struct.unpack_from(">I", dzr, 12 + c * 12)[0]
    stride = 0x20 if tag.startswith("ACT") else 0x24 if tag.startswith("SCO") else 0
    if not stride:
        continue
    for e in range(cnt):
        eo = off + e * stride
        if eo + 8 > len(dzr):
            break
        nm8 = dzr[eo:eo + 8].split(b"\0")[0].decode("ascii", "replace").strip()
        if nm8:
            names[nm8] += 1

# ---- 2. the donor's own name -> procname table ----------------------------
# OBJNAME("kusax1", fpcNm_GRASS_e, 255, 0)
stage = (SRC / "d" / "d_stage.cpp").read_text(encoding="utf-8", errors="replace")
block = stage[stage.index("l_objectName[] = {"):]
block = block[:block.index("\n};")]
OBJNAME = re.compile(r'OBJNAME\(\s*"([^"]*)"\s*,\s*([A-Za-z0-9_]+)\s*,')
name2proc = {}
for m in OBJNAME.finditer(block):
    name2proc[m.group(1)] = m.group(2)
print("l_objectName rows parsed: %d" % len(name2proc))

# §P3: match broadly, validate against the declared domain, surface loudly.
unrecognised = [n for n in names if n not in name2proc]
if unrecognised:
    print("!! names NOT in l_objectName (%d): %s"
          % (len(unrecognised), " ".join(sorted(unrecognised))))

# ---- 3. the collapse -------------------------------------------------------
proc2names = defaultdict(list)
for n in names:
    proc2names[name2proc.get(n, "<ABSENT>")].append(n)

print()
print("distinct NAMES placed     : %d  (%d placements)"
      % (len(names), sum(names.values())))
print("distinct PROCS behind them: %d" % len(proc2names))
print()

# ---- 4. does each proc load an arc at all? --------------------------------
# A proc with no arcname anywhere in its TU is a LOGIC TAG: "no arc" is the
# CORRECT outcome and the catch-all's inert no-op already serves it.
# Found by scanning every actor TU for its profile's fpcNm and its arcname
# strings — discovered from source, not from a name-guessing convention.
# ---------------------------------------------------------------------------
# WITHDRAWN AND REWRITTEN: the first cut of these patterns required `&\w+` as
# resLoad's first argument and so missed `dComIfG_resLoad(&i_this->mPhase,
# "Kamome")` — every actor using an indirected phase read as a logic TAG.
# It reported fpcNm_KAMOME_e / fpcNm_BB_e as arcless while phase 1 had already
# opened Kamome.arc (9 bck) and Bb.arc (14 bck) off the disc. THE SILENCE WAS
# MY REGEX'S, NOT THE DONOR'S — the same failure mode as reading a packed float
# instead of the file's bytes. Validated below against known-arc actors so the
# detector cannot go quiet again without the run failing loudly.
#
# `dComIfG_getObjectRes("Kamome", ...)` is the STRONGEST signal: it names the
# arc as a literal at every use site, not just at load.
# ---------------------------------------------------------------------------
# NO \b HERE: the donor writes `const char daLwood_c::m_arcname[6] = "Lwood";`
# and `_` is a word character, so `\barcname` never matches `m_arcname`. The
# control caught exactly this on the second pass.
ARCNAME = re.compile(r'arc_?name\w*\s*(?:\[[^\]]*\])?\s*=\s*"([^"]+)"', re.I)
RESLOAD = re.compile(r'dComIfG_res(?:Load|Delete)\s*\([^,()]*(?:\([^()]*\))?[^,()]*,\s*"([^"]+)"')
GETRES = re.compile(r'dComIfG_getObjectRes\s*\(\s*"([^"]+)"')
PHASEREQ = re.compile(r'dComIfG_PhaseHandler\w*\s*\([^,]*,\s*"([^"]+)"')

# Every arc that actually ships, by lowercase stem — the declared domain the
# extracted literals are validated against.
DISC_ARCS = {p.stem.lower() for p in OBJ.iterdir() if p.suffix.lower() == ".arc"}
print("arcs on disc in res/Object: %d" % len(DISC_ARCS))

proc_src = {}
proc_arcs = defaultdict(set)
proc_batched = set()
for f in (SRC / "d" / "actor").rglob("d_a_*.cpp"):
    try:
        t = f.read_text(encoding="utf-8", errors="replace")
    except Exception:
        continue
    # CLASS C detection: the actor is VISIBLE but owns no arc because a shared
    # engine subsystem draws it in batch. d_grass draws through GXLoadPosMtxImm
    # with its own vertex data — no arc, no J3D model, nothing the catch-all's
    # resLoad->J3D2->draw body can consume. Treating these as logic tags would
    # score an EMPTY ISLAND as a correct no-op.
    batched = bool(re.search(r'#include\s+"d/d_(grass|flower|tree)\.h"', t))
    procs = set(re.findall(r'\bfpcNm_[A-Za-z0-9_]+', t))
    if batched:
        for p in procs:
            proc_batched.add(p)
    arcs = (set(ARCNAME.findall(t)) | set(RESLOAD.findall(t))
            | set(GETRES.findall(t)) | set(PHASEREQ.findall(t)))
    # An arc literal must actually exist on the disc, or it is not an arc name
    # — this is the §P3 validation step, and it keeps stray string literals
    # (message ids, bmd member names) from masquerading as arcs.
    arcs = {a for a in arcs if a.lower() in DISC_ARCS}
    for p in procs:
        proc_src.setdefault(p, []).append(f.name)
        proc_arcs[p] |= arcs

# ---- 4a. NEGATIVE CONTROL on the detector itself --------------------------
# The vacuous-pass doctrine, applied to my own instrument: these procs are
# KNOWN to load an arc because phase 1 opened those arcs off the disc and
# counted their members. If the detector calls any of them arcless it has gone
# silent again, and every "TAG" verdict in this run is worthless. Fail LOUD
# rather than publish a second set of numbers built on regex silence.
CONTROL = {
    "fpcNm_KAMOME_e": "Kamome",   # phase 1: 2 model, 9 bck
    "fpcNm_BB_e": "Bb",           # phase 1: 2 model, 14 bck
    "fpcNm_Lwood_e": "Lwood",     # phase 1: 1 model, 1 dzb
}
broken = [p for p, want in CONTROL.items()
          if want.lower() not in {a.lower() for a in proc_arcs.get(p, set())}]
if broken:
    print()
    print("!! DETECTOR CONTROL FAILED for: %s" % " ".join(broken))
    print("!! These procs demonstrably load arcs (phase 1 read them off the")
    print("!! disc). The arc-name extractor is still blind. NOT PUBLISHING.")
    sys.exit(1)
print("detector control PASSED on %d known-arc procs" % len(CONTROL))
print()

rows = []
for proc, nms in sorted(proc2names.items(), key=lambda kv: -sum(names[n] for n in kv[1])):
    plc = sum(names[n] for n in nms)
    arcs = proc_arcs.get(proc, set())
    rows.append((proc, len(nms), plc, sorted(arcs), proc_src.get(proc, [])))

tally = defaultdict(lambda: [0, 0, 0])  # class -> [procs, names, placements]
print("%-28s %5s %5s  %-9s %s" % ("proc", "names", "plc", "class", "arc(s)"))
for proc, nn, plc, arcs, srcs in rows:
    if proc in proc_batched:
        cls, note = "C-BATCHED", "engine-drawn, no arc"
    elif arcs:
        cls, note = "A-ARC", " ".join(sorted(arcs)[:4])
    else:
        cls, note = "B-TAG", "no arc, no visual"
    t = tally[cls]
    t[0] += 1
    t[1] += nn
    t[2] += plc
    print("%-28s %5d %5d  %-9s %s" % (proc, nn, plc, cls, note))

print()
LABEL = {
    "A-ARC":     "A  ARC-BACKED    catch-all serviceable IFF given a name->arc map",
    "B-TAG":     "B  LOGIC TAG     no arc AND no visual - inert no-op is CORRECT",
    "C-BATCHED": "C  ENGINE-BATCH  visible, drawn by shared subsystem, NO arc ever",
}
for cls in ("A-ARC", "B-TAG", "C-BATCHED"):
    p, n, pl = tally[cls]
    print("%s\n     %2d procs · %2d names · %3d placements (%.0f%% of room)"
          % (LABEL[cls], p, n, pl, 100.0 * pl / sum(names.values())))

# ---- 5. Housing Q2: the .bck column, scoped to class A --------------------
# Only class A can reach the catch-all at all, so the animation question is
# only meaningful there. Arc sets are OVER-attributed (a TU naming another
# actor's proc inherits its arcs), so this is an UPPER bound on animated arcs.
seen, anim_arcs, still_arcs = set(), [], []
for proc, nn, plc, arcs, srcs in rows:
    if proc in proc_batched or not arcs:
        continue
    for a in arcs:
        if a.lower() in seen:
            continue
        seen.add(a.lower())
        p = next((q for q in OBJ.iterdir() if q.stem.lower() == a.lower()), None)
        if not p:
            continue
        try:
            d = S.yaz0(p.read_bytes())
            n = sum(1 for nm, b in S.rarc_members(d)
                    if nm.lower().endswith((".bck", ".bca")))
        except Exception:
            continue
        (anim_arcs if n else still_arcs).append((a, n))
print()
print("HOUSING Q2 - class-A arcs, animation column (upper bound):")
print("  arcs carrying .bck/.bca : %d" % len(anim_arcs))
print("  arcs with NO animation  : %d" % len(still_arcs))
print("  animated: %s" % " ".join("%s(%d)" % t for t in sorted(anim_arcs)))
