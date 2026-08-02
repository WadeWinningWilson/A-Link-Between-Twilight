"""Foundry: donor actor params-decode extractor (multi-type support, bus §214).

Mechanically extracts how a donor actor DECODES its placement params — the
shift/mask reads that select subtypes (History's Ikada mType case). Verbatim
quotes with line numbers; field SEMANTICS stay [INFERENCE-NEEDED] unless the
donor's own identifiers name them (mType etc. — quoted, not invented).

Usage: param_map.py <decomp .cpp/.h> [more files...] then last arg = out.md
"""
import sys, re

PATTERNS = [
    re.compile(r"fopAcM_GetParam\s*\("),
    re.compile(r"fopAcM_GetParamBit\s*\("),
    re.compile(r"\bprm[A-Za-z_0-9]*\b.*(>>|&)"),
    re.compile(r"(>>\s*(?:0x[0-9A-Fa-f]+|\d+))?\s*&\s*0x[0-9A-Fa-f]+"),
    re.compile(r"\bm(Type|Prm|Param)[A-Za-z_0-9]*\b"),
]
KEEP_HINT = re.compile(
    r"fopAcM_GetParam|GetParamBit|prm|mType|mPrm|mParam|parameters?\b", re.I)

def main():
    *srcs, out_md = sys.argv[1:]
    out = ["# Donor params decode map", ""]
    for src in srcs:
        lines = open(src, encoding="utf-8", errors="replace").read().splitlines()
        out.append(f"## `{src}`")
        n = 0
        for i, ln in enumerate(lines, 1):
            s = ln.strip()
            if len(s) > 220 or not KEEP_HINT.search(s):
                continue
            if any(p.search(s) for p in PATTERNS[:3]) or \
               (PATTERNS[4].search(s) and ("=" in s or "(" in s)):
                out.append(f"- L{i} `{s}`")
                n += 1
        out.append(f"\n({n} param-related lines)\n")
    out.append("> Field semantics beyond the donor's own identifiers are "
               "[INFERENCE-NEEDED]; verify against decomp usage before labeling "
               "subtypes in any census.")
    open(out_md, "w", encoding="utf-8").write("\n".join(out) + "\n")
    print(f"wrote {out_md}")

if __name__ == "__main__":
    main()
