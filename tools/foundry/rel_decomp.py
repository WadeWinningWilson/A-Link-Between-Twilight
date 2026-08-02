"""Foundry §252: rel_decomp — the un-stub pipeline for donor REL actors.

    dtk split asm  ->  m2c draft C  ->  offset-receipt annotation  ->  draft.cpp

Chain of custody (covenant):
  1. `dtk dol split` over the donor's own binaries (orig/GZLE01 junctions to
     the user's extracted donor tree; DOL sha1-verified vs build.sha1) emits
     relocated, symbol-annotated PPC asm per module: build/GZLE01/<mod>/asm/.
  2. m2c (-t ppc = ppc-mwcc-c++, no context — the tww ctx is C++ and m2c's
     context parser is C-only) drafts each function.
  3. This tool annotates every inferred struct field whose offset falls inside
     fopAc_ac_c (0x000-0x290, fully layout-annotated donor header) with the
     REAL member + receipt. Offsets past the base stay [INFERENCE-NEEDED]
     until named from sibling-template analogy or debug-map evidence.

The draft is PORT-GRADE decomp: behavioral fidelity gated by the oracle stack
(probe differ / state taps / golden traces), NOT byte-matching. Never label a
draft MATCH; it is DRAFT until the acceptance gate passes in the receiver.

Usage:
  rel_decomp.py <module>                 --list      (function table + sizes)
  rel_decomp.py <module> -f fn [-f fn2]  [--out o]   (draft chosen functions)
  rel_decomp.py <module> --all           [--out o]   (draft every global fn)
Run with D:\\Decomps\\foundry-py312\\Scripts\\python.exe.
"""
import os, re, subprocess, sys, glob

PY = sys.executable
DECOMP = r"D:\XXXXXXX\WW DP"
M2C = r"D:\XXXXXXX\m2c\m2c.py"
LAYOUT_HDRS = [os.path.join(DECOMP, "include", "f_op", "f_op_actor.h")]

def asm_path(module):
    hits = glob.glob(os.path.join(DECOMP, "build", "GZLE01", module,
                                  "asm", "**", module + ".s"), recursive=True)
    if not hits:
        raise SystemExit(
            f"no dtk asm for {module} — run: dtk dol split "
            f"config/GZLE01/config.yml build/GZLE01  (from {DECOMP})")
    return hits[0]

def m2c_ready_asm(s_path):
    """§256 jtbl pre-pass: mwcc emits anonymous local jump tables ('@5200');
    m2c only treats symbols named jtbl_*/jpt_*/lbl_* as jump tables and fails
    the whole function otherwise. Rename EXACTLY the '@N' objects whose body
    is pure .rel entries (that shape IS a jump table; float/string literals
    use .4byte/.float instead). Writes <module>.m2c.s beside the original."""
    text = open(s_path, encoding="utf-8", errors="replace").read()
    jtbls = []
    for m in re.finditer(
            r'\.obj ("@(\d+)"), local\n((?:\s*\.rel [^\n]+\n)+)\.endobj \1',
            text):
        jtbls.append((m.group(1), f"jtbl_{m.group(2)}"))
    for old, new in jtbls:
        text = text.replace(old, new)
    out = s_path[:-2] + ".m2c.s"
    open(out, "w", encoding="utf-8").write(text)
    return out, len(jtbls)

def fn_table(s_path):
    """[(mangled, linkage, size_insns)] from dtk .fn/.endfn blocks."""
    out, cur, start = [], None, 0
    for i, ln in enumerate(open(s_path, encoding="utf-8", errors="replace")):
        m = re.match(r"\.fn\s+(\S+?),\s*(\w+)", ln)
        if m:
            cur, start = (m.group(1), m.group(2)), i
        elif ln.startswith(".endfn") and cur:
            out.append((cur[0], cur[1], i - start - 1))
            cur = None
    return out

def base_layout():
    """offset -> 'type name' for fopAc_ac_c (and any other layout header)."""
    lay = {}
    for hdr in LAYOUT_HDRS:
        t = open(hdr, encoding="utf-8", errors="replace").read()
        m = re.search(r"class fopAc_ac_c\s*\{(.*?)\n\};", t, re.S)
        if not m:
            continue
        for f in re.finditer(r"/\*\s*0x([0-9A-Fa-f]+)\s*\*/\s*([^;\n]+);",
                             m.group(1)):
            lay[int(f.group(1), 16)] = f.group(2).strip()
    return lay

