#!/usr/bin/env python3
# ============================================================================
# sig_diff.py — IS IT THE SAME FUNCTION, OR JUST THE SAME NAME?
#
# THE GAP THIS CLOSES (Housing/Engine, pre-port signature check). My
# `upstream_conformance --symbol` check answers "does this name resolve to
# exactly ONE function on both images". It said `fopAcM_GetParamBit` was SAFE,
# and that verdict was CORRECT — and useless, because:
#
#   DONOR    (WW DP/include/f_op/f_op_actor_mng.h:150)
#     inline u32 fopAcM_GetParamBit(u32 param, u8 shift, u8 bit)   <- a VALUE
#   RECEIVER (include/f_op/f_op_actor_mng.h:188)
#     inline u32 fopAcM_GetParamBit(void* ac, u8 shift, u8 bit)    <- a POINTER
#
# Same name, same arity, incompatible first parameter. A plugin binding the
# receiver's symbol would pass a small integer where an actor pointer is
# expected, and the receiver would DEREFERENCE it. **It compiles, the name
# resolves, the pre-flight says SAFE, and it wild-reads at runtime.**
#
# **A SYMBOL BEING SAFE TO RESOLVE IS NOT A SYMBOL BEING SAFE TO CALL.**
# That second question needs both headers side by side, which is what this
# does. It is the compile_gate tier-2 class (right name, wrong shape) applied
# to a plugin that has NO compile-time link to the receiver at all — nothing
# else in the stack can catch it.
#
# VERDICTS: SAME · DIVERGENT (bind is unsafe) · DONOR-ONLY / RECEIVER-ONLY /
# UNKNOWN. Absent is reported, never scored as SAME (№31-C).
#
# Usage:  sig_diff.py <symbol> [<symbol> ...]
#         sig_diff.py --actor <profile-sym>     every callable of one actor
# ============================================================================
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
DONOR = Path("D:/XXXXXXX/WW DP")
RECV_INC = REPO / "include"
RECV_SRC = REPO / "src"


def find_decls(symbol, roots):
    """EVERY declaration/definition of `symbol` under roots.

    REWRITTEN after an audit found the first version comparing CALL SITES
    against declarations. It matched `symbol(` anywhere on a line and only
    required a token before the name, so `plyrToObjVec.z * cM_ssin(yRot);`
    parsed as a declaration whose "return type" was `plyrToObjVec.z *`. That
    produced confident DIVERGENT verdicts for functions that are byte-identical
    in both trees — cM_ssin and cM_scos among them.

    A real declaration: optional storage/inline keywords, a return type, the
    name, a parameter list that CLOSES ON THIS LINE, and then `{` or `;`.
    Anchoring to start-of-line and requiring that terminator excludes call
    sites, which are embedded in expressions and never end that way.

    Returns ALL matches, not the first — the donor carries OVERLOAD SETS
    (dComIfG_getObjectIDRes has both `int id` and `u16 id`) and picking one
    arbitrarily manufactures a divergence that does not exist.

    ========================================================================
    NO EXTERNAL `grep` — 2026-08-14, found by `control.py audit`.
    This scanned with `subprocess.run(["grep", ...])`. There is no `grep` on
    PATH under PowerShell, so the call raised FileNotFoundError and the tool
    died before printing a verdict. It worked from Git Bash and nowhere else.
    **That is the compile_gate tier-2 defect with the environments swapped**
    (tier 2 runs from cmd/PowerShell and not Git Bash): a gate whose answer
    depends on which shell launched it is not a gate. Scanning in Python
    removes the dependency, and with it BOTH documented grep hazards — the
    unsupported `\b` that returned silent zero matches, and the drive-letter
    colon that mangled `path:line:text`, since nothing is re-parsed now.
    ========================================================================
    """
    decl = re.compile(
        r"^\s*(?:template\s*<[^>]*>\s*)?"
        r"(?:(?:inline|static|extern|virtual|constexpr|friend|DUSK_CONST)\s+)*"
        # a RETURN TYPE, never a control-flow keyword: `return f(x);` and
        # `else g(y);` both otherwise parse as declarations returning "return"
        # / "else", which is how a call site was cited as the receiver's
        # declaration of fopAcM_GetParamBit.
        r"(?!(?:return|if|else|while|for|switch|case|do|new|delete|throw))"
        r"(?P<ret>[A-Za-z_][\w:]*(?:\s*[*&])*)\s+"
        r"(?:[A-Za-z_]\w*::)?" + re.escape(symbol) +
        r"\s*\((?P<params>[^()]*)\)\s*(?:const\s*)?[{;]\s*$")
    needle = symbol + "("
    out = []
    for root in roots:
        root = Path(root)
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if path.suffix not in (".h", ".cpp") or not path.is_file():
                continue
            try:
                text = path.read_text(encoding="utf-8", errors="replace")
            except OSError:
                continue
            # cheap whole-file reject first: the fixed-string prefilter `grep
            # -F` used to do, minus the process spawn.
            if needle not in text:
                continue
            lines = text.splitlines()
            for i, ln in enumerate(lines):
                if needle not in ln:
                    continue
                # ============================================================
                # MULTI-LINE DECLARATIONS — fixed 2026-08-14.
                # The matcher required the parameter list to CLOSE ON THE SAME
                # LINE, so any declaration wrapped across lines was invisible.
                # `fopAcM_create` wraps in BOTH trees, so the tool reported it
                # DONOR-ONLY — "no receiver declaration found" — for the actor
                # spawn entry point the receiver plainly declares twice
                # (f_op_actor_mng.h:528 and :532).
                # **That verdict is worse than unknown: it tells a porter to
                # supply a function that already exists.** Joining continuation
                # lines until the parens balance makes the wrapped form visible.
                # Call sites that wrap are still rejected — they have no return
                # type before the name, which is what the anchor tests.
                # ============================================================
                cand = ln.rstrip()
                j = i
                while (cand.count("(") > cand.count(")")
                       and j + 1 < len(lines) and j - i < 6):
                    j += 1
                    cand = cand.rstrip() + " " + lines[j].strip()
                m = decl.match(cand)
                if not m:
                    continue
                out.append((str(path), str(i + 1), cand.strip(),
                            m.group("params")))
    return out


