"""Foundry §259 (kit v4, §248 pitfall 3 — History's #1 ranked ask):
the VALUE-COLLISION CHECKER.

The landmine class: a WW constant the port lacks gets #defined with its donor
value into a port enum/flag SPACE where that value already means something else
(receipt: WW daPyFlg0_SCOPE_CANCEL = 0x00080000 = port FLG0_FAST_SWORD_CUT —
a naive shim would set fast-sword-cut on Link; both live in d_a_player.h).
A silent collision corrupts save/player state; nothing else in the kit can
hurt you this way.

Method (fully mechanical, honest on no-match):
  1. index every enum member in the DONOR include/src tree (value-resolved:
     hex/dec/(1<<n)/implicit-increment/inherit-previous+1),
  2. same for the RECEIVER,
  3. for each donor constant the TU uses that is ABSENT from receiver enums:
     the receiver SPACE = enums living in files with the SAME BASENAME as the
     donor constant's header (d_a_player.h <-> d_a_player.h); any receiver
     member there with the SAME VALUE = **COLLISION**,
  4. no same-basename receiver enum = NO-SPACE [hand-check] — never silent.

Usage: value_collision.py <donor.cpp> [donor.h ...] [--out collisions.md]
"""
import os, re, sys

DONOR_ROOTS = [r"D:\XXXXXXX\WW DP\include", r"D:\XXXXXXX\WW DP\src"]
RECEIVER_ROOTS = [r"C:\Users\xxxxx\Documents\dusklight\include",
                  r"C:\Users\xxxxx\Documents\dusklight\src"]

ENUM_RE = re.compile(r"enum\s+(\w+)?\s*(?::\s*[\w:]+)?\s*\{(.*?)\}", re.S)
MEM_RE = re.compile(r"^\s*(\w+)\s*(?:=\s*([^,/]+?))?\s*(?:,|$)", re.M)

def _eval(expr, prev):
    if expr is None:
        return prev + 1
    e = expr.strip()
    if re.fullmatch(r"[\d0-9xXa-fA-F()<<|+\-*\s]+", e):
        try:
            return int(eval(e, {"__builtins__": {}}, {}))
        except Exception:
            return None
    return None  # symbolic (refs another constant) — honest unknown

def enum_index(roots):
    """name -> (value|None, enum_name, header_basename, path)"""
    idx = {}
    for root in roots:
        for dp, _d, fs in os.walk(root):
            for fn in fs:
                if not fn.endswith((".h", ".hpp", ".inc")):
                    continue
                p = os.path.join(dp, fn)
                try:
                    t = open(p, encoding="utf-8", errors="replace").read()
                except OSError:
                    continue
                for em in ENUM_RE.finditer(t):
                    ename = em.group(1) or "(anon)"
                    prev = -1
                    for mm in MEM_RE.finditer(em.group(2)):
                        name, expr = mm.group(1), mm.group(2)
                        if name in ("public", "private", "protected"):
                            continue
                        v = _eval(expr, prev)
                        if v is not None:
                            prev = v
                        idx.setdefault(name, (v, ename, fn, p))
    return idx

def check(tu_text, donor_idx, recv_idx):
    """rows for every donor enum constant the TU uses."""
    recv_by_file, recv_by_enum = {}, {}
    for name, (v, en, fn, p) in recv_idx.items():
        recv_by_file.setdefault(fn, []).append((name, v, en))
        recv_by_enum.setdefault(en, []).append((name, v, en))
    used = set(re.findall(r"\b[A-Za-z_]\w{3,}\b", tu_text))
    rows = []
    for name in sorted(used & set(donor_idx)):
        v, den, dfn, _ = donor_idx[name]
        if name in recv_idx:
            continue  # PRESENT — surface.md's business, not a define
        if v == 0:
            continue  # enum-zero: every space starts at 0; not a flag clash
        # SPACE: the receiver enum with the SAME NAME beats same-file enums
        # (daPy_FLG0 <-> daPy_FLG0); same-basename file is the fallback.
        if den != "(anon)" and den in recv_by_enum:
            space, where = recv_by_enum[den], f"enum {den}"
        else:
            space, where = recv_by_file.get(dfn, []), f"file {dfn}"
        if v is None:
            rows.append((name, "?", f"{den} @ {dfn}",
                         "**[HAND-CHECK: donor value symbolic]**"))
            continue
        # §259 coupling: a same-value hit whose NAME is this constant's
        # codemod rename is not a collision — it IS the constant, renamed.
        sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
        from ww2tp_codemod import port_name
        renamed = port_name(name)
        if renamed != name and any(rn == renamed for rn, rv, ren in space
                                   if rv == v):
            rows.append((name, f"0x{v:X}", f"{den} @ {dfn}",
                         f"RENAME-EQUIVALENT: donor name IS port `{renamed}` "
                         "(same value by design) — apply the codemod, do not "
                         "define"))
            continue
        hits = [f"`{rn}` (enum {ren})" for rn, rv, ren in space if rv == v]
        if hits:
            hits = [h + f" [space: {where}]" for h in hits[:4]]
            rows.append((name, f"0x{v:X}", f"{den} @ {dfn}",
                         "**COLLISION with " + ", ".join(hits) +
                         " — DO NOT #define the donor value; shim to 0 or a "
                         "free bit and record it (§248 pitfall 3)**"))
        elif not space:
            rows.append((name, f"0x{v:X}", f"{den} @ {dfn}",
                         "[NO-SPACE: no receiver enum in a same-name header — "
                         "hand-check before defining]"))
        else:
            rows.append((name, f"0x{v:X}", f"{den} @ {dfn}",
                         "safe-to-define (no value clash in space)"))
    return rows

def main():
    argv = sys.argv[1:]
    out = None
    if "--out" in argv:
        out = argv[argv.index("--out") + 1]
        argv = [a for a in argv if a != "--out" and a != out]
    tu = "\n".join(open(s, encoding="utf-8", errors="replace").read()
                   for s in argv)
    rows = check(tu, enum_index(DONOR_ROOTS), enum_index(RECEIVER_ROOTS))
    lines = ["# Value-collision check (§248 pitfall 3 — run BEFORE any shim "
             "#define)", "",
             "| donor constant | value | donor enum @ header | verdict |",
             "|---|---|---|---|"]
    lines += [f"| `{n}` | {v} | {src} | {verdict} |"
              for n, v, src, verdict in rows]
    n_col = sum("COLLISION" in r[3] for r in rows)
    lines += ["", f"**{len(rows)} absent donor constants checked; "
              f"{n_col} COLLISION(s).** A collision silently corrupts "
              "player/save state — the one kit check that can HURT you."]
    text = "\n".join(lines) + "\n"
    if out:
        os.makedirs(os.path.dirname(out) or ".", exist_ok=True)
        open(out, "w", encoding="utf-8").write(text)
        print(f"wrote {out}: {len(rows)} checked, {n_col} collisions")
    else:
        print(text)

if __name__ == "__main__":
    main()
