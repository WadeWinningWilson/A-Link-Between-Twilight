#!/usr/bin/env python3
# ============================================================================
# res_consumer_census.py — DOES THIS CONSUMER EXPECT A PARSED OBJECT OR RAW
# BYTES?
#
# THE QUESTION THIS ANSWERS, and it is not the one we started with. Foundry
# measured that `getRes` returns vtable-bearing objects for every `.bmd` and
# `.btk` (27 of 41 non-null lookups) and read it as a silent-non-draw defect.
# Housing/Engine rooted it at `d_resorce.cpp:455` / `:556`: those node types are
# PARSED AT LOAD TIME BY DESIGN — `loaderBasicBmd` and `J3DAnmLoaderDataBase::
# load` REPLACE the raw buffer before it is ever stored. The vtable IS the
# storage contract, not a stub.
#
# **So the measurement was right and the interpretation inherited an unexamined
# premise: that the slot holds file bytes for these types.** It does not, and
# `J3D2` is consumed by the loader and never reaches the slot.
#
# WHICH LEAVES EXACTLY ONE WAY THE ROOTING COULD STILL BE WRONG, and this tool
# exists to find it: **a consumer that expects RAW BYTES from a node type the
# loader PARSES.** That consumer reads a vtable pointer as file data — the
# original defect, relocated from producer to consumer. Housing/Engine asked
# for it in the words that make it worth building: *"That test kills my answer
# if my answer is wrong, which is why it is the right one to build."*
#
# ── v2, AND WHY v1'S "1,280 UNCAST" WAS THE TOOL AND NOT THE CODEBASE ───────
# v1 required the cast to sit immediately after an `=`. Most real sites do not
# look like that: the cast rides INSIDE a call argument
# (`mpBgW->Set((cBgD_t*)dComIfG_getObjectRes(...))`) or lands on a CONTINUATION
# LINE of a wrapped statement. Those 811 were never uncast — they were
# UNMATCHED. **The tool reported the limit of its own regex as a property of
# the codebase.** That is the third time this estate has caught that exact
# shape in one session, and it was visible only by reading the sample instead
# of trusting the bucket.
#
# LIMITS, stated because this lane has shipped unlabelled ones before:
#   · It classifies the CAST, not the node type of the resource fetched. A raw
#     cast is a CANDIDATE, not a defect: `u8*` on a genuinely raw node type
#     (`.dat`, `.bti`, `.dzb`) is CORRECT and expected.
#   · `enclosing-return` is an INFERENCE — a signature match, not a parse. It
#     is reported separately so it can be weighed differently from a cast.
#   · Text-level. A cast split across lines BY the formatter is still missed.
#
# Usage:
#   res_consumer_census.py            — the census
#   res_consumer_census.py --control  — prove it can produce noise, then exit
# Exit 0 census ran · 1 control FAILED (tool cannot be trusted) · 2 bad input.
# ============================================================================
import re
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
SRC = REPO / "src"

ACCESSORS = ("dComIfG_getObjectRes", "dComIfG_getStageRes", "getRes")
ACC = "|".join(ACCESSORS)

# A cast to any of these means the caller expects the LOADER'S OUTPUT — a
# constructed C++ object. These are the types d_resorce.cpp actually produces.
PARSED = (
    "J3DModelData", "J3DAnmTransform", "J3DAnmTextureSRTKey", "J3DAnmTevRegKey",
    "J3DAnmTexPattern", "J3DAnmColor", "J3DAnmVtxColor", "J3DAnmVisibilityFull",
    "J3DAnmCluster", "J3DAnmBase",
    # Discovered from the tool's own UNRECOGNISED-TYPE report rather than from
    # memory: BCK/BCKS -> J3DAnmTransformKey, BMDG -> J3DDeformData. Both node
    # types ARE in d_resorce.cpp's 22-entry parse dispatch.
    "J3DAnmTransformKey", "J3DDeformData",
)
# ============================================================================
# A FIFTH BUCKET — CLASS CAST ON A NODE TYPE THE LOADER DOES **NOT** PARSE.
#
# This is the defect shape the tool was built for, at the OPPOSITE POLARITY to
# the one predicted. I hunted "raw cast on a PARSED type"; there are none. What
# exists is "CLASS cast on a RAW type."
#
# `d_resorce.cpp` parses 22 node types and **`'BMT '` IS NOT ONE OF THEM**, so
# a `.bmt` stays raw in the slot. `J3DModelLoaderDataBase::loadMaterialTable()`
# is the call that turns those bytes into a `J3DMaterialTable*`, and its ONLY
# callers are `d_ext_npc_mount.cpp:2176/2180`. Three other sites cast a getRes
# result straight to `J3DMaterialTable*` with no parse in between.
#
# **CANDIDATE, NOT DEFECT** — and the distinction is load-bearing here, because
# two of the three are vanilla TP paths that presumably work. Either BMT is
# parsed somewhere this lane has not found, or the type is layout-compatible
# with the raw bytes where they touch it, or it is latent upstream. Rooting
# belongs to the owning lane; this tool's job was to make it visible.
# ============================================================================
CLASS_ON_RAW = ("J3DMaterialTable",)
# ============================================================================
# A THIRD BUCKET, ADDED AFTER THE FIRST RUN CLASSIFIED IT WRONG.
#
# `ResTIMG` and `cBgD_t` are NOT loader outputs — they are STRUCT OVERLAYS ON A
# RAW BUFFER. `d_resorce.cpp` has no `TIMG`/`BTI ` node type in its parse
# dispatch, and `d_resorce.cpp:155` casts a raw fetch straight to `ResTIMG*`.
#
# Counting them as PARSED overstated that bucket by 79 and, worse, would have
# let a genuine raw-vs-parsed mismatch on a TIMG hide inside the "expects the
# loader's output" column. **An overlay cast is a RAW reading with a typed name
# on it.**
# ============================================================================
OVERLAY = ("cBgD_t", "ResTIMG", "JUTTexture",
           # .dat overlays, discovered the same way. Their files
           # (pol_efftbl.dat, pol_effcol.dat, ...) appear as RAW
           # data in the boot log, which corroborates the bucket.
           "dKy_pol_efftbl_struct", "dKy_pol_effcol_struct",
           "dKy_pol_sound_struct", "dKy_pol_arg_struct",
           "dKy_color_data_struct", "dMap_prm_res_s")