# Type words that must never be mistaken for a parameter NAME. Without this,
# an unnamed multi-word parameter loses its last word: `unsigned int` would
# normalise to `unsigned`. Game typedefs included because they are what these
# headers actually use.
TYPE_WORDS = {
    "void", "bool", "char", "short", "int", "long", "float", "double",
    "signed", "unsigned", "wchar_t",
    "u8", "u16", "u32", "u64", "s8", "s16", "s32", "s64",
    "f32", "f64", "BOOL", "size_t", "uintptr_t", "intptr_t",
}


def norm_params(params):
    """Parameter TYPES only — names and spacing removed.

    ========================================================================
    UNNAMED PARAMETERS AND COMMENT ARTIFACTS — fixed 2026-08-14.
    This dropped the last bare identifier to strip the parameter NAME. When a
    declaration omits the name — which the RECEIVER's headers do constantly —
    the last bare identifier IS THE TYPE, so it stripped that instead and
    returned EMPTY. Every one of the handoff's "~4 parser artifacts" is this
    single bug plus comment leakage:

        donor    float cM_rndF(float max);   -> ['float']
        receiver float cM_rndF(float);       -> ['']      ** FALSE DIVERGENT

    cM_rndF, cM_rndFX and fopAcM_setCarryNow were all reported DIVERGENT for
    this reason and are NOT divergent; dBgS_GetWaterHeight leaked `/*pos*/`
    into its type for the same lack of comment stripping. **These were false
    positives in a gate whose entire job is to stop an unsafe by-name bind** —
    the mirror of the vacuous pass: a vacuous FAILURE, which burns a porter's
    day reproducing a divergence that was never there.
    ========================================================================
    """
    params = params.strip().rstrip(")").strip()
    if params in ("", "void"):
        return []
    types = []
    depth = 0
    cur = ""
    for ch in params:
        if ch == "," and depth == 0:
            types.append(cur)
            cur = ""
            continue
        if ch in "<(":
            depth += 1
        elif ch in ">)":
            depth -= 1
        cur += ch
    if cur.strip():
        types.append(cur)
    outs = []
    for t in types:
        t = re.sub(r"/\*.*?\*/", " ", t)           # `cXyz& /*pos*/`
        t = re.sub(r"//.*$", " ", t)
        t = re.sub(r"\b(const|DUSK_CONST|register|volatile)\b", " ", t)
        t = re.sub(r"=\s*[^,]+$", "", t)           # default args
        t = t.strip()
        # Drop the trailing parameter NAME — but ONLY when there is something
        # left that can be the type. A single token is the TYPE of an unnamed
        # parameter, and a trailing type WORD (`unsigned int`) is never a name.
        if not t.endswith(("*", "&")):
            words = t.split()
            if len(words) > 1 and words[-1] not in TYPE_WORDS:
                t = " ".join(words[:-1])
        outs.append(re.sub(r"\s+", "", t))
    return outs


