#!/usr/bin/env python3
# ============================================================================
# gen_cam_data.py — generate the WW camera style/type tables for the WW layer.
#
# WHY GENERATED. The donor's camera data is COMPILED-IN source (d_cam_style.cpp
# styles[] / d_cam_type.cpp types[] / d_cam_type2.cpp mvBGTypes[]) — there is
# no camtype.dat on the WW disc (that file is a TP-ism; verified §672). Porting
# ~130 style records x 32 fields by hand invites silent transcription errors,
# so the tables are parsed MECHANICALLY from the donor's own TUs and emitted in
# the RECEIVER's record layout. The donor source is the spec; this tool is the
# consumption-boundary translation (zero-bake: donor files are read, never
# edited).
#
# WHAT IT VERIFIES (hard failures, not warnings):
#   * dCamStyle_e enum order == styles[] row order, name for name — the type
#     table stores enum values as s16 indices, so misalignment poisons every
#     type row.
#   * every style's algorithm has a receiver mapping (shared by name, or an
#     explicit extra-engine socket — nothing is dropped, per the user ruling).
#   * every type row carries exactly 20 slots; params 28/29 (dropped in the
#     28-param receiver record) are LOCKON_FOVY_MIN/MAX and nothing else.
#
# OUTPUT. src/d/ext_plugin/ww_cam_data.inc:
#   * kWwCamStyleBlobBE[]  — a camstyle.dat-SHAPED byte blob (header + records,
#     big-endian) so receiver-side accessors (BE() wrappers) read it exactly
#     like file data. Record alg fields hold RECEIVER engine indices.
#   * kWwCamTypes[]        — donor-axis type rows {name, s16[20]} (native
#     endian; consumed by WW-layer selector code only).
#   * kWwCamBgTypeNames[]  — the BG-attribute -> type-name table.
#
# VERSION: the donor tables carry #if VERSION guards; this emits GZLE01 (USA,
# VERSION=2), matching the sanctioned dump the disc reader serves.
#
# Usage: python tools/ww_crew_restoration_skeleton/gen_cam_data.py
# Regenerate whenever the donor checkout moves; the .inc header records the
# source paths + row counts for drift review.
# ============================================================================
import re
import struct
import sys
from pathlib import Path

DONOR = Path("D:/XXXXXXX/WW DP")
OUT = Path(__file__).resolve().parents[2] / "src/d/ext_plugin/ww_cam_data.inc"

VERSION = 2  # GZLE01 (USA); donor global.h: DEMO 0, JPN 1, USA 2, PAL 3
VERSION_MACROS = {"VERSION_DEMO": 0, "VERSION_JPN": 1, "VERSION_USA": 2, "VERSION_PAL": 3}

# Donor dCamAlg_e index -> receiver engine index. Receiver engine_tbl
# (d_camera.cpp:201) holds 20 engines: 0-11 shared with the donor by name,
# 12-19 TP-only. Donor algorithms with no receiver engine get EXTRA-ENGINE
# SOCKET numbers (>= 20, dispatched through dCamera_setExtraEngineHook);
# 20 = CRAWL is live (ww_cam_crawl.cpp), the rest are sockets awaiting their
# ports — data reaches them, the hook logs the visit, nothing is dropped.
ALG_MAP = {
    0: (0, "LET -> letCamera"),
    1: (1, "FOLLOW -> chaseCamera"),
    2: (2, "LOCKON -> lockonCamera"),
    3: (3, "TALKTO -> talktoCamera"),
    4: (4, "SUBJECT -> subjectCamera"),
    5: (5, "FIXED_POSITION -> fixedPositionCamera"),
    6: (6, "FIXED_FRAME -> fixedFrameCamera"),
    7: (7, "TOWER -> towerCamera"),
    8: (8, "RIDE -> rideCamera"),
    9: (21, "HUNG -> SOCKET 21 (unported)"),
    10: (9, "MANUAL -> manualCamera"),
    11: (10, "EVENT -> eventCamera"),
    12: (20, "CRAWL -> extra engine 20 (LIVE, ww_cam_crawl)"),
    13: (11, "HOOKSHOT -> hookshotCamera"),
    14: (22, "TORNADO -> SOCKET 22 (unported)"),
    15: (23, "VOMIT -> SOCKET 23 (unported)"),
    16: (24, "SHIELD -> SOCKET 24 (unported)"),
    17: (25, "NON_OWNER -> SOCKET 25 (unported)"),
    18: (26, "FOLLOW2 -> SOCKET 26 (unported)"),
    19: (27, "DEMO -> SOCKET 27 (unported)"),
}


