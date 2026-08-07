#!/usr/bin/env python3
# ============================================================================
# gen_cluster_dispatch.py — generate a step-19 dispatch layer for one cluster.
#
# WHY GENERATED
# A cluster is 35-40 free functions the receiver calls directly. Hand-copying
# that many signatures is not honest work: this campaign has already paid twice
# for transcription error -- a dropped profile row that the readiness check
# structurally could not see, and an uppercase-only regex that turned a real
# symbol into a phantom. Both were "I copied them all" claims.
#
# So the set comes from TWO independent inputs that must agree:
#   1. the LINKER's own unresolved list (what the build actually needs)
#   2. the HEADER's declarations (the signatures)
# A symbol in (1) with no match in (2) is reported, never guessed at.
#
# WHAT IT EMITS
#   struct   one function pointer per entry point
#   dispatch the public names, forwarding when bound, safe default when not
#   install  the WW-side binding of every field
#
# The defaults are safe because the return types say so: void does nothing,
# bool is false ("not a WW case"), pointers are NULL, scalars are 0. None
# fabricates a value the receiver could mistake for real data.
#
# Usage: gen_cluster_dispatch.py <symbol-prefix> <header> [<header> ...]
# ============================================================================
import collections
import pathlib
import re
import sys

REPO = pathlib.Path(__file__).resolve().parents[2]
LOG = REPO / "build" / "excl2.log"


def unresolved(prefix):
    if not LOG.is_file():
        sys.exit(f"no exclusion log at {LOG} — run the exclusion build first")
    text = LOG.read_text(encoding="utf-8", errors="replace")
    out = set()
    for s in re.findall(r'unresolved external symbol\s+"([^"]+)"', text):
        m = re.search(r'\b(' + re.escape(prefix) + r'[A-Za-z0-9_]+)', s)
        if m:
            out.add(m.group(1))
    return out


def declarations(headers, prefix):
    decls = {}
    for h in headers:
        t = (REPO / h).read_text(encoding="utf-8", errors="replace")
        # Strip preprocessor lines and comments BEFORE flattening. Without this
        # a `#if TARGET_PC` on the preceding line is absorbed into the next
        # declaration's return type ("if TARGET_PC void") -- the same silent
        # mis-transcription this generator exists to prevent, reintroduced by
        # the generator itself. It happened; hence the comment.
        t = re.sub(r"^\s*#.*$", "", t, flags=re.M)
        t = re.sub(r"//[^" + chr(10) + r"]*", "", t)
        t = re.sub(r"/\*.*?\*/", "", t, flags=re.S)
        flat = re.sub(r"\s+", " ", t)
        pat = (r'([A-Za-z_][A-Za-z0-9_:\*&\s]*?)\s(' + re.escape(prefix)
               + r'[A-Za-z0-9_]+)\s*\(([^)]*)\)\s*;')
        for m in re.finditer(pat, flat):
            decls[m.group(2)] = (m.group(1).strip(), m.group(3).strip())
    return decls


def default_for(ret):
    r = ret.replace(" ", "")
    if r == "void":
        return None
    if r == "bool":
        return "false"
    if r.endswith("*") or r.endswith("&"):
        return "NULL"
    return "0"


def main():
    if len(sys.argv) < 3:
        sys.exit(__doc__ or "usage: gen_cluster_dispatch.py <prefix> <header>...")
    prefix, headers = sys.argv[1], sys.argv[2:]
    want, decls = unresolved(prefix), declarations(headers, prefix)

    rows, missing = [], []
    for name in sorted(want):
        if name not in decls:
            missing.append(name)
            continue
        ret, args = decls[name]
        params = []
        if args and args.replace(" ", "") != "void":
            for a in args.split(","):
                m = re.search(r'([A-Za-z_][A-Za-z0-9_]*)\s*$', a.strip().replace("[]", ""))
                params.append(m.group(1) if m else "")
        rows.append((name, ret, args, params, default_for(ret)))

    print(f"{prefix}: {len(want)} unresolved, {len(decls)} declared, "
          f"{len(rows)} generated")
    if missing:
        # REPORTED, never guessed. A symbol the linker wants that no header
        # declares is a real gap and the operator has to see it.
        print(f"  UNMATCHED ({len(missing)}) — declared nowhere in the given headers:")
        for m in missing:
            print("   ", m)
    print("  return types:",
          dict(collections.Counter(r[1] for r in rows)))

    # Field names are uniformly prefixed. `dExtNpcMount_delete` would otherwise
    # become a field called `delete` -- a C++ keyword and a hard syntax error.
    # Sanitising only the colliding names would leave the next keyword to be
    # discovered by a failing build, so every field takes the prefix.
    short = lambda n: "fn_" + n[len(prefix):].lstrip("_")
    st = ["struct " + prefix.rstrip("_") + "Api {"]
    ds, rg = [], []
    for name, ret, args, params, dflt in rows:
        f = short(name)
        st.append(f"    {ret} (*{f})({args});")
        call = ", ".join(p for p in params if p)
        ds.append(f"{ret} {name}({args}) {{")
        if dflt is None:
            ds.append(f"    if (g_api.{f}) {{ g_api.{f}({call}); }}")
        else:
            ds.append(f"    return g_api.{f} ? g_api.{f}({call}) : {dflt};")
        ds.append("}")
        rg.append(f"    g_api.{f} = &{prefix}Impl_{f[3:]};")
    st.append("};")

    out = REPO / "build" / "gen"
    out.mkdir(parents=True, exist_ok=True)
    (out / f"{prefix}struct.txt").write_text("\n".join(st), encoding="utf-8")
    (out / f"{prefix}dispatch.txt").write_text("\n".join(ds), encoding="utf-8")
    (out / f"{prefix}install.txt").write_text("\n".join(rg), encoding="utf-8")
    (out / f"{prefix}names.txt").write_text(
        # short() carries the `fn_` field prefix; the IMPL symbol must not, or
        # this map and the install list would name different functions.
        "\n".join(f"#define {n} {prefix}Impl_{short(n)[3:]}" for n, *_ in rows),
        encoding="utf-8")
    print(f"  wrote 4 fragments to {out}")
    return 1 if missing else 0


if __name__ == "__main__":
    raise SystemExit(main())
