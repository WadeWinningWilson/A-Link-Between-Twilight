"""Foundry §260: rodata_extract — the pipeline's missing DATA pass (History's
§259-bis wall: m2c lifts code only; every .obj table came out hollow).

Reads the dtk split asm's data sections (.data/.rodata/.sdata*) and emits
byte-faithful C for every table, with typed decodes where the shape is
self-evident:
  - pure .rel-into-stringbase objects  -> const char* tables (strings resolved)
  - {0, 0xFFFFFFFF, <fn symbol>}      -> mwcc pointer-to-member-fn triple
                                          (non-virtual): port form &Class::fn
  - numeric words                      -> raw u32 ground truth + per-word
                                          float/int annotation (never a guess:
                                          the raw bytes ARE the artifact)

Covenant: bytes come from the donor's own binaries via dtk (§256 chain of
custody). The raw arrays are the receipt; typed decodes cite their evidence.

Usage: rodata_extract.py <module> [--obj name ...] [--out-dir d]
       (default: every .obj in the module's data sections)
"""
import os, re, sys, struct, glob

DECOMP = r"D:\XXXXXXX\WW DP"

DATA_SECTS = (".data", ".rodata", ".sdata", ".sdata2", ".bss", ".sbss")

def parse(s_path):
    """-> objs: name -> (section, [directives]), stringbases: base -> {label|0: str}"""
    objs, cur, sect = {}, None, None
    strbase, strcur, stroff_label = {}, None, None
    for ln in open(s_path, encoding="utf-8", errors="replace"):
        t = ln.strip()
        m = re.match(r"\.(data|rodata|sdata2?|bss|sbss)\b", t) or \
            re.match(r'\.section\s+\.(\w+)', t)
        if m:
            sect = "." + m.group(1)
            continue
        m = re.match(r'\.obj ("?[@\w$]+"?), (\w+)', t)
        if m:
            cur = m.group(1).strip('"')
            objs[cur] = (sect, [])
            if cur.startswith("@stringBase"):
                strcur, stroff_label = cur, 0
                strbase[cur] = {}
            continue
        if t.startswith(".endobj"):
            cur = strcur = None
            continue
        if cur is None:
            continue
        lm = re.match(r"(\.L_\w+):", t)
        if lm:
            if strcur:
                stroff_label = lm.group(1)
            else:
                objs[cur][1].append(("label", lm.group(1)))
            continue
        dm = re.match(r'\.(string|asciz)\s+"(.*)"$', t)
        if dm:
            if strcur:
                strbase[strcur][stroff_label] = dm.group(2)
                stroff_label = None  # next string needs its own label
            objs[cur][1].append(("string", dm.group(2)))
            continue
        dm = re.match(r"\.rel\s+(\S+?),\s*(\.L_\w+)", t)
        if dm:
            objs[cur][1].append(("rel", dm.group(1).strip('"'), dm.group(2)))
            continue
        dm = re.match(r"\.(4byte|2byte|byte|8byte|float|double)\s+(.+)$", t)
        if dm:
            objs[cur][1].append((dm.group(1), dm.group(2).strip().strip('"')))
            continue
    return objs, strbase

def word_note(v):
    """annotate a u32 as float if it decodes to a tame value."""
    try:
        f = struct.unpack(">f", struct.pack(">I", v))[0]
        if f != 0 and 1e-6 < abs(f) < 1e6:
            return f"{f:g}f"
    except Exception:
        pass
    return None