def verdict(symbol):
    d = find_decls(symbol, [DONOR / "include", DONOR / "src"])
    r = find_decls(symbol, [RECV_INC, RECV_SRC])
    if not d and not r:
        return "UNKNOWN", "declared in neither tree", None, None
    if not d:
        return "RECEIVER-ONLY", "no donor declaration found", None, r[0]
    if not r:
        return "DONOR-ONLY", "no receiver declaration found", d[0], None
    # OVERLOAD-AWARE. If ANY donor signature matches ANY receiver signature,
    # the donor's call has a compatible receiver target and this is not a
    # divergence. Comparing one arbitrary declaration from each side invented
    # divergences for real overload sets (dComIfG_getObjectIDRes carries both
    # `int id` and `u16 id` donor-side; the receiver keeps only `u16`).
    dsigs = [norm_params(x[3]) for x in d]
    rsigs = [norm_params(x[3]) for x in r]
    for ds in dsigs:
        if ds in rsigs:
            extra = ""
            if len(dsigs) > 1 or len(rsigs) > 1:
                extra = " (overload set: %d donor / %d receiver, a compatible "                         "pair exists)" % (len(dsigs), len(rsigs))
            return "SAME", "%d param(s) identical%s" % (len(ds), extra), d[0], r[0]
    ds, rs = dsigs[0], rsigs[0]
    detail = ("arity %d vs %d" % (len(ds), len(rs))) if len(ds) != len(rs) else (
        "param %s differs: donor %s vs receiver %s" % (
            ",".join(str(i) for i, (a, b) in enumerate(zip(ds, rs)) if a != b),
            "/".join(a for a, b in zip(ds, rs) if a != b),
            "/".join(b for a, b in zip(ds, rs) if a != b)))
    if len(dsigs) > 1 or len(rsigs) > 1:
        detail += " (NO compatible pair among %d donor / %d receiver overloads)" % (
            len(dsigs), len(rsigs))
    return "DIVERGENT", detail, d[0], r[0]


def main():
    syms = [a for a in sys.argv[1:] if not a.startswith("--")]
    if "--actor" in sys.argv:
        sys.path.insert(0, str(HERE))
        import cargo_rank as CR
        want = syms[0] if syms else None
        syms = []
        for ix, sym in CR.registered_syms():
            if sym != want:
                continue
            tu = CR.tu_for(sym)
            if tu is None:
                print("UNRESOLVED: no TU for %s" % sym)
                return 2
            src = CR.strip_comments(tu.read_text(encoding="utf-8", errors="replace"))
            hits = set(CR.RE_API.findall(src))
            syms = sorted(h for h in hits
                          if not (h.endswith("_e") or h.isupper()))
            print("ACTOR %s -> %d callable candidate(s)\n" % (sym, len(syms)))
    if not syms:
        print("usage: sig_diff.py <symbol> ... | sig_diff.py --actor <PROFILE_SYM>")
        return 2

    bad = 0
    for s in syms:
        v, why, d, r = verdict(s)
        mark = "**" if v == "DIVERGENT" else "  "
        print("%s%-12s %-34s %s" % (mark, v, s, why))
        if v == "DIVERGENT":
            bad += 1
            print("      donor    %s:%s" % (d[0].split("\\")[-1], d[1]))
            print("        %s" % d[2][:104])
            print("      receiver %s:%s" % (r[0].split("\\")[-1], r[1]))
            print("        %s" % r[2][:104])
    print()
    print("%d DIVERGENT of %d checked. **A divergent symbol must NOT be bound "
          "by name**" % (bad, len(syms)))
    print("— reproduce the donor's own definition plugin-side, or bind the")
    print("receiver call the donor's version actually wraps.")
    # ====================================================================
    # THE LIMIT, RESTATED 2026-08-14. It used to read "compares the FIRST
    # declaration found in each tree" — that was true of the version before
    # the overload-aware rewrite and was left standing after it, which is the
    # stale-banner error this lane already logged once. What is actually true:
    # ====================================================================
    print("LIMIT: overload sets ARE compared in full (any donor signature")
    print("matching any receiver signature scores SAME). What is NOT compared")
    print("is ABI: this reads SPELLING, so a typedef difference (CMtxP vs")
    print("const Mtx) shows DIVERGENT while being layout-identical. A")
    print("macro-built signature is invisible to it entirely. SAME here means")
    print("'no divergence seen', never 'proven identical'.")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
