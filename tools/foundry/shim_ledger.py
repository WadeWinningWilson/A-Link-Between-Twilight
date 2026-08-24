"""Foundry §261 (kit v4, §248 pitfall 4 — ask #4): the SHARED SHIM LEDGER.

bm1 and Aryll needed the SAME shims; History discovered the overlap by hand and
nearly re-defined two (§248). The ledger kills both costs: every shim any port
added is registered (auto-scanned from the shared shim header(s)), and each new
actor's shims_skeleton says "N of your M shims already exist — reuse; K are new".

Scan sources: include/d/d_ext_*shims*.h (History's §223 architecture: free fns
keep donor names; adapters are dExt<Tag>_*). Declarations AND #defines count.

Usage: shim_ledger.py --scan      rebuild docs/WW Linked/port-kits/_shims-ledger.md
Library: ledger() -> {name: (kind, header, line)} for enemy_port_kit.
"""
import os, re, glob

RECEIVER = r"%USERPROFILE%\Documents\dusklight"
LEDGER_MD = os.path.join(RECEIVER, "docs", "WW Linked", "port-kits",
                         "_shims-ledger.md")

DECL = re.compile(r"^[ \t]*(?:extern\s+|inline\s+|static\s+)*"
                  r"[\w:<>*&\s]+?\b(\w+)\s*\([^;{}]*\)\s*[;{]", re.M)
DEFINE = re.compile(r"^[ \t]*#define\s+(\w+)", re.M)

def shim_headers():
    return sorted(glob.glob(os.path.join(RECEIVER, "include", "d",
                                         "*shims*.h")))

def ledger():
    """name -> (kind, header-rel, line)"""
    out = {}
    for h in shim_headers():
        rel = os.path.relpath(h, RECEIVER).replace(os.sep, "/")
        text = open(h, encoding="utf-8", errors="replace").read()
        # strip comments so commented-out decls don't register
        clean = re.sub(r"/\*.*?\*/", lambda m: "\n" * m.group(0).count("\n"),
                       text, flags=re.S)
        clean = re.sub(r"//[^\n]*", "", clean)
        for rx, kind in ((DECL, "fn"), (DEFINE, "define")):
            for m in rx.finditer(clean):
                name = m.group(1)
                if name in ("if", "while", "for", "switch", "return",
                            "sizeof", "defined"):
                    continue
                line = clean.count("\n", 0, m.start()) + 1
                out.setdefault(name, (kind, rel, line))
    # §261: adapters carry their INTRODUCING actor's prefix
    # (dExtNpcBm1_stopZelAnime) — alias the donor-name suffix so the NEXT
    # actor's skeleton (probing `stopZelAnime` / `dExtAh_stopZelAnime`)
    # still finds the existing shim instead of re-defining it.
    for key in list(out):
        m = re.match(r"dExt\w+?_(\w+)$", key)
        if m:
            out.setdefault(m.group(1), out[key])
    return out

def scan():
    idx = ledger()
    lines = ["# Shared shim ledger (§248 pitfall 4 / §261) — AUTO-GENERATED "
             "by shim_ledger.py --scan; rerun after adding shims", "",
             "> Before writing ANY shim from a skeleton: if it is in this "
             "table, REUSE it (include the header) — a duplicate definition "
             "is a link error at best and a divergent-behavior bug at worst.",
             "",
             "| shim | kind | declared at |", "|---|---|---|"]
    for name in sorted(idx):
        kind, rel, line = idx[name]
        lines.append(f"| `{name}` | {kind} | `{rel}:{line}` |")
    lines += ["", f"**{len(idx)} shims registered across "
              f"{len(shim_headers())} shim header(s).**"]
    open(LEDGER_MD, "w", encoding="utf-8").write("\n".join(lines) + "\n")
    print(f"wrote {LEDGER_MD}: {len(idx)} shims")

if __name__ == "__main__":
    scan()