def emit(name, sect, dirs, strbase):
    cname = re.sub(r"\W", "_", name)
    rels = [d for d in dirs if d[0] == "rel"]
    words = [d for d in dirs if d[0] == "4byte"]
    # --- string-pointer table -------------------------------------------
    bases = {d[1] for d in rels}
    if rels and len(bases) == 1 and next(iter(bases)).startswith("@stringBase"):
        base = next(iter(bases))
        vals = []
        for d in dirs:
            if d[0] == "4byte" and d[1].startswith("@stringBase"):
                vals.append(strbase.get(base, {}).get(0, "?BASE?"))
            elif d[0] == "rel":
                vals.append(strbase.get(base, {}).get(d[2], f"?{d[2]}?"))
        body = ", ".join(f'"{v}"' for v in vals)
        return (f"// {name} ({sect}) — string-pointer table, {len(vals)} "
                f"entries [decode: .rel into {base}]\n"
                f"static const char* {cname}[{len(vals)}] = {{ {body} }};\n")
    # --- mwcc ptmf triple {0, -1, fn} -----------------------------------
    if (len(words) == 3 and words[0][1] in ("0x00000000", "0")
            and words[1][1].lower() in ("0xffffffff", "-1")
            and re.match(r"[A-Za-z_]", words[2][1])):
        fn = words[2][1]
        short = fn.split("__")[0]
        cls = re.sub(r"^\d+", "", (fn.split("__")[1] if "__" in fn else ""))
        cls = re.sub(r"F\w*$", "", cls)
        return (f"// {name} ({sect}) — mwcc POINTER-TO-MEMBER-FN triple "
                f"{{this_delta=0, vtbl_off=-1 (non-virtual), fn}}\n"
                f"// donor fn: {fn}\n"
                f"// port form: &{cls}::{short}\n"
                f"#define {cname}_PTMF (&{cls}::{short})\n")
    # --- raw words (+float annotations), byte-faithful ------------------
    lines = [f"// {name} ({sect}) — raw ground truth, {len(dirs)} directives"]
    vals, notes = [], []
    for d in dirs:
        if d[0] == "4byte":
            if re.match(r"0[xX]|\d", d[1]):
                v = int(d[1], 0) & 0xFFFFFFFF
                vals.append(f"0x{v:08X}")
                fn = word_note(v)
                notes.append(fn or "")
            else:
                vals.append(f"(u32)&{re.sub(r'[^A-Za-z0-9_]', '_', d[1])} "
                            f"/* SYMBOL: {d[1]} — resolve in port */")
                notes.append("")
        elif d[0] in ("2byte", "byte", "8byte", "float", "double", "string"):
            vals.append(f"/* .{d[0]} {d[1]} */")
            notes.append("")
    rows = []
    for i, (v, nt) in enumerate(zip(vals, notes)):
        rows.append("    " + v + "," + (f"  // [{i}] {nt}" if nt else f"  // [{i}]"))
    lines.append(f"static const u32 {cname}[] = {{")
    lines += rows
    lines.append("};")
    return "\n".join(lines) + "\n"

def main():
    argv = sys.argv[1:]
    module = argv[0]
    hits = glob.glob(os.path.join(DECOMP, "build", "GZLE01", module,
                                  "asm", "**", module + ".s"), recursive=True)
    if not hits:
        raise SystemExit(f"no dtk asm for {module}")
    only = [argv[i + 1] for i, a in enumerate(argv) if a == "--obj"] or None
    out_dir = (argv[argv.index("--out-dir") + 1] if "--out-dir" in argv
               else ".")
    objs, strbase = parse(hits[0])
    picked = [n for n in objs if not n.startswith("@stringBase")
              and objs[n][0] in DATA_SECTS
              and (only is None or n in only)]
    parts = [f"// ============================================================",
             f"// RODATA EXTRACTION — {module} ({len(picked)} tables)",
             f"// rodata_extract.py §260: byte-faithful donor data (dtk asm),",
             f"// typed decode only where the shape self-receipts (strings,",
             f"// ptmf). Raw arrays ARE the ground truth — verify any typed",
             f"// re-declaration against them (memcmp bar, §260).",
             f"// ============================================================",
             ""]
    for n in picked:
        sect, dirs = objs[n]
        if not dirs:
            parts.append(f"// {n} ({sect}) — EMPTY/BSS, no initializer\n")
            continue
        parts.append(emit(n, sect, dirs, strbase))
    os.makedirs(out_dir, exist_ok=True)
    out = os.path.join(out_dir, "rodata_tables.h")
    open(out, "w", encoding="utf-8").write("\n".join(parts))
    print(f"wrote {out}: {len(picked)} tables")

if __name__ == "__main__":
    main()