def die(msg):
    print(f"gen_cam_data: FAIL: {msg}", file=sys.stderr)
    sys.exit(1)


# ============================================================================
# RECEIVER camstyle.dat, extracted live from the user's TP image.
#
# WHY (measured 2026-08-10, the below-the-map flight): the receiver
# RE-AUTHORED its style records — same 4CC identities, permuted slots AND
# retuned values (donor TT02 d5=-30 sits at receiver r2; the receiver keeps
# its own -99999.99 sentinel convention). No permutation reproduces it, so a
# donor param vector fed to a RECEIVER engine reads sentinels/garbage as real
# values. The receiver's OWN record for the same 4CC is its native dialect of
# that donor style — for styles that run receiver engines it is the correct
# consumption-boundary translation. Styles that run WW-layer engines (sockets
# 20+) keep DONOR order: the ported donor code reads donor indices.
# ============================================================================
# ============================================================================
# yaz0_decompress MOVED to tools/foundry/ww_disc.py (§683 import-back — one
# implementation, Foundry-owned since the §680 promotion). This import keeps
# every existing caller in this file working unchanged.
# ============================================================================
import sys as _sys
_sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "foundry"))
from ww_disc import yaz0_decompress  # noqa: E402


def read_tp_camstyle():
    """Extract res/Object/CamParam.arc::camstyle.dat from the configured TP ISO."""
    import json
    import os
    iso_path = os.environ.get("TP_ISO")
    if not iso_path:
        cfg = os.path.expandvars(r"%APPDATA%\TwilitRealm\Dusklight\config.json")
        iso_path = json.load(open(cfg, encoding="utf-8")).get("backend.isoPath", "")
    if not iso_path or not Path(iso_path).is_file():
        die("TP ISO not found (backend.isoPath in config.json, or set TP_ISO)")
    f = open(iso_path, "rb")
    boot = f.read(0x440)
    if boot[:6].decode("ascii", "replace") not in ("GZ2E01", "GZ2P01", "GZ2J01"):
        die(f"configured image is not TP (game id {boot[:6]!r})")
    fst_off, fst_size = struct.unpack(">II", boot[0x424:0x42C])
    f.seek(fst_off)
    fst = f.read(fst_size)
    n = struct.unpack(">I", fst[8:12])[0]
    strings = fst[n * 12:]
    target = None
    dirs = [(n, "")]
    for i in range(1, n):
        while i >= dirs[-1][0]:
            dirs.pop()
        e = fst[i * 12:(i + 1) * 12]
        name = strings[(e[1] << 16) | (e[2] << 8) | e[3]:].split(b"\0")[0].decode("ascii", "replace")
        off, ln = struct.unpack(">II", e[4:12])
        if e[0]:
            dirs.append((ln, dirs[-1][1] + name + "/"))
        elif dirs[-1][1] + name == "res/Object/CamParam.arc":
            target = (off, ln)
    if target is None:
        die("res/Object/CamParam.arc not found on the TP image")
    f.seek(target[0])
    arc = f.read(target[1])
    if arc[:4] == b"Yaz0":
        arc = yaz0_decompress(arc)
    if arc[:4] != b"RARC":
        die("CamParam.arc is not RARC after decompression")
    data_off = struct.unpack(">I", arc[0xC:0x10])[0] + 0x20
    _, _, num_ents, ent_tbl = struct.unpack(">IIII", arc[0x20:0x30])
    str_tbl = struct.unpack(">I", arc[0x34:0x38])[0] + 0x20
    ent_tbl += 0x20
    for i in range(num_ents):
        e = arc[ent_tbl + i * 0x14: ent_tbl + (i + 1) * 0x14]
        name_off = struct.unpack(">H", e[6:8])[0]
        d_o, d_l = struct.unpack(">II", e[8:16])
        nm = arc[str_tbl + name_off: arc.index(b"\0", str_tbl + name_off)].decode("ascii", "replace")
        if nm.lower() == "camstyle.dat":
            blob = arc[data_off + d_o: data_off + d_o + d_l]
            count = struct.unpack(">i", blob[4:8])[0]
            recs = {}
            for j in range(count):
                r = blob[8 + j * 0x78: 8 + (j + 1) * 0x78]
                recs[r[0:4].decode("ascii", "replace")] = {
                    "alg": struct.unpack(">H", r[4:6])[0],
                    "flags": struct.unpack(">H", r[6:8])[0],
                    "params": list(struct.unpack(">28f", r[8:120])),
                }
            return recs
    die("camstyle.dat not found inside CamParam.arc")