# ============================================================================
# A FOURTH BUCKET — `void*` IS NOT A BYTE READING.
#
# v2 put `void` in RAW and the RAW count went 32 -> 132. Reading the new
# entries, nearly all are `void* objRes = dComIfG_getObjectRes(...)`: an
# OPAQUE HANDLE that is passed on or cast later. **`u8*`/`char*` ASSERT that
# the slot holds bytes; `void*` asserts NOTHING and cannot misread a vtable.**
#
# Keeping them together would have buried the ~32 real byte-readers under 100
# handles and let the headline number read as a hundred candidate defects.
# **A bucket that grows 4x when you widen the net is usually the net, not the
# finding.**
# ============================================================================
OPAQUE = ("void",)
# A cast to these means the caller expects BYTES. On a parsed node type that is
# the defect; on a raw node type it is correct.
RAW = ("u8", "s8", "char", "int", "u16", "s16", "u32", "s32", "f32")

RX_ANY = re.compile(r"\b(" + ACC + r")\s*\(")
COMMENTY = re.compile(r"^\s*(?://|\*|/\*)")
# The accessors' OWN DEFINITIONS are not call sites. Three of the final
# fourteen "unresolved" were `void* dRes_control_c::getRes(...) {` -- the
# function being censused, counted as a consumer of itself.
DEFN = re.compile(r"^\s*(?:static\s+)?[A-Za-z_][\w:]*\s*\*?\s*(?:" + ACC
                  + r")\s*\([^)]*(?:\)\s*\{?\s*$|,\s*$)")
