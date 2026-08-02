"""Foundry §222: donor-actor API surface scan — the port-vs-transplant decider.

For a donor actor source, extracts every external identifier it calls/uses and
checks which exist in the receiver tree. Output: coverage % + the MISSING list
(= the literal work plan for a direct source port). High coverage → direct-port
the donor file (5k lines is mechanical when the API matches); low coverage →
chassis-transplant (receiver enemy chassis driven by the donor state map).

Usage: api_surface.py <donor.cpp> [donor.h ...] --receiver <repo-root> <out.md>
"""
import sys, re, os, subprocess
from collections import Counter

CALL = re.compile(r"\b([A-Za-z_][A-Za-z_0-9]{3,})\s*\(")
TYPE = re.compile(r"\b([A-Z][A-Za-z_0-9]{3,}(?:_c|_class|_e)?)\b")
SKIP = {"if", "for", "while", "switch", "return", "sizeof", "defined", "static",
        "const", "float", "double", "void", "case", "else", "memcpy", "memset",
        "strcmp", "strncpy", "strlen", "printf", "sqrtf", "fabsf", "cosf", "sinf",
        "atan2f", "abs", "min", "max", "new", "delete", "this", "TRUE", "FALSE",
        "NULL", "BOOL", "assert"}

IDENT = re.compile(rb"[A-Za-z_][A-Za-z_0-9]{3,}")

CLASS_OPEN = re.compile(r"\b(class|struct)\s+([A-Za-z_]\w*)")
DECL_NAME = re.compile(r"\b([A-Za-z_]\w{3,})\s*\(")

def build_decl_index(donor_include_root):
    """§224 (History request): one pass over donor headers → for every function-
    like declaration: (kind free/method, enclosing class, verbatim signature,
    file:line). Powers both the shim-strategy classification and the stub sheet."""
    decls = {}
    for dirpath, _dirs, files in os.walk(donor_include_root):
        for fn in files:
            if not fn.endswith((".h", ".inc")):
                continue
            path = os.path.join(dirpath, fn)
            try:
                lines = open(path, encoding="utf-8",
                             errors="replace").read().splitlines()
            except OSError:
                continue
            class_stack = []   # (name, depth_at_open)
            depth = 0
            pending_class = None
            for i, ln in enumerate(lines):
                m = CLASS_OPEN.search(ln)
                if m and ";" not in ln.split(m.group(2))[-1][:4]:
                    pending_class = m.group(2)
                for ch in ln:
                    if ch == "{":
                        depth += 1
                        if pending_class:
                            class_stack.append((pending_class, depth))
                            pending_class = None
                    elif ch == "}":
                        if class_stack and class_stack[-1][1] == depth:
                            class_stack.pop()
                        depth -= 1
                s = ln.strip()
                dm = DECL_NAME.search(s)
                if not dm or s.startswith(("if", "for", "while", "switch",
                                           "return", "//", "/*", "#")):
                    continue
                name = dm.group(1)
                # capture full signature: join until ; or {
                sig = s
                j = i
                while ";" not in sig and "{" not in sig and j + 1 < len(lines) \
                        and j - i < 6:
                    j += 1
                    sig += " " + lines[j].strip()
                sig = sig.split("{")[0].split(";")[0].strip()
                encl = class_stack[-1][0] if class_stack else None
                rel = os.path.relpath(path, donor_include_root)
                decls.setdefault(name, []).append(
                    (("method:" + encl) if encl else "free", sig,
                     f"{rel}:{i+1}"))
    return decls

def build_index(roots):
    """One pass over receiver sources: the set of all identifiers present."""
    idx = set()
    for root in roots:
        for dirpath, _dirs, files in os.walk(root):
            for fn in files:
                if not fn.endswith((".h", ".hpp", ".cpp", ".c", ".inc")):
                    continue
                try:
                    data = open(os.path.join(dirpath, fn), "rb").read()
                except OSError:
                    continue
                idx.update(m.group(0).decode("ascii", "ignore")
                           for m in IDENT.finditer(data))
    return idx

def build_locator(roots, wanted):
    """§261 (§248 pitfall 7): identifier -> ([header rels], [src rels]) for a
    SMALL wanted-set — so 'present' can say WHERE, and header-declared (one
    #include) separates from src-only (no include can reach it)."""
    hdrs, srcs = {}, {}
    wanted_b = {w.encode() for w in wanted}
    for root in roots:
        for dirpath, _dirs, files in os.walk(root):
            for fn in files:
                if not fn.endswith((".h", ".hpp", ".cpp", ".c", ".inc")):
                    continue
                p = os.path.join(dirpath, fn)
                try:
                    data = open(p, "rb").read()
                except OSError:
                    continue
                rel = os.path.relpath(p, root).replace(os.sep, "/")
                is_h = fn.endswith((".h", ".hpp"))
                for w in wanted_b:
                    if w in data and re.search(rb"\b" + w + rb"\b", data):
                        tgt = hdrs if is_h else srcs
                        tgt.setdefault(w.decode(), []).append(rel)
    return hdrs, srcs