def preprocess(text):
    """Strip comments; apply #if/#elif/#else/#endif using VERSION semantics."""
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    out = []
    # Stack of (parent_active, this_branch_active, any_branch_taken)
    stack = []

    def cond(expr):
        e = expr.strip()
        for name, val in VERSION_MACROS.items():
            e = e.replace(name, str(val))
        e = e.replace("VERSION", str(VERSION))
        try:
            return bool(eval(e, {"__builtins__": {}}))
        except Exception:
            die(f"cannot evaluate preprocessor condition: {expr!r}")

    for line in text.splitlines():
        s = line.strip()
        if s.startswith("#ifdef") or s.startswith("#ifndef"):
            # Include guards / feature ifdefs: treat as transparent (active
            # when the parent is) — none of the donor camera tables use them
            # for content selection.
            parent = all(fr[1] for fr in stack) if stack else True
            stack.append([parent, parent, True])
        elif s.startswith("#if "):
            parent = all(fr[1] for fr in stack) if stack else True
            taken = cond(s[4:]) if parent else False
            stack.append([parent, taken, taken])
        elif s.startswith("#elif "):
            fr = stack[-1]
            fr[1] = fr[0] and not fr[2] and cond(s[6:])
            fr[2] = fr[2] or fr[1]
        elif s.startswith("#else"):
            fr = stack[-1]
            fr[1] = fr[0] and not fr[2]
            fr[2] = True
        elif s.startswith("#endif"):
            stack.pop()
        elif s.startswith("#"):
            continue  # includes etc.
        else:
            if all(fr[1] for fr in stack) if stack else True:
                out.append(line)
    if stack:
        die("unbalanced preprocessor conditionals")
    return "\n".join(out)


def parse_enum(header_text, enum_name):
    m = re.search(rf"enum {enum_name}\s*\{{(.*?)\}};", header_text, flags=re.S)
    if not m:
        die(f"enum {enum_name} not found")
    names = []
    values = {}
    next_val = 0
    for tok in m.group(1).split(","):
        tok = tok.strip()
        if not tok:
            continue
        if "=" in tok:
            name, val = (t.strip() for t in tok.split("=", 1))
            next_val = int(val, 0)
        else:
            name = tok
        names.append(name)
        values[name] = next_val
        next_val += 1
    return names, values


def parse_styles(text):
    m = re.search(r"styles\[\]\s*=\s*\{(.*)\n\};", text, flags=re.S)
    if not m:
        die("styles[] initializer not found")
    body = m.group(1)
    rows = []
    # Row shape: { '4CC', dCamAlg_X_e, { 30 floats }, flags-expr, }
    row_re = re.compile(
        r"\{\s*'(?P<id>[A-Za-z0-9_]{4})'\s*,\s*(?P<alg>dCamAlg_\w+)\s*,\s*"
        r"\{(?P<params>[^}]*)\}\s*,\s*(?P<flags>[^,}]+)\s*,\s*\}",
        flags=re.S)
    for m2 in row_re.finditer(body):
        params = [p.strip() for p in m2.group("params").split(",") if p.strip()]
        if len(params) != 30:
            die(f"style {m2.group('id')}: {len(params)} params, expected 30")
        rows.append({
            "id": m2.group("id"),
            "alg": m2.group("alg"),
            "params": [float(p.rstrip("fF")) for p in params],
            "flags": m2.group("flags").strip(),
        })
    return rows