# ============================================================================
# v3 — C++-STYLE CASTS WERE INVISIBLE, AND THAT WAS MOST OF THE REMAINING 273.
#
# v2 matched only the C-style `(T*)acc(...)`. This codebase also writes
# `static_cast<J3DModelData*>(dComIfG_getObjectRes(...))` — and those were
# landing in UNRESOLVED while being the single most explicit statement of
# intent a call site can make. **The most type-safe sites were the ones the
# type census could not read.**
#
# Same lesson a third time, so it is worth naming plainly: every round of this
# tool, the unresolved bucket has been MY PATTERN'S BLIND SPOT rather than the
# codebase's ambiguity. The bucket is not a measurement of the code until the
# patterns stop growing.
# ============================================================================
CAST_ANY = re.compile(
    r"(?:\(\s*(?:const\s+)?([A-Za-z_][A-Za-z0-9_]*)\s*\*+\s*\)"
    r"|(?:static_cast|reinterpret_cast|const_cast)\s*<\s*(?:const\s+)?"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*\*+\s*>)\s*\(?\s*(?:" + ACC + r")\s*\(")
# A pointer-identity comparison reads no CONTENT — it cannot misread a vtable.
COMPARE = re.compile(r"(?:==|!=)\s*(?:" + ACC + r")\s*\(|(?:" + ACC
                     + r")\s*\([^;]*\)\s*(?:==|!=)")
DECL = re.compile(
    r"^\s*(?:const\s+)?([A-Za-z_][A-Za-z0-9_:]*)\s*\*+\s*[A-Za-z_]\w*\s*=\s*[^;]*?(?:"
    + ACC + r")\s*\(")
NULLCHECK = re.compile(
    r"(?:" + ACC + r")\s*\([^;]*\)\s*(?:==|!=)\s*(?:NULL|nullptr|0)\b"
    r"|(?:if|while)\s*\(\s*!?\s*(?:" + ACC + r")\s*\(")
RETURNS = re.compile(r"^\s*return\b[^;]*?(?:" + ACC + r")\s*\(")
FUNCSIG = re.compile(
    r"^(?:static\s+|inline\s+|virtual\s+)*([A-Za-z_][A-Za-z0-9_:]*)\s*\*+\s*"
    r"[A-Za-z_][A-Za-z0-9_:]*\s*\(")


def classify_type(ctype):
    if ctype in PARSED:
        return "PARSED"
    if ctype in OVERLAY:
        return "OVERLAY"
    if ctype in OPAQUE:
        return "OPAQUE"
    if ctype in RAW:
        return "RAW"
    if ctype in CLASS_ON_RAW:
        return "CLASS_ON_RAW"
    return None


def enclosing_return_type(lines, idx):
    """Walk upward for the enclosing function signature. Best effort, and it is
    LABELLED as inference wherever used — a signature match is not a parse."""
    for j in range(idx, max(-1, idx - 400), -1):
        m = FUNCSIG.match(lines[j])
        if m:
            return m.group(1)
    return None


BUCKETS = ("PARSED", "OVERLAY", "OPAQUE", "RAW", "CLASS_ON_RAW",
           "NULLCHECK", "UNRESOLVED")


# ============================================================================
# v4 — TWO RESOLVERS THAT FINISH THE JOB, both predicted by the pattern.
#
# After v3, 85 remained: 50 `modelData = dComIfG_getObjectRes(...)` with the
# cast absent because the VARIABLE carries the type, and 35 where the cast sits
# on an ADJACENT LINE because the statement wraps. Neither is ambiguity in the
# code — both are line-at-a-time reading.
#
#   · STATEMENT JOINING. A logical statement, not a line, is the unit a cast
#     belongs to. Walk back to the previous `;{}` and forward to the next `;`,
#     then match on the whole thing.
#   · DECLARATION LOOKUP. For `var = acc(...)` with no cast, search backward in
#     the file for `Type* var`. **Labelled `var-decl (INFERRED)`** — nearest
#     preceding declaration is a heuristic, not a scope analysis, and a shadowed
#     name would fool it.
# ============================================================================
VARDECL_TMPL = r"\b([A-Za-z_][A-Za-z0-9_:]*)\s*\*+\s*%s\s*(?:[;,=)]|$)"
ASSIGN_NOCAST = re.compile(r"^\s*([A-Za-z_][\w\.\->\[\]]*?)\s*=\s*(?:" + ACC + r")\s*\(")


def logical_statement(lines, idx):
    """The whole statement around lines[idx] — a cast belongs to a STATEMENT,
    not to a line."""
    start = idx
    while start > 0 and not re.search(r"[;{}]\s*$", lines[start - 1]):
        start -= 1
        if idx - start > 12:
            break
    end = idx
    while end < len(lines) - 1 and not re.search(r";\s*$", lines[end]):
        end += 1
        if end - idx > 12:
            break
    return " ".join(l.strip() for l in lines[start:end + 1])


def find_var_type(lines, idx, var):
    """Nearest preceding declaration of `var`. INFERENCE, not scope analysis."""
    base = var.split("->")[-1].split(".")[-1].split("[")[0]
    if not base or not re.match(r"^[A-Za-z_]\w*$", base):
        return None
    rx = re.compile(VARDECL_TMPL % re.escape(base))
    for j in range(idx, max(-1, idx - 300), -1):
        m = rx.search(lines[j])
        if m:
            return m.group(1)
    return None


def scan(root):
    out = {k: [] for k in BUCKETS}
    for f in sorted(root.rglob("*.cpp")) + sorted(root.rglob("*.c")):
        try:
            lines = f.read_text(encoding="utf-8", errors="replace").splitlines()
        except OSError:
            continue
        rel = f.relative_to(REPO).as_posix()
        for n, line in enumerate(lines, 1):
            if not RX_ANY.search(line) or COMMENTY.match(line) or DEFN.match(line):
                continue
            txt = line.strip()[:110]
            stmt = logical_statement(lines, n - 1)

            m = CAST_ANY.search(line) or CAST_ANY.search(stmt)
            if m:
                ct = m.group(1) or m.group(2)
                how = "cast" if CAST_ANY.search(line) else "cast (joined stmt)"
                out[classify_type(ct) or "UNRESOLVED"].append((rel, n, ct, how, txt))
                continue
            m = DECL.match(line) or DECL.match(stmt)
            if m:
                out[classify_type(m.group(1)) or "UNRESOLVED"].append(
                    (rel, n, m.group(1), "declared-type", txt))
                continue
            if COMPARE.search(line) or NULLCHECK.search(line):
                out["NULLCHECK"].append((rel, n, "-", "compare/null-check", txt))
                continue
            m = ASSIGN_NOCAST.match(line)
            if m:
                vt = find_var_type(lines, n - 1, m.group(1))
                if vt:
                    out[classify_type(vt) or "UNRESOLVED"].append(
                        (rel, n, vt, "var-decl (INFERRED)", txt))
                    continue
            if RETURNS.match(line):
                rt = enclosing_return_type(lines, n - 1)
                b = (classify_type(rt) if rt else None) or "UNRESOLVED"
                out[b].append((rel, n, rt or "?", "enclosing-return (INFERRED)", txt))
                continue
            out["UNRESOLVED"].append((rel, n, "?", "unresolved", txt))
    return out


def control():
    """Prove the detector produces NOISE before its silence is read as a result.

    TWO specimens, deliberately: one assigned, one as an INLINE CALL ARGUMENT.
    v1 caught only the assigned shape, and that gap is exactly what produced
    the false '1,280 uncast'. A control that only tests the shape you already
    handle cannot catch the shape you missed."""
    tmp = SRC / "d" / "_res_census_control_specimen.cpp"
    tmp.write_text(
        'void f(void) {\n'
        '    u8* p = (u8*)dComIfG_getObjectRes("CTRL", 3);\n'
        '    g((u8*)dComIfG_getObjectRes("CTRL_ARG", 4));\n'
        '}\n', encoding="utf-8")
    try:
        out = scan(SRC)
        hits = [r for r in out["RAW"] if "_res_census_control_specimen" in r[0]]
        print("NEGATIVE CONTROL - planting RAW consumers (assigned AND inline-arg)")
        print("  specimen: %s" % tmp.relative_to(REPO).as_posix())
        if len(hits) == 2:
            print("  CONTROL PASSED - both flagged RAW. v1 would have caught only")
            print("  the assigned one; that gap WAS the bug this version fixes.")
            return 0
        print("  *** CONTROL FAILED - %d/2 flagged. Do not trust this run. ***"
              % len(hits))
        return 1
    finally:
        tmp.unlink(missing_ok=True)


def main():
    if "--control" in sys.argv:
        return control()
    if not SRC.is_dir():
        print("no src/ at %s" % SRC)
        return 2
    out = scan(SRC)
    total = sum(len(v) for v in out.values())
    print("RES CONSUMER CENSUS - what does each caller expect back?")
    print("  call sites scanned : %d" % total)
    for b in BUCKETS:
        print("    %-11s %5d   (%4.1f%%)" % (b, len(out[b]), 100.0 * len(out[b]) / total))
    print()
    how = {}
    for b in ("PARSED", "OVERLAY", "OPAQUE", "RAW", "CLASS_ON_RAW"):
        for _, _, _, h, _ in out[b]:
            how[h] = how.get(h, 0) + 1
    print("  HOW each typed site was resolved - a cast is EVIDENCE, an")
    print("  enclosing return type is an INFERENCE:")
    for h, n in sorted(how.items(), key=lambda kv: -kv[1]):
        print("    %-30s %d" % (h, n))
    unknown = {}
    for rel, n, ct, h, txt in out["UNRESOLVED"]:
        if ct and ct not in ("?",):
            unknown[ct] = unknown.get(ct, 0) + 1
    if unknown:
        print()
        print("  UNRECOGNISED TYPE NAMES seen at resolved-but-unclassified sites")
        print("  (the type list should come from THIS, not from memory):")
        for ct, n in sorted(unknown.items(), key=lambda kv: -kv[1]):
            print("    %-30s %d" % (ct, n))
    print()
    print("  *** CLASS-ON-RAW - a class cast on a node type d_resorce.cpp does")
    print("      NOT parse. CANDIDATE, not defect - see the header. ***")
    for rel, n, ct, h, txt in out["CLASS_ON_RAW"]:
        print("    %s:%d  (%s*) %s" % (rel, n, ct, txt[:70]))
    print()
    print("  *** RAW-EXPECTING SITES - the only ones that can carry the defect.")
    print("      A raw cast on a genuinely raw node type is CORRECT. ***")
    for rel, n, ctype, h, txt in out["RAW"]:
        print("    %s:%d  (%s*) [%s]" % (rel, n, ctype, h))
    print()
    print("  *** STILL UNRESOLVED - enumerated in FULL, never truncated. ***")
    for rel, n, ctype, h, txt in out["UNRESOLVED"]:
        print("    %s:%d  %s" % (rel, n, txt[:88]))
    return 0


if __name__ == "__main__":
    sys.exit(main())
