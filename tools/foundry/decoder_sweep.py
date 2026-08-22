#!/usr/bin/env python3
# ============================================================================
# decoder_sweep.py — combination sweep over source-spelling toggles (DECODER)
#
# v0 of the transform-search harness the park ledgers name. Given a spec
# listing binary spelling "slots" (current text vs alternate text) inside ONE
# translation unit, this compiles every combination directly with mwcc
# (bypassing ninja) and scores ONE symbol with objdiff-cli, reporting the
# best combinations. Interaction effects between spellings are exactly what
# hand-probing misses; 32 combinations run in ~4 minutes.
#
# Proven on d_a_grid::ho_move (2026-08-22): falsified the 5-knob toggle
# space around the caller-save temp-rotation park in one run.
#
# Spec (JSON):
# {
#   "root": "D:/XXXXXXX/WW DP",
#   "src": "src/d/actor/d_a_grid.cpp",          // relative to root
#   "obj": "build/GZLE01/src/d/actor/d_a_grid.o",
#   "unit": "framework/d/actor/d_a_grid",
#   "symbol": "ho_move__FP8daGrid_c",
#   "cflags": "...",   // optional; default = grep the obj's edge in build.ninja
#   "slots": [ {"name": "...", "current": "...", "alt": "..."}, ... ]
# }
#
# The source file is ALWAYS restored to its pre-sweep bytes at the end,
# including on error. The winning combination is only REPORTED — applying
# it is a deliberate act, not a side effect.
# ============================================================================
import argparse, itertools, json, os, re, subprocess, sys, tempfile

def ninja_cflags(root, obj):
    edge = "build " + obj.replace("/", "\\") + ": mwcc_sjis"
    lines = open(os.path.join(root, "build.ninja"), encoding="utf-8").read().splitlines()
    for i, l in enumerate(lines):
        if l.startswith(edge):
            buf, j = [], i + 1
            while j < len(lines) and not lines[j].startswith("build "):
                buf.append(lines[j]); j += 1
            m = re.search(r"cflags = (.*?)(?:\n  \w+ =|\Z)",
                          "\n".join(x.rstrip("$").rstrip() for x in buf), re.S)
            if m:
                return " ".join(m.group(1).split())
    raise SystemExit(f"no mwcc edge found for {obj}")

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("spec")
    ap.add_argument("--max-combos", type=int, default=64,
                    help="refuse to run more than this many builds")
    args = ap.parse_args()
    spec = json.load(open(args.spec, encoding="utf-8"))
    root = spec["root"]
    src = os.path.join(root, spec["src"])
    cflags = spec.get("cflags") or ninja_cflags(root, spec["obj"])
    slots = spec["slots"]
    n = len(slots)
    total = 2 ** n
    if total > args.max_combos:
        raise SystemExit(f"{total} combinations > --max-combos {args.max_combos}")

    base = open(src, encoding="utf-8").read()
    for s in slots:
        if s["current"] not in base:
            raise SystemExit(f"slot '{s['name']}': current text not found in {src}")

    diffj = os.path.join(tempfile.gettempdir(), "decoder_sweep_diff.json")
    objdiff = os.path.join(root, "build", "tools", "objdiff-cli.exe")
    compile_cmd = (f'build\\tools\\sjiswrap.exe build\\compilers\\GC\\1.3.2\\mwcceppc.exe '
                   f'{cflags} -c {spec["src"].replace("/", chr(92))} '
                   f'-o {spec["obj"].replace("/", chr(92))}')

    def score():
        r = subprocess.run(compile_cmd, cwd=root, shell=True, capture_output=True, text=True)
        if r.returncode != 0:
            return None, (r.stderr or r.stdout)[-160:]
        subprocess.run([objdiff, "diff", "-p", ".", "-u", spec["unit"], "-o", diffj,
                        spec["symbol"]], cwd=root, capture_output=True, text=True)
        d = json.load(open(diffj))
        for sym in d["left"]["symbols"]:
            if spec["symbol"] in (sym.get("name") or ""):
                return sym.get("match_percent"), None
        return None, "symbol missing from diff"

    results = []
    try:
        for bits in itertools.product([0, 1], repeat=n):
            text = base
            for s, b in zip(slots, bits):
                if b:
                    text = text.replace(s["current"], s["alt"], 1)
            open(src, "w", encoding="utf-8", newline="\n").write(text)
            pct, err = score()
            tag = "".join(map(str, bits))
            if err:
                print(tag, "FAIL", err, flush=True)
            else:
                results.append((pct, tag))
                print(tag, f"{pct:.4f}", flush=True)
    finally:
        open(src, "w", encoding="utf-8", newline="\n").write(base)
        # recompile so the .o on disk matches the restored source, not the
        # last combination the loop happened to build
        subprocess.run(compile_cmd, cwd=root, shell=True, capture_output=True, text=True)
        print("BASELINE RESTORED (source AND object back to pre-sweep state)")

    results.sort(reverse=True)
    if results:
        base_pct = dict((t, p) for p, t in results).get("0" * n)
        print(f"\nbaseline {'0'*n}: {base_pct}")
        print("TOP 5:")
        for pct, tag in results[:5]:
            marks = [s["name"] for s, b in zip(slots, tag) if b == "1"]
            print(f"  {tag} -> {pct:.4f}   alts: {marks or '(none)'}")
        print("slot order:", [s["name"] for s in slots], "(0=current, 1=alt)")

if __name__ == "__main__":
    main()