def parse_types(text):
    m = re.search(r"types\[\]\s*=\s*\{(.*)\n\};", text, flags=re.S)
    if not m:
        die("types[] initializer not found")
    rows = []
    row_re = re.compile(
        r'\{\s*"(?P<name>[^"]+)"\s*,\s*\{(?P<slots>[^}]*)\}\s*,\s*\}', flags=re.S)
    for m2 in row_re.finditer(m.group(1)):
        slots = [s.strip() for s in m2.group("slots").split(",") if s.strip()]
        if len(slots) != 20:
            die(f"type {m2.group('name')}: {len(slots)} slots, expected 20")
        rows.append({"name": m2.group("name"), "slots": slots})
    return rows


def parse_bg_types(text):
    m = re.search(r"mvBGTypes\[\d+\]\s*=\s*\{(.*?)\};", text, flags=re.S)
    if not m:
        die("mvBGTypes[] not found")
    return re.findall(r'"([^"]*)"', m.group(1))


def eval_flags(expr, flag_values):
    expr = expr.strip()
    if expr == "0":
        return 0
    val = 0
    for tok in expr.split("|"):
        tok = tok.strip()
        if tok not in flag_values:
            die(f"unknown flag token {tok!r}")
        val |= flag_values[tok]
    return val


def main():
    param_h = preprocess((DONOR / "include/d/d_cam_param.h").read_text(encoding="utf-8"))
    style_cpp = preprocess((DONOR / "src/d/d_cam_style.cpp").read_text(encoding="utf-8"))
    type_cpp = preprocess((DONOR / "src/d/d_cam_type.cpp").read_text(encoding="utf-8"))
    type2_cpp = preprocess((DONOR / "src/d/d_cam_type2.cpp").read_text(encoding="utf-8"))

    style_enum_names, style_enum_vals = parse_enum(param_h, "dCamStyle_e")
    param_enum_names, param_enum_vals = parse_enum(param_h, "dCamStyleParam_e")
    flag_names, flag_vals = parse_enum(param_h, "dCamParamFlag_e")
    alg_names, alg_vals = parse_enum(param_h, "dCamAlgorythmn_e")

    # The 28-param receiver record drops donor params 28/29; prove they are the
    # LOCKON_FOVY pair (crawl reads only 0,1,3,4,5,10,25 — loses nothing).
    dropped = sorted(n for n, v in param_enum_vals.items() if v >= 28)
    if dropped != ["dCamStyleParam_LOCKON_FOVY_MAX", "dCamStyleParam_LOCKON_FOVY_MIN"]:
        die(f"params >=28 are {dropped}, expected the LOCKON_FOVY pair")

    styles = parse_styles(style_cpp)
    types = parse_types(type_cpp)
    bg_types = parse_bg_types(type2_cpp)

    # HARD alignment check: enum order == row order, name for name. The type
    # table stores dCamStyle_*_e values as indices into styles[]. NONE is the
    # -1 "no style" sentinel, not a row.
    row_names = [n for n in style_enum_names if n != "dCamStyle_NONE_e"]
    if len(styles) != len(row_names):
        die(f"{len(styles)} style rows vs {len(row_names)} non-NONE enum names")
    for i, row in enumerate(styles):
        expect = f"dCamStyle_{row['id']}_e"
        if row_names[i] != expect or style_enum_vals[row_names[i]] != i:
            die(f"row {i} is {row['id']} but enum[{i}] is "
                f"{row_names[i]}={style_enum_vals[row_names[i]]}")

    # ------------------------------------------------------------------------
    # Emit the camstyle.dat-shaped blob: {u8 magic[4]; s32be count; records}.
    #
    # PARAM/FLAG SOURCE PER RECORD (the below-the-map lesson, see header):
    #   * layer engine (mapped >= 20)      -> DONOR params[0:28] + donor flags
    #     (the ported donor code reads donor indices/flag bits)
    #   * receiver engine + same-4CC twin  -> the TWIN's params + flags
    #     (the receiver's native dialect of the same donor style identity)
    #   * receiver engine, donor-only 4CC  -> nearest same-donor-algorithm twin
    #     by donor-param distance — a LABELED STAND-IN, owed the per-engine
    #     param-name translation (each receiver engine re-derived its slot
    #     vocabulary; decoding all of them is its own work order)
    # ------------------------------------------------------------------------
    tp_styles = read_tp_camstyle()
    donor_by_id = {row["id"]: row for row in styles}

    def donor_dist(a, b):
        d = 0.0
        for x, y in zip(a["params"], b["params"]):
            sx = x < -99999.0
            sy = y < -99999.0
            if sx != sy:
                d += 10.0
            elif not sx:
                d += min(abs(x - y) / (1.0 + abs(x) + abs(y)), 1.0)
        return d

    blob = bytearray(b"WWCS")
    blob += struct.pack(">i", len(styles))
    crawl_styles = []
    twin_count = 0
    fallbacks = []
    for row in styles:
        alg_idx = alg_vals[row["alg"]]
        if alg_idx not in ALG_MAP:
            die(f"style {row['id']}: algorithm {row['alg']} has no receiver mapping")
        mapped, _ = ALG_MAP[alg_idx]
        if mapped == 20:
            crawl_styles.append(row["id"])
        donor_flags = eval_flags(row["flags"], flag_vals)
        if donor_flags > 0xFFFF:
            die(f"style {row['id']}: flags {donor_flags:#x} exceed the receiver's u16")

        if mapped >= 20:
            out_params = row["params"][:28]
            out_flags = donor_flags
        else:
            twin = tp_styles.get(row["id"])
            if twin is not None and twin["alg"] != mapped:
                print(f"  NOTE {row['id']}: twin alg {twin['alg']} != mapped {mapped} — "
                      f"treating as donor-only")
                twin = None
            if twin is not None:
                twin_count += 1
            else:
                cands = [cc for cc, d_row in donor_by_id.items()
                         if cc in tp_styles and alg_vals[d_row["alg"]] == alg_idx and
                         tp_styles[cc]["alg"] == mapped]
                if not cands:
                    die(f"style {row['id']}: no receiver twin family for its algorithm")
                best = min(cands, key=lambda cc: donor_dist(row, donor_by_id[cc]))
                twin = tp_styles[best]
                fallbacks.append((row["id"], best))
            out_params = twin["params"]
            out_flags = twin["flags"]

        blob += struct.pack(">i", int.from_bytes(row["id"].encode("ascii"), "big"))
        blob += struct.pack(">HH", mapped, out_flags)
        for p in out_params:
            blob += struct.pack(">f", p)
    rec_size = 4 + 2 + 2 + 28 * 4
    assert len(blob) == 8 + rec_size * len(styles)
    print(f"receiver-engine records: {twin_count} native twins, {len(fallbacks)} fallback "
          f"stand-ins (owed the per-engine param translation):")
    for a, b in fallbacks:
        print(f"  {a} -> receiver {b}")

    # Type rows: resolve enum tokens to s16 indices (donor axis, native endian).
    type_rows = []
    for t in types:
        idxs = []
        for tok in t["slots"]:
            if tok == "dCamStyle_NONE_e":
                idxs.append(-1)
            elif tok in style_enum_vals:
                idxs.append(style_enum_vals[tok])
            else:
                die(f"type {t['name']}: unknown style token {tok!r}")
        type_rows.append((t["name"], idxs))

    # Report the facts a reviewer should check against the §670-era decode.
    subject = next((r for r in type_rows if r[0] == "Subject"), None)
    if subject is None:
        die("no Subject type row")
    subj_live = {i: styles[v]["id"] for i, v in enumerate(subject[1]) if v >= 0}
    print(f"styles: {len(styles)}  types: {len(type_rows)}  bg-type names: {len(bg_types)}")
    print(f"CRAWL-algorithm styles: {crawl_styles}")
    print(f"Subject non-NONE slots: {subj_live}")
    for cc in crawl_styles:
        row = next(r for r in styles if r["id"] == cc)
        print(f"  {cc}: flags={eval_flags(row['flags'], flag_vals):#05x}")

    # ------------------------------------------------------------------------
    # SHADOW type table, receiver shape ({char name[24]; s16be [2][11]}) in a
    # camtype.dat-shaped blob. Receiver code OUTSIDE the replaced selection
    # block still reads mCamTypeData[...] (event-style restore, same-style
    # compares), so while the WW selector is authoritative the pointer must
    # back those reads coherently. Slots are filled from donor slots via the
    # MEASURED mode correspondence (receiver r <- donor d): r0-6 <- d0-6
    # (aligned), r7 <- d10 (SX aim), r8 <- d11 (SY aim), r9 <- d15 (JN jump,
    # same status1 0x10 bit both lineages), r10 <- d6 (nearest; receiver 10 =
    # chain-block class, donor has no direct slot). Wolf row duplicates the
    # human row — the donor has no wolf axis. The donor-axis rows above stay
    # the authority; this is a compatibility view only.
    # ------------------------------------------------------------------------
    shadow_map = [0, 1, 2, 3, 4, 5, 6, 10, 11, 15, 6]
    shadow = bytearray(b"WWCT")
    shadow += struct.pack(">i", len(type_rows))
    for name, idxs in type_rows:
        nm = name.encode("ascii")
        if len(nm) > 23:
            die(f"type name too long for name[24]: {name}")
        shadow += nm + b"\0" * (24 - len(nm))
        for _wolf in range(2):
            for r in shadow_map:
                shadow += struct.pack(">h", idxs[r])
    assert len(shadow) == 8 + (24 + 2 * 2 * 11) * len(type_rows)

    lines = []
    a = lines.append
    a("// ============================================================================")
    a("// ww_cam_data.inc — GENERATED by tools/ww_crew_restoration_skeleton/gen_cam_data.py")
    a("// DO NOT HAND-EDIT. Regenerate against the donor checkout instead.")
    a("//")
    a("// Source: donor d/d_cam_style.cpp + d/d_cam_type.cpp + d/d_cam_type2.cpp,")
    a(f"// preprocessed for GZLE01 (VERSION={VERSION}).")
    a(f"// {len(styles)} styles, {len(type_rows)} types, {len(bg_types)} BG type names.")
    a("//")
    a("// kWwCamStyleBlobBE is shaped EXACTLY like the receiver's camstyle.dat")
    a("// ({u8[4]; s32be styleNum; then 0x78-byte records {s32be id; u16be alg;")
    a("// u16be flags; f32be params[28]}}) so receiver BE() accessors read it as")
    a("// file data. Record alg fields already hold RECEIVER engine indices:")
    for k in sorted(ALG_MAP):
        a(f"//   donor {k:2d} {ALG_MAP[k][1]}")
    a("// Donor params 28/29 (LOCKON_FOVY_MIN/MAX) are dropped by the receiver's")
    a("// 28-param record; verified the tail is exactly that pair.")
    a("// ============================================================================")
    a("")
    a("// clang-format off")
    a(f"static const int kWwCamStyleCount = {len(styles)};")
    a(f"static const int kWwCamTypeCount = {len(type_rows)};")
    a(f"static const int kWwCamBgTypeCount = {len(bg_types)};")
    a("")
    for cc in crawl_styles:
        a(f"static const int kWwCamStyleIdx_{cc} = {style_enum_vals[f'dCamStyle_{cc}_e']};  // CRAWL-algorithm style")
    a("")
    a(f"alignas(4) static const unsigned char kWwCamStyleBlobBE[{len(blob)}] = {{")
    for off in range(0, len(blob), 16):
        chunk = ", ".join(f"0x{b:02X}" for b in blob[off:off + 16])
        a(f"    {chunk},")
    a("};")
    a("")
    a("// Receiver-shaped SHADOW type rows (camtype.dat-shaped, big-endian) — see")
    a("// the generator for the r<-d slot correspondence. Compatibility view only;")
    a("// kWwCamTypes (donor axis) is the authority.")
    a(f"alignas(4) static const unsigned char kWwCamTypeShadowBE[{len(shadow)}] = {{")
    for off in range(0, len(shadow), 16):
        chunk = ", ".join(f"0x{b:02X}" for b in shadow[off:off + 16])
        a(f"    {chunk},")
    a("};")
    a("")
    a("static const dExtWwCamType kWwCamTypes[] = {")
    for name, idxs in type_rows:
        idx_str = ", ".join(str(v) for v in idxs)
        a(f'    {{"{name}", {{{idx_str}}}}},')
    a("};")
    a("")
    a("static const char* const kWwCamBgTypeNames[] = {")
    for name in bg_types:
        a(f'    "{name}",')
    a("};")
    a("// clang-format on")
    a("")

    OUT.write_text("\n".join(lines), encoding="utf-8", newline="\n")
    print(f"wrote {OUT} ({len(blob)} blob bytes)")


if __name__ == "__main__":
    main()
