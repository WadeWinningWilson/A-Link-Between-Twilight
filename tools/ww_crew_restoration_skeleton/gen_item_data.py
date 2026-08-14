#!/usr/bin/env python3
"""gen_item_data.py — d_a_item port step 1 (docs/ww-item-actor-port-plan.md).

Generates ww_item_data.inc from the DONOR's own source: dItem_data::item_resource
[0x100] + item_arcname_tbl + item_texture_tbl, with every dRes_INDEX_*_e resolved
to its literal from the donor's generated assets headers (GZLE01 = the shipped
game, per the retail-presence law). ZERO hand transcription (trap #1/#2/#6
discipline); donor enum names survive as comments so provenance is greppable.

Usage: python gen_item_data.py [--out <path>]  (default: prints summary only;
--out writes the .inc). Never writes into the repo by default.
"""
import argparse
import re
import sys
from pathlib import Path

DONOR = Path(r"D:/XXXXXXX/WW DP")
CPP = DONOR / "src/d/d_item_data.cpp"
ASSETS = DONOR / "assets/GZLE01/res"


def parse_seq_enum(text, first_name):
    """Parse a plain sequential enum given its first enumerator name."""
    i = text.index(first_name)
    start = text.rindex("{", 0, i)
    end = text.index("}", i)
    names = re.findall(r"([A-Za-z_][A-Za-z0-9_]*)\s*(?:=\s*(\d+|0x[0-9A-Fa-f]+))?\s*[,}]",
                       text[start:end + 1])
    out, cur = {}, -1
    for name, val in names:
        cur = int(val, 0) if val else cur + 1
        out[name] = cur
    return out


def parse_string_table(text, decl):
    i = text.index(decl)
    start = text.index("{", i)
    depth, j = 0, start
    while True:
        if text[j] == "{":
            depth += 1
        elif text[j] == "}":
            depth -= 1
            if depth == 0:
                break
        j += 1
    body = text[start:j]
    return re.findall(r'"([^"]*)"', body)