def annotate(draft, lay):
    """Append the real fopAc member (+receipt) to every inferred struct field
    whose offset lands inside the base class. Annotation-only — no renames, so
    zero correctness risk; the porter sees the receipt inline."""
    ends = sorted(lay)
    def note(off):
        if off >= 0x290:
            return "  /* [INFERENCE-NEEDED: past fopAc base — name via " \
                   "sibling template or debug map] */"
        i = max((e for e in ends if e <= off), default=None)
        if i is None:
            return ""
        member = lay[i]
        d = off - i
        return (f"  /* = fopAc_ac_c::{member} @0x{i:X}"
                + (f" +0x{d:X}" if d else "")
                + " (receipt f_op_actor.h) */")
    out = []
    for ln in draft.splitlines():
        m = re.match(r"\s*/\* 0x([0-9A-Fa-f]+) \*/.*?(?:unk|pad)", ln)
        if m:
            ln += note(int(m.group(1), 16))
        out.append(ln)
    return "\n".join(out)

def main():
    argv = sys.argv[1:]
    module = argv[0]
    s_path = asm_path(module)
    s_path, n_jtbl = m2c_ready_asm(s_path)
    if n_jtbl:
        print(f"[jtbl pre-pass] renamed {n_jtbl} anonymous jump tables")
    fns = fn_table(s_path)
    if "--list" in argv:
        for name, link, size in fns:
            print(f"{size:5d}  {link:6s}  {name}")
        print(f"-- {len(fns)} functions, "
              f"{sum(s for _, _, s in fns)} insns total")
        return
    if "--all" in argv:
        picked = [n for n, l, _ in fns if l != "weak"]
    else:
        picked = [argv[i + 1] for i, a in enumerate(argv) if a == "-f"]
    out_path = None
    if "--out" in argv:
        out_path = argv[argv.index("--out") + 1]
    lay = base_layout()
    cmd = [PY, M2C, "-t", "ppc", "--passes", "2"]
    for f in picked:
        cmd += ["-f", f]
    cmd.append(s_path)
    r = subprocess.run(cmd, capture_output=True, text=True)
    draft = r.stdout
    if r.returncode != 0 or "Decompilation failure" in draft:
        # salvage: retry per-function so one failure doesn't kill the batch
        parts = []
        for f in picked:
            r1 = subprocess.run([PY, M2C, "-t", "ppc", "-f", f, s_path],
                                capture_output=True, text=True)
            parts.append(r1.stdout if r1.returncode == 0 else
                         f"/* [M2C-FAILED {f}: hand-decode via dol_disasm "
                         f"path] */\n{r1.stderr[-400:]}")
        draft = "\n\n".join(parts)
    draft = annotate(draft, lay)
    header = (f"// ====================================================\n"
              f"// PORT-GRADE DECOMP DRAFT — {module} ({len(picked)} fns)\n"
              f"// pipeline: dtk split asm -> m2c(ppc-mwcc-c++, passes=2)\n"
              f"//           -> fopAc offset receipts (rel_decomp.py §252)\n"
              f"// asm: {os.path.relpath(s_path, DECOMP)}\n"
              f"// STATUS: DRAFT — never MATCH. Acceptance = receiver oracle\n"
              f"// stack (probe differ / state taps), per covenant.\n"
              f"// ====================================================\n\n")
    text = header + draft
    if out_path:
        os.makedirs(os.path.dirname(out_path) or ".", exist_ok=True)
        open(out_path, "w", encoding="utf-8").write(text)
        print(f"wrote {out_path} ({len(picked)} fns)")
        # §260: the DATA pass — m2c lifts code only; without this every
        # .obj table (anm specs, event names, ptmf action tables) is hollow
        # (History's §259-bis wall). One run per module, automatic.
        subprocess.run([PY, os.path.join(os.path.dirname(os.path.abspath(
            __file__)), "rodata_extract.py"), module, "--out-dir",
            os.path.dirname(out_path)], check=True)
    else:
        print(text)

if __name__ == "__main__":
    main()
