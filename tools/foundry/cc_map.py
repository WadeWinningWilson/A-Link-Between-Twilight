"""Foundry §223: donor collider-spec extractor (collision parity).

Extracts the static dCcD_Src* initializer blocks (Cyl/Sph/Cps/Tri) from a donor
actor source, verbatim with line numbers — the actor's authored collision law
(radius/height/flags), ready to port 1:1. Registration-side rules are DN-1
territory (standable BG must use the sanctioned host-room resolver); cc_
colliders (touch/hit) are unaffected by DN-1.

Usage: cc_map.py <donor.cpp> [more...] <out.md>
"""
import sys, re

BLOCK = re.compile(
    r"static\s+(dCcD_Src(?:Cyl|Sph|Cps|Tri|Pnt))\s+(\w+)\s*=\s*\{", re.M)

def main():
    *srcs, out_md = sys.argv[1:]
    out = ["# Donor collider specs (dCcD_Src blocks, verbatim)", ""]
    for src in srcs:
        text = open(src, encoding="utf-8", errors="replace").read()
        lines = text.splitlines()
        out.append(f"## `{src}`")
        n = 0
        for m in BLOCK.finditer(text):
            start = text[:m.start()].count("\n")
            depth = 0
            body = []
            for j in range(start, min(start + 80, len(lines))):
                body.append(lines[j])
                depth += lines[j].count("{") - lines[j].count("}")
                if depth <= 0 and j > start:
                    break
            out.append(f"\n### L{start+1} `{m.group(1)} {m.group(2)}`")
            out.append("```cpp")
            out += body
            out.append("```")
            n += 1
        out.append(f"\n({n} collider blocks)\n")
    open(out_md, "w", encoding="utf-8").write("\n".join(out) + "\n")
    print(f"wrote {out_md}")

if __name__ == "__main__":
    main()