def main():
    argv = sys.argv[1:]
    ri = argv.index("--receiver")
    srcs, root, out_md = argv[:ri], argv[ri + 1], argv[ri + 2]
    text = "\n".join(open(s, encoding="utf-8", errors="replace").read()
                     for s in srcs)
    # strip comments/strings crudely
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", " ", text)
    text = re.sub(r'"[^"\n]*"', " ", text)
    calls = Counter(m.group(1) for m in CALL.finditer(text))
    # locally-defined names don't count as external
    local = set(re.findall(r"\b(\w+)\s*\([^;{)]*\)\s*\{", text))
    local |= set(re.findall(r"(?:class|struct|enum)\s+(\w+)", text))
    idents = {n: c for n, c in calls.items()
              if n not in SKIP and n not in local and not n.isupper()}
    present, missing = [], []
    search_roots = [p for p in (os.path.join(root, "src"),
                                os.path.join(root, "include"),
                                os.path.join(root, "libs")) if os.path.isdir(p)]
    idx = build_index(search_roots)
    for name in sorted(idents, key=lambda n: -idents[n]):
        (present if name in idx else missing).append((name, idents[name]))
    cov = 100.0 * len(present) / max(1, len(present) + len(missing))
    # §224: classify missing + pull verbatim signatures from donor headers
    donor_inc = None
    for s in srcs:
        p = s
        while p and os.path.basename(p) != "src":
            p = os.path.dirname(p)
        if p:
            donor_inc = os.path.join(os.path.dirname(p), "include")
            break
    decls = build_decl_index(donor_inc) if donor_inc and \
        os.path.isdir(donor_inc) else {}
    lines = [f"# API surface — {', '.join('`'+os.path.basename(s)+'`' for s in srcs)} vs receiver",
             "",
             f"**{len(present)}/{len(present)+len(missing)} external identifiers "
             f"present in receiver = {cov:.1f}% coverage.** Missing list below is "
             "the direct-port work plan; kind decides the shim strategy "
             "(free → adapter function; method → class edit / wrapper).", "",
             f"## MISSING ({len(missing)}) — the shim stub sheet",
             "| identifier | uses | kind | verbatim donor signature (cited) |",
             "|---|---|---|---|"]
    for n, c in missing:
        found = decls.get(n)
        if found:
            kind, sig, where = found[0]
            extra = f" (+{len(found)-1} overloads)" if len(found) > 1 else ""
            sig = sig.replace("|", "\\|")
            lines.append(f"| `{n}` | {c} | {kind}{extra} | `{sig}` — {where} |")
        else:
            lines.append(f"| `{n}` | {c} | UNKNOWN | not found in donor headers "
                         "— [INFERENCE-NEEDED: locate decl (src-local static? "
                         "macro?)] |")
    # §261 (§248 pitfall 7 — ABSENT vs PRESENT@header): locate every present
    # symbol; emit the INCLUDE PLAN (headers grouped) + src-only warnings.
    hdrs, srcloc = build_locator(search_roots, {n for n, _ in present})
    by_header, src_only = {}, []
    for n, _c in present:
        hs = hdrs.get(n)
        if hs:
            by_header.setdefault(hs[0], []).append(n)
        elif n in srcloc:
            src_only.append((n, srcloc[n][0]))
    lines += ["", f"## INCLUDE PLAN (§261) — headers that supply this TU's "
              f"{len(present)} receiver-present symbols",
              "", "> A symbol listed here needs an `#include`, NOT a shim "
              "(§248 pitfall 7 — conflating the two inflates cost and wastes "
              "build rounds). One row per header, ready to become the "
              "include block.", "",
              "| header | symbols it supplies |", "|---|---|"]
    for h in sorted(by_header, key=lambda h: -len(by_header[h])):
        syms = by_header[h]
        shown = ", ".join(f"`{s}`" for s in syms[:10])
        more = f" (+{len(syms)-10} more)" if len(syms) > 10 else ""
        lines.append(f"| `{h}` | {shown}{more} |")
    if src_only:
        lines += ["", f"**SRC-ONLY ({len(src_only)}) — present but no header "
                  "declares them: an #include CANNOT reach these; treat as "
                  "ABSENT (extern or shim):**", ""]
        lines += [f"- `{n}` (only in `{w}`)" for n, w in src_only[:20]]
    lines += ["", f"## PRESENT ({len(present)}) — top 40 by use",
              "| identifier | uses | first header |", "|---|---|---|"]
    for n, c in present[:40]:
        h = hdrs.get(n, [""])
        lines.append(f"| `{n}` | {c} | `{h[0]}` |" if h[0] else
                     f"| `{n}` | {c} | (src-only) |")
    open(out_md, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"coverage {cov:.1f}% ({len(present)} present, {len(missing)} missing); wrote {out_md}")

if __name__ == "__main__":
    main()
