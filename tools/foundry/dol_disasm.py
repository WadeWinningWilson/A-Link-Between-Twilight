"""Foundry §251: dol_disasm — donor-verbatim PPC disassembly of any VA range
straight from the donor DOL (sha1-verified == decomp framework.dol, so the
decomp source's /* 80xxxxxx-80xxxxxx */ comments address it directly).

The decode instrument for un-decompiled (/* Nonmatching */) donor stubs:
symbol names come from the decomp source comments + debug linker maps; the
bytes come from the user's own donor files (covenant: everything derives from
the donor ISO).

Usage: dol_disasm.py <startVA> <endVA> [--dol path]
       dol_disasm.py --func <name> [--dol path]   (find range via decomp src comment)
"""
import sys, os, re, struct

DOL_DEFAULT = r"D:\XXXXXXX\Ex WW\sys\main.dol"
DONOR_SRC = r"D:\XXXXXXX\WW DP\src"
SYMBOLS = r"D:\XXXXXXX\WW DP\config\GZLE01\symbols.txt"

def dol_sections(path):
    d = open(path, "rb").read()
    offs = struct.unpack(">18I", d[0x00:0x48])
    addrs = struct.unpack(">18I", d[0x48:0x90])
    sizes = struct.unpack(">18I", d[0x90:0xD8])
    return d, [(a, o, s) for a, o, s in zip(addrs, offs, sizes) if s]

def read_va(path, start, end):
    d, secs = dol_sections(path)
    for a, o, s in secs:
        if a <= start and end <= a + s:
            return d[o + (start - a): o + (end - a)]
    raise SystemExit(f"VA range {start:#x}-{end:#x} not inside any DOL section")

def find_func(name):
    """Locate the /* start-end .text mangled */ comment for a function name."""
    pat = re.compile(r"/\*\s*([0-9A-Fa-f]{8})-([0-9A-Fa-f]{8})\s+\.text\s+(\S+)\s*\*/")
    for dirpath, _d, files in os.walk(DONOR_SRC):
        for fn in files:
            if not fn.endswith((".cpp", ".inc", ".c")):
                continue
            p = os.path.join(dirpath, fn)
            t = open(p, encoding="utf-8", errors="replace").read()
            for m in pat.finditer(t):
                if name in m.group(3):
                    return int(m.group(1), 16), int(m.group(2), 16), m.group(3), p
    raise SystemExit(f"function {name!r} not found in donor source comments")

def symbol_map():
    """VA -> name from the dtk symbols.txt (for branch-target annotation)."""
    out = {}
    if os.path.exists(SYMBOLS):
        for line in open(SYMBOLS, encoding="utf-8", errors="replace"):
            m = re.match(r"(\S+)\s*=\s*\S+:(0x[0-9A-Fa-f]+)", line)
            if m:
                out[int(m.group(2), 16)] = m.group(1)
    return out

def disasm(code, base, syms):
    from capstone import Cs, CS_ARCH_PPC, CS_MODE_32, CS_MODE_BIG_ENDIAN
    md = Cs(CS_ARCH_PPC, CS_MODE_32 | CS_MODE_BIG_ENDIAN)
    lines = []
    for ins in md.disasm(code, base):
        note = ""
        m = re.search(r"0x[0-9a-f]+$", ins.op_str)
        if m and ins.mnemonic.startswith(("b", "bl")):
            tgt = int(m.group(0), 16)
            if tgt in syms:
                note = f"    ; -> {syms[tgt]}"
        lines.append(f"{ins.address:08x}:  {ins.mnemonic:<10} {ins.op_str}{note}")
    return lines

def main():
    argv = sys.argv[1:]
    dol = DOL_DEFAULT
    if "--dol" in argv:
        i = argv.index("--dol"); dol = argv[i + 1]; del argv[i:i + 2]
    if argv[0] == "--func":
        start, end, mangled, src = find_func(argv[1])
        print(f"; {mangled}  ({src})")
    else:
        start, end = int(argv[0], 16), int(argv[1], 16)
    syms = symbol_map()
    code = read_va(dol, start, end)
    print(f"; VA {start:08x}-{end:08x}  ({len(code)} bytes, {len(code)//4} insns)")
    print("\n".join(disasm(code, start, syms)))

if __name__ == "__main__":
    main()