def parse_res_enums():
    vals = {}
    for h in ASSETS.rglob("*.h"):
        t = h.read_text(errors="replace")
        for m in re.finditer(r"(dRes_INDEX_[A-Za-z0-9_]+_e)\s*=\s*(0x[0-9A-Fa-f]+|\d+)", t):
            vals[m.group(1)] = int(m.group(2), 0)
    return vals


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--out")
    args = ap.parse_args()

    text = CPP.read_text(errors="replace")
    arc_idx = parse_seq_enum(text, "ARCNAME_IDX_ALWAYS")
    tex_idx = parse_seq_enum(text, "TEXTURE_IDX_")  # first TEXTURE_IDX_* literal
    # the first texture enumerator may not literally be TEXTURE_IDX_; find it:
    if len(tex_idx) <= 1:
        m = re.search(r"TEXTURE_IDX_[A-Za-z0-9_]+", text)
        tex_idx = parse_seq_enum(text, m.group(0))
    arcs = parse_string_table(text, "item_arcname_tbl[0x70]")
    texs = parse_string_table(text, "item_texture_tbl")
    res = parse_res_enums()

    def table_rows(decl):
        tbl_i = text.index(decl)
        tbl_end = text.index("\n};", tbl_i)
        body = text[tbl_i:tbl_end]
        # RETAIL LAW ("the shipped game is the spec", §422): resolve VERSION_DEMO
        # preprocessor forks to the retail (#else / absent) branch before parsing.
        body = re.sub(r"#if VERSION == VERSION_DEMO.*?#else(.*?)#endif", r"\1", body, flags=re.S)
        body = re.sub(r"#if VERSION == VERSION_DEMO.*?#endif", "", body, flags=re.S)
        rows = re.findall(r"\{([^{}]*)\}", body)
        assert len(rows) == 0x100, f"{decl}: row count {len(rows)} != 256"
        return rows

    raw_rows = table_rows("item_resource[0x100] = {")
    raw_field = table_rows("field_item_res[0x100] = {")
    raw_info = table_rows("item_info[0x100] = {")

    def resolve(tok):
        tok = tok.strip()
        m = re.match(r"item_arcname_tbl\[([A-Za-z0-9_]+)\]", tok)
        if m:
            return arcs[arc_idx[m.group(1)]], m.group(1)
        m = re.match(r"item_texture_tbl\[([A-Za-z0-9_]+)\]", tok)
        if m:
            return texs[tex_idx[m.group(1)]], m.group(1)
        if tok in res:
            return res[tok], tok
        if tok in ("NULL", "nullptr"):
            return None, "NULL"
        return int(tok, 0), None

    fields = ["mArcname", "mTexture", "mBmdIdx", "mSrtIdx", "mSrtIdx2", "mTevIdx",
              "mTevIdx2", "mBckIdx", "mTevFrm", "mItemMesgNum", "mUnused",
              "mUnknown", "mHeapSize"]
    fields_f = ["mArc", "mBmdIdx", "mSrtIdx", "mSrtIdx2", "mTevIdx", "mTevIdx2",
                "mBckIdx", "mTevFrm", "mUnknown", "mHeapSize"]
    fields_i = ["mShadowSize", "mCollisionH", "mCollisionR", "mFlag"]

    unresolved = 0

    def parse(raws, flds, label):
        nonlocal unresolved
        out = []
        for ridx, raw in enumerate(raws):
            toks = [t.split("*/")[-1].strip() for t in
                    re.sub(r"//[^\n]*", "", raw).split(",") if t.split("*/")[-1].strip()]
            if len(toks) != len(flds):
                print(f"{label} row {ridx}: {len(toks)} tokens (expect {len(flds)}): "
                      f"{toks[:4]}...", file=sys.stderr)
                unresolved += 1
                out.append([(0, None)] * len(flds))
                continue
            vals = []
            for t in toks:
                try:
                    vals.append(resolve(t))
                except Exception:
                    vals.append((f"/*UNRESOLVED {t}*/-1", t))
                    unresolved += 1
            out.append(vals)
        return out

    rows = parse(raw_rows, fields, "item_resource")
    rows_f = parse(raw_field, fields_f, "field_item_res")
    rows_i = parse(raw_info, fields_i, "item_info")

    # The receiver's model resolver (dExtNpcMount_acquireModelData) takes MEMBER
    # NAMES; the donor table carries INDICES. The assets-header enum encodes
    # <ARC>_<TYPE>_<NAME> and res_arc.py generated it FROM the arc's real member
    # list, so the member name derives mechanically: lower(NAME).lower(TYPE).
    def member_from(enum_name):
        m = re.match(r"dRes_INDEX_[A-Z0-9]+?_(BDL|BTK|BRK|BCK|BMT)_(.+)_e", enum_name or "")
        return (m.group(2).lower() + "." + m.group(1).lower()) if m else None

    # field table gains derived member-name columns after each anim/model index
    fields_f2 = ["mArc", "mBmdIdx", "mBmdMember", "mSrtIdx", "mSrtMember",
                 "mSrtIdx2", "mTevIdx", "mTevMember", "mTevIdx2", "mBckIdx",
                 "mBckMember", "mTevFrm", "mUnknown", "mHeapSize"]
    rows_f2 = []
    for vals in rows_f:
        d = dict(zip(fields_f, vals))
        row = []
        for fld in fields_f2:
            if fld.endswith("Member"):
                src = {"mBmdMember": "mBmdIdx", "mSrtMember": "mSrtIdx",
                       "mTevMember": "mTevIdx", "mBckMember": "mBckIdx"}[fld]
                row.append((member_from(d[src][1]), None))
            else:
                row.append(d[fld])
        rows_f2.append(row)
    rows_f, fields_f = rows_f2, fields_f2

    print(f"arcnames={len(arcs)} textures={len(texs)} resEnums={len(res)} "
          f"rows={len(rows)} unresolved={unresolved}")
    # spot checks (donor receipts)
    spot = rows[0x01]
    print("spot 0x01 GREEN_RUPEE:", spot[0][0], spot[2][0], "| mesg:", spot[9][0])
    spot = rows[0x32]
    print("spot 0x32 FUKU mesg (expect 0):", spot[9][0])

    if args.out:
        def emit(f, name, struct, table, flds, hexf):
            f.write(f"static const {struct} {name}[0x100] = {{\n")
            for ridx, vals in enumerate(table):
                f.write(f"    {{ // 0x{ridx:02X}\n")
                for (v, nm), fld in zip(vals, flds):
                    if isinstance(v, str):
                        lit = f'"{v}"'
                    elif v is None:
                        lit = "NULL"
                    else:
                        lit = f"0x{v:X}" if fld in hexf else str(v)
                    com = f"  // {nm}" if nm else ""
                    f.write(f"        /* {fld:12s} */ {lit},{com}\n")
                f.write("    },\n")
            f.write("};\n\n")

        with open(args.out, "w", encoding="utf-8") as f:
            f.write("// GENERATED by gen_item_data.py from DONOR d_item_data.cpp + GZLE01\n")
            f.write("// assets headers. Do not hand-edit — regenerate. (port plan step 1)\n")
            f.write("// KIT-LINEAGE: native-port (generated donor data)\n")
            emit(f, "l_wwItemResource", "WwItemResource", rows, fields,
                 ("mUnused", "mUnknown", "mHeapSize"))
            emit(f, "l_wwFieldItemRes", "WwFieldItemRes", rows_f, fields_f,
                 ("mUnknown", "mHeapSize"))
            emit(f, "l_wwItemInfo", "WwItemInfo", rows_i, fields_i, ())
        print(f"wrote {args.out}")
    return 0 if unresolved == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
