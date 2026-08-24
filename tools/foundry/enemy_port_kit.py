# ============================================================================
# ERA WARNING (Foundry kit-family sweep, 2026-08-21 - user order: kits may
# still carry mount/bake/legs-era recipes; History saw remnants and this
# sweep CONFIRMED them family-wide).
#
# THIS KIT'S MODEL RULE IS MOUNT-ERA AND INVERTED FOR PLUGIN PORTS:
# 'route dExtNpcMount_acquireModelData - NEVER raw' is correct FORK-side
# and IMPOSSIBLE plugin-side (the symbol does not exist on vanilla; the
# plugin's model path is the receiver getRes choke point). MOD_ARCS
# points at the model_replacements era.
#
# Era doctrine of record: actor_kit.py's banner + its era-split checklist
# ([FORK] vs [PLUGIN] columns) and the trace method at
# docs/state/ww-plugin-outset.md #How-ports-land. PLUGIN delivery never
# routes through dExtNpcMount / f_pc edits / files.cmake / model_replacements
# - those are FORK-era mechanisms a stock-dusklight user never receives.
# This banner marks the era; a DEEP retool of this kit is a named plate item,
# not implied by the banner's presence.
# ============================================================================
"""Foundry §230: the Enemy Port Kit — one command per donor actor, everything
History derived by hand for the pig (§225-§229), generated.

Emits a kit directory:
  surface.md        api_surface v2 (coverage + classified stub sheet)
  statemap.md       state_map (all four idioms)
  ccmap.md          collider specs
  parammap.md       params decode
  res-manifest.md   resLoad arcs + dRes_INDEX_* usage + mod-arc/header pre-flight
  renames.md        known WW->TP rename hits, line-cited (the compile-cascade
                    killer; dictionary accumulates per port)
  shims_skeleton.h  History's §223 shim pattern, pre-generated: free fns keep
                    donor names, method gaps become dExt<Tag>_ adapters —
                    signatures verbatim from donor headers, bodies TODO
  README.md         the §229 recipe checklist (resource model, spawn flip, gate)

Usage: enemy_port_kit.py <donor.cpp> [donor.h] --tag <Tag> --out <kit-dir>
"""
import sys, os, re, subprocess

PY = sys.executable
TOOLS = os.path.dirname(os.path.abspath(__file__))
RECEIVER = r"%USERPROFILE%\Documents\dusklight"
MOD_ARCS = r"%USERPROFILE%\AppData\Roaming\TwilitRealm\Dusklight\model_replacements\WW-Crew-Restoration\arcs"

# WW->TP rename dictionary — accumulates per port (§229 pig compile cascade).
RENAMES = {
    "cPhs_State": ("cPhs_Step", "§229 pig cascade root (5 compile rounds)"),
    # add each new port's discoveries here, with the bus § receipt
}

# Semantic PATTERNS (not 1:1 renames) — each a symbolicated §229 receipt.
PATTERNS = [
    ("setUserArea\\s*\\(\\s*\\(\\s*u32\\s*\\)",
     "64-BIT TRUNCATION: donor (u32)this puns pointer→int; port API is "
     "uintptr_t — use setUserArea((uintptr_t)this). §229 gotcha 2 (wild "
     "low-32-bit deref in joint callback)."),
    ("\\(\\s*(?:u32|s32)\\s*\\)\\s*(?:this|i_this|actor|p[A-Z]\\w*)",
     "POINTER CAST AUDIT: (u32)/(s32) on a pointer-ish expression — latent "
     "AV on x86_64. §229 gotcha 2: audit every occurrence."),
    ("updateDL\\s*\\(",
     "McaMORF RENDER SPLIT: the port has NO McaMorf::updateDL(); donor "
     "updateDL = modelCalc() + dComIfGd_setList() + entryDL() (mount draw "
     "d_ext_npc_mount.cpp:9014/9398/9410 — 'modelCalc() IS WHAT MAKES THE "
     "BASE MATRIX REAL'). play()+entryDL() alone renders INVISIBLE. §229 "
     "gotcha 1 (render)."),
]

# Res-routing law (§229 DN-3 for direct ports): required handling per res type.
RES_ROUTING = [
    # ========================================================================
    # ERA-SPLIT 2026-08-21 (deep retool, follows actor_kit's precedent): the
    # model law was MOUNT-ERA stated as if universal. It stays TRUE for FORK
    # ports and is IMPOSSIBLE for PLUGIN ports — dExtNpcMount does not exist
    # on vanilla; a plugin TU routed through it cannot link. The plugin law is
    # the receiver's own getRes choke point (trace method step 4, lwood R2-R6
    # template; d_resorce.cpp parses BMD-family AT LOAD, so the slot already
    # holds J3DModelData* — 'parse-at-consume' is the RECEIVER'S job there,
    # not the port's).
    # ========================================================================
    (("BDL", "BMD", "BDLM", "BDLI", "BDLC"),
     "MODEL [FORK] — NEVER raw: route dExtNpcMount_acquireModelData(arc, "
     "name) (parse-at-consume, DN-3); NULL-guard → graceful missing actor. "
     "[PLUGIN] — the receiver getRes choke point via registry.cpp hooks; "
     "the receiver parses BMD-family at load, the slot holds J3DModelData* "
     "already (res_consumer_census.py is the receipt). dExtNpcMount is "
     "FORK-ONLY and will not link against vanilla."),
    (("BCK", "BAS"),
     "ANIM — raw getObjectRes is FINE (bound via morf/setAnm binder, which "
     "fixes it)"),
    (("BTP", "BMT", "BRK", "BTK"),
     "TEX/MAT — NEVER call methods directly on the raw resource: bind via the "
     "port's anim-init on a FIXED model, or NULL + guard call sites; size "
     "tables off the FIXED model's getMaterialNum()"),
]

def run(tool, *args):
    subprocess.run([PY, os.path.join(TOOLS, tool), *args], check=True)

# ---------------------------------------------------------------------------
# v3 (§232 dogfood gaps): include closure · symbol equivalence · typed skeleton
# ---------------------------------------------------------------------------
INCLUDE_RE = re.compile(r'^\s*#include\s+"([^"]+)"', re.M)
DONOR_INC = r"<decomp-root>\WW DP\include"
DONOR_ASSETS = r"<decomp-root>\WW DP\assets\GZLE01"
DONOR_SRC = r"<decomp-root>\WW DP\src"

def _donor_path(rel):
    for root in (DONOR_INC, DONOR_ASSETS, DONOR_SRC):
        p = os.path.join(root, rel.replace("/", os.sep))
        if os.path.exists(p):
            return p
    return None

def _receiver_path(rel):
    for root in ("include", "src", os.path.join("assets", "GZ2E01"),
                 os.path.join("libs", "JSystem", "include")):
        p = os.path.join(RECEIVER, root, rel.replace("/", os.sep))
        if os.path.exists(p):
            return p
    return None

DECLARED = re.compile(r"\b(?:class|struct|enum)\s+([A-Za-z_]\w+)|"
                      r"typedef\s+[^;]*?\b([A-Za-z_]\w+)\s*;")

def include_closure(srcs, tu_text):
    """Walk donor #includes recursively. Classify each header:
    DONOR-ONLY (stage it) · NATIVE-OK · NATIVE-DIVERGED (receiver header exists
    but lacks donor-declared symbols the TU uses — §232 gap 2) · NOT-FOUND."""
    seen, order = set(), []
    frontier = []
    for s in srcs:
        frontier += INCLUDE_RE.findall(open(s, encoding="utf-8",
                                            errors="replace").read())
    while frontier:
        rel = frontier.pop(0)
        if rel in seen or len(seen) > 400:
            continue
        seen.add(rel)
        dp = _donor_path(rel)
        rp = _receiver_path(rel)
        if rp:
            status, missing = "NATIVE-OK", []
            if dp:
                donor_txt = open(dp, encoding="utf-8", errors="replace").read()
                recv_txt = open(rp, encoding="utf-8", errors="replace").read()
                declared = {g for m in DECLARED.finditer(donor_txt)
                            for g in m.groups() if g}
                declared = {d for d in declared
                            if len(d) >= 4 and d not in
                            ("this", "else", "true", "false", "void")}
                used = {d for d in declared
                        if re.search(r"\b" + re.escape(d) + r"\b", tu_text)}
                missing = sorted(d for d in used
                                 if not re.search(r"\b" + re.escape(d) + r"\b",
                                                  recv_txt))
                if missing:
                    status = "NATIVE-DIVERGED"
            order.append((rel, status, missing))
        elif dp:
            order.append((rel, "DONOR-ONLY (stage)", []))
            frontier += INCLUDE_RE.findall(
                open(dp, encoding="utf-8", errors="replace").read())
        else:
            order.append((rel, "NOT-FOUND", []))
    return order

def main():
    argv = sys.argv[1:]
    tag = argv[argv.index("--tag") + 1]
    out = argv[argv.index("--out") + 1]
    srcs = argv[:argv.index("--tag")]
    os.makedirs(out, exist_ok=True)
    text = "\n".join(open(s, encoding="utf-8", errors="replace").read()
                     for s in srcs)

    run("api_surface.py", *srcs, "--receiver", RECEIVER,
        os.path.join(out, "surface.md"))
    # §259 (§248 asks #1+#2): value-collision check + applied codemod
    run("value_collision.py", *srcs, "--out",
        os.path.join(out, "collisions.md"))
    run("ww2tp_codemod.py", *srcs, "--out-dir",
        os.path.join(out, "ported_src"))
    run("state_map.py", *srcs, os.path.join(out, "statemap.md"))
    run("cc_map.py", *srcs, os.path.join(out, "ccmap.md"))
    run("param_map.py", *srcs, os.path.join(out, "parammap.md"))

    # --- res manifest ---------------------------------------------------
    arcs = sorted(set(re.findall(r'resLoad\w*\(\s*"(\w+)"', text)) |
                  set(re.findall(r'dComIfG_getObjectRes\(\s*"(\w+)"', text)) |
                  # §235 NPC idiom: runtime arc-name variables
                  # (strcpy(mArcName, "Bm") → resLoad(&phs, mArcName))
                  set(re.findall(r'strcpy\(\s*\w*[Aa]rc\w*\s*,\s*"(\w+)"', text)) |
                  set(re.findall(r'\w*[Aa]rc\w*\s*=\s*"(\w+)"', text)))
    res_ids = sorted(set(re.findall(r"\b(dRes_INDEX_\w+)\b", text)))
    # §235 NPC idiom: resources fetched by NAME string, not index
    res_names = sorted(set(re.findall(
        r'"(\w+\.(?:bdl|bmd|bck|btp|btk|bmt|brk|bas|bti))"', text)))
    lines = [f"# Res manifest — tag `{tag}`", "",
             "| donor arc | mod arc present? | assets res header present? |",
             "|---|---|---|"]
    for a in arcs:
        arc_ok = os.path.exists(os.path.join(MOD_ARCS, a + ".arc"))
        hdr = os.path.join(RECEIVER, "assets", "GZ2E01", "res", "Object", a + ".h")
        lines.append(f"| `{a}` | {'YES' if arc_ok else '**MISSING**'} | "
                     f"{'YES' if os.path.exists(hdr) else '**MISSING — copy donor <arc>.h (§229 resource model)**'} |")
    lines += ["", f"## dRes_INDEX_* used ({len(res_ids)})", ""]
    lines += [f"- `{r}`" for r in res_ids]
    if res_names:
        lines += ["", f"## String-named resources used ({len(res_names)}) "
                  "(§235 NPC idiom — verify each exists in the mod arc)", ""]
        lines += [f"- `{r}`" for r in res_names]
    # §236: BMT structure census for staged arcs (the pig-shade gap class —
    # material-only BMTs AV any unconditional texture-table swap).
    bmt_rows = []
    for a in arcs:
        ap = os.path.join(MOD_ARCS, a + ".arc")
        if not os.path.exists(ap):
            continue
        try:
            import io as _io
            sys.path.insert(0, r"<decomp-root>\wwrando")
            from gclib.rarc import RARC
            data = open(ap, "rb").read()
            rarc = RARC(_io.BytesIO(data))
            for fe in rarc.file_entries:
                nm = getattr(fe, "name", "")
                if nm.lower().endswith(".bmt"):
                    raw = fe.data.getvalue() if hasattr(fe.data, "getvalue") \
                        else bytes(fe.data)
                    bmt_rows.append((a, nm, b"TEX1" in raw, b"MAT3" in raw))
        except Exception as ex:
            bmt_rows.append((a, f"<census failed: {ex.__class__.__name__}>",
                             None, None))
    if bmt_rows:
        lines += ["", "## BMT structure (§236 law: guard replaceTextures on "
                  "TEX1; material-only = J3DMatCopyFlag_Material semantics)",
                  "", "| arc | bmt | TEX1 | MAT3 | exposure |", "|---|---|---|---|---|"]
        for a, nm, t, m3 in bmt_rows:
            if t is None:
                lines.append(f"| `{a}` | {nm} | — | — | — |")
                continue
            expo = "**MATERIAL-ONLY — texture-swap guard REQUIRED**" if not t \
                else ("**TEXTURE-ONLY — material-copy guard required**"
                      if not m3 else "")
            lines.append(f"| `{a}` | `{nm}` | {'Y' if t else 'N'} | "
                         f"{'Y' if m3 else 'N'} | {expo} |")
    lines += ["", f"## dRes_INDEX_* routing (§229 DN-3 law for direct ports)", "",
              "| res index | type | REQUIRED routing |", "|---|---|---|"]
    for r in res_ids:
        rtype = "?"
        rule = ("[INFERENCE-NEEDED: type not in index name — check donor "
                "res header]")
        for suffixes, law in RES_ROUTING:
            if any(f"_{s}_" in r or r.endswith(s) for s in suffixes):
                rtype, rule = "/".join(suffixes[:1]), law
                break
        lines.append(f"| `{r}` | {rtype} | {rule} |")
    lines += ["", "> §229 resource model: mod-arc resLoad is GLOBAL; "
              "dRes_info_c::loadResource builds mRes[] for any resLoad'd mod "
              "arc — the donor Create IS the pattern, no rewrite. Models are "
              "the ONE exception: DN-3 says parse-at-consume via "
              "dExtNpcMount_acquireModelData (d_ext_npc_mount.cpp:3873; Bmt "
              "variant :3887). Diagnostic tell: 'Loading Resource: X.bdl' in "
              "the log yet a crash inside a J3D method = raw-vs-fixed, not a "
              "missing resource."]
    # §251 (recipe 14, v4): arc-string resolver — a WW actor loading a model
    # from a SISTER actor's arc (Aryll: telescope from "Link") that the port
    # lacks = silent create-hang (cPhs_ERROR every frame; no crash, no log).
    call_arcs = sorted(set(re.findall(
        r'(?:getObjectIDRes|getObjectRes|acquireModelData\w*)\(\s*"(\w+)"',
        text)))
    var_arc_calls = [ln.strip()[:120] for ln in text.splitlines()
                     if re.search(r"(?:getObjectIDRes|getObjectRes|"
                                  r"acquireModelData\w*)\(\s*[A-Za-z_]", ln)]
    lines += ["", "## Arc-string resolution (§251 / §247 recipe 14 — the "
              "silent-create-hang check)", "",
              "| arc string in res call | own resLoad arc? | mounted? | verdict |",
              "|---|---|---|---|"]
    for a in call_arcs:
        own = a in arcs
        mounted = os.path.exists(os.path.join(MOD_ARCS, a + ".arc"))
        verdict = "" if (own or mounted) else \
            "**FOREIGN + UNMOUNTED — silent create-hang (§247 recipe 14): " \
            "rebundle the resource into the actor's own arc or mount this one**"
        if not own and mounted:
            verdict = "sister-arc idiom — verify mount ORDER (must be " \
                      "resident before this actor's Create)"
        lines.append(f"| `{a}` | {'yes' if own else 'NO'} | "
                     f"{'yes' if mounted else 'NO'} | {verdict} |")
    if var_arc_calls:
        lines += ["", "**[RUNTIME-CHECK] res calls with a VARIABLE arc arg "
                  "(cannot verify statically — trace at first run):**", ""]
        lines += [f"- `{c}`" for c in var_arc_calls[:10]]
    open(os.path.join(out, "res-manifest.md"), "w", encoding="utf-8").write(
        "\n".join(lines) + "\n")

    # --- §251 v4: CUTSCENE-NPC TIER (History's §247 Aryll saga) -----------
    # Which of this TU's l_objectName names appear in a donor .stb cast?
    stb_json = os.path.join(RECEIVER, "docs", "WW Linked", "fact-sheets",
                            "json", "stb-cast.json")
    casts, obj_rows = {}, {}
    if os.path.exists(stb_json):
        import json as _json
        cast_idx = _json.load(open(stb_json, encoding="utf-8"))
        prof_keys = set(re.findall(r"g_profile_(\w+)", text))
        dstage = open(os.path.join(DONOR_SRC, "d", "d_stage.cpp"),
                      encoding="utf-8", errors="replace").read()
        for m in re.finditer(r'OBJNAME\("([^"]+)",\s*(\w+),\s*([^)]+)\)', dstage):
            nm, proc, arg = m.groups()
            if re.sub(r"^fpcNm_|_e$", "", proc) in prof_keys:
                obj_rows[nm] = m.group(0)
                if nm in cast_idx:
                    casts[nm] = cast_idx[nm]
    is_cutscene = bool(casts)
    cl = [f"# Cutscene tier — tag `{tag}` "
          f"({'CUTSCENE ACTOR' if is_cutscene else 'not in any .stb cast'})", ""]
    if is_cutscene:
        cl += ["> **This actor is in a donor storyboard cast. Demo-path shims "
               "are LOAD-BEARING (§247): a NULL `demo_*`/`getDemo*`/"
               "`getP_Bt*` shim = crash / blank face / mis-texture at "
               "runtime. DO NOT green this port while any demo shim is "
               "NULL.** Apply cookbook recipes 10–15; native scripted-face "
               "trio is decoded at `docs/WW Linked/getP_BtpData-decode.md` "
               "(land that, don't re-shim).", "",
               "## .stb casts this actor appears in", ""]
        for nm, lst in sorted(casts.items()):
            cl += [f"**`{nm}`**:"] + [f"- `{w}`" for w in lst] + [""]
        cl += ["## Vanilla l_objectName row(s) — restore VERBATIM (recipe 4)",
               "",
               "The storyboard binds by `dStage_searchName(name)` → (proc, "
               "argument). `255 == (s8)-1` matches the population's "
               "`i_argument = -1`. Flag any existing `\"name\" → mount-host` "
               "row that shadows this."]
        cl += [f"- `{row}`" for nm, row in sorted(obj_rows.items())]
        cl += ["",
               "## Face-actor render order (recipe 6 sub-rule)", "",
               "`modelCalc()` re-runs material calc and RESETS the texture "
               "pattern — enter face anims AFTER it: `modelCalc → btp.entry "
               "→ btk.entry → setList → entryDL → btk.remove → btp.remove`.",
               "",
               "## Demo transform (recipe 5)", "",
               "`dDemo_setDemoData` must set **`current.angle = shape_angle`**"
               " (donor behavior; the port dropped it once — fixed for Aryll; "
               "verify still present)."]
    else:
        cl += ["No l_objectName of this TU appears in any donor `.stb` — "
               "demo shims are inert-safe (bm1 precedent, §247).",
               "", "Names checked: " +
               (", ".join(f"`{n}`" for n in sorted(obj_rows)) or
                "(no OBJNAME rows resolved — [INFERENCE-NEEDED])")]
    open(os.path.join(out, "cutscene.md"), "w", encoding="utf-8").write(
        "\n".join(cl) + "\n")

    # §263 (§248 ask #6 — pitfalls 8/9/10): placement status + multi-profile
    # registration template + res-coherence, one sheet.
    MOD = os.path.dirname(MOD_ARCS)
    reg = [f"# Registration & placement sheet — tag `{tag}` (§263)", ""]
    # --- pitfall 8: placement status per OBJNAME name --------------------
    amap_p = os.path.join(MOD, "population", "actor_map.ini")
    amap = (open(amap_p, encoding="utf-8", errors="replace").read()
            if os.path.exists(amap_p) else "")
    import glob as _glob
    csvs = _glob.glob(os.path.join(MOD, "population", "*.csv"))
    inis = _glob.glob(os.path.join(MOD, "npc", "*.ini"))
    ini_arcs = {}
    for ip in inis:
        it = open(ip, encoding="utf-8", errors="replace").read()
        m = re.search(r"^arc=(\w+)", it, re.M)
        if m:
            ini_arcs.setdefault(m.group(1), []).append(
                (os.path.basename(ip), it))
    reg += ["## Placement status (§248 pitfall 8) — socket-flip vs new-data, "
            "known UP FRONT", "",
            "| ACTR name | actor_map `[Name]` | placement CSV rows | actor "
            "ini (arc match) | status |", "|---|---|---|---|---|"]
    for nm in sorted(obj_rows) or ["(no OBJNAME names resolved)"]:
        if nm.startswith("("):
            reg.append(f"| {nm} | | | | |")
            continue
        has_map = bool(re.search(rf"^\[{re.escape(nm)}\]", amap, re.M))
        n_rows = sum(len(re.findall(rf"\b{re.escape(nm)}\b",
                     open(c, encoding="utf-8", errors="replace").read()))
                     for c in csvs)
        ini_hit = [f for a in arcs for f, _t in ini_arcs.get(a, [])]
        score = sum((has_map, n_rows > 0, bool(ini_hit)))
        status = ("**authored — spawn-wire is a socket FLIP**" if score == 3
                  else "none — needs NEW placement data (DZR-derived)"
                  if score == 0 else f"partial ({score}/3)")
        reg.append(f"| `{nm}` | {'yes' if has_map else 'NO'} | {n_rows} | "
                   f"{', '.join(ini_hit) or 'NO'} | {status} |")
    # --- pitfall 9: multi-profile registration block ---------------------
    profs_here = sorted(set(re.findall(r"g_profile_(\w+)", text)))
    fpc = open(os.path.join(RECEIVER, "include", "f_pc", "f_pc_name.h"),
               encoding="utf-8", errors="replace").read()
    slots = [int(m, 16) for m in re.findall(
        r"/\*\s*(0x[0-9A-Fa-f]+)\s*\*/\s*X\(fpcNm_", fpc)]
    nxt = (max(slots) + 1) if slots else 0
    done = [p for p in profs_here if f"fpcNm_{p}_e" in fpc]
    todo = [p for p in profs_here if p not in done]
    reg += ["", f"## Multi-profile registration (§248 pitfall 9) — "
            f"{len(profs_here)} profile(s); next free slot 0x{nxt:X}", ""]
    if done:
        reg += [f"**ALREADY REGISTERED (skip): "
                + ", ".join(f"`fpcNm_{p}_e`" for p in done) + "** — the "
                "blocks below cover only the unregistered profiles.", ""]
    profs_here = todo
    reg += ["> FOUR files, index-matched — an off-by-one in the array vs the "
            "enum silently mis-dispatches. Copy-paste, in this order:", "",
            "```cpp", "// include/f_pc/f_pc_name.h  (X-macro list, before the "
            "terminator)"]
    for i, p in enumerate(profs_here):
        reg.append(f"/* 0x{nxt+i:X} */ X(fpcNm_{p}_e) \\")
    reg += ["", "// include/f_pc/f_pc_profile_lst.h"]
    for p in profs_here:
        reg.append(f"extern actor_process_profile_definition g_profile_{p};"
                   f"  // {tag} direct port")
    reg += ["", "// src/f_pc/f_pc_profile_lst.cpp  (array — SAME ORDER as "
            "the enum slots above)"]
    for i, p in enumerate(profs_here):
        reg.append(f"&g_profile_{p}.base.base,  // MUST land at index "
                   f"fpcNm_{p}_e (0x{nxt+i:X})")
    reg += ["", "// files.cmake"]
    for s in srcs:
        if s.endswith(".cpp"):
            reg.append(f"        src/d/actor/{os.path.basename(s)}")
    reg += ["```"]
    # --- pitfall 10: res-coherence (header <-> cpp arc <-> ini arc) ------
    reg += ["", "## Res-coherence (§248 pitfall 10) — three-way agreement or "
            "a runtime NULL", "",
            "| arc | res header (assets) | cpp res-call strings agree | ini "
            "`arc=` agrees |", "|---|---|---|---|"]
    for a in arcs:
        hdr_ok = os.path.exists(os.path.join(
            RECEIVER, "assets", "GZ2E01", "res", "Object", a + ".h"))
        cpp_ok = (a in call_arcs) or not call_arcs
        ini_ok = a in ini_arcs
        reg.append(f"| `{a}` | {'yes' if hdr_ok else '**MISSING**'} | "
                   f"{'yes' if cpp_ok else '**drift — cpp uses: ' + ', '.join(call_arcs) + '**'} | "
                   f"{'yes (' + ini_arcs[a][0][0] + ')' if ini_ok else 'no ini yet'} |")
    open(os.path.join(out, "registration.md"), "w", encoding="utf-8").write(
        "\n".join(reg) + "\n")

    # §262 (§248 pitfalls 1+2): cascade graph + dormancy map — needs the
    # cutscene bit (DEMO-ONLY symbols are load-bearing iff .stb-cast actor).
    run("cascade_map.py", *srcs,
        *(["--cutscene"] if is_cutscene else []),
        "--out", os.path.join(out, "cascade.md"))

    # §251 v4 (recipe 2): ready-to-paste create-trace block
    open(os.path.join(out, "create_trace.h"), "w", encoding="utf-8").write(f"""\
#pragma once
// ============================================================
// §251 create-trace (kit v4, §247 recipe 2) — paste into the actor's
// _create/CreateHeap paths BEFORE first spawn attempt; a "spawns then
// vanishes" (cPhs_ERROR loop) becomes a one-build diagnosis.
// Toggle: -DDUSK_CREATE_TRACE=1. STRIP BEFORE PUSH (probe discipline).
// ============================================================
#if defined(DUSK_CREATE_TRACE) && DUSK_CREATE_TRACE
#define DUSK_CT(phase) OSReport("[DuskLog] ct tag={tag.lower()} phase=%s\\n", phase)
#define DUSK_CT_RES(what, p) \\
    OSReport("[DuskLog] ct tag={tag.lower()} res=%s ptr=%p\\n", what, (void*)(p))
#else
#define DUSK_CT(phase)
#define DUSK_CT_RES(what, p)
#endif
// Usage: DUSK_CT("resLoad-wait"); DUSK_CT("heap"); DUSK_CT("model");
//        DUSK_CT_RES("telescope.bdl", pModelData);  // after EVERY acquire
""")

    # --- rename dictionary pass ------------------------------------------
    lines = ["# Known WW→TP renames hit in this source (apply BEFORE compile 1)",
             "", "| donor | port | receipt | hits |", "|---|---|---|---|"]
    for old, (new, receipt) in RENAMES.items():
        n = len(re.findall(r"\b" + re.escape(old) + r"\b", text))
        if n:
            lines.append(f"| `{old}` | `{new}` | {receipt} | {n} |")
    lines.append("\n> Dictionary lives in enemy_port_kit.py RENAMES — add every "
                 "new port's compile-cascade discoveries with their bus §.")
    # §231: semantic-pattern audit (the symbolicated §229 gotcha classes)
    lines += ["", "## Pattern audit (§229 symbolicated gotchas — fix BEFORE "
              "first run, they are runtime AVs/invisibility, not compile "
              "errors)", ""]
    src_lines = text.splitlines()
    hits = 0
    for pat, why in PATTERNS:
        rx = re.compile(pat)
        first = True
        for i, ln in enumerate(src_lines, 1):
            if rx.search(ln):
                if first:
                    lines.append(f"### {why}")
                    first = False
                lines.append(f"- L{i} `{ln.strip()[:160]}`")
                hits += 1
        if not first:
            lines.append("")
    if hits == 0:
        lines.append("(no pattern hits — still run the §229 render triple: "
                     "modelCalc() → dComIfGd_setList() → entryDL())")
    open(os.path.join(out, "renames.md"), "w", encoding="utf-8").write(
        "\n".join(lines) + "\n")

    # --- v3: include closure + equivalence (closure.md) -------------------
    closure = include_closure(srcs, text)
    lines = ["# Include closure (§232 gap 1+2 — resolve BEFORE compile 1)", "",
             "| header | status | donor symbols the TU uses but receiver lacks |",
             "|---|---|---|"]
    n_stage = n_div = 0
    for rel, status, missing in closure:
        n_stage += status.startswith("DONOR-ONLY")
        n_div += status.startswith("NATIVE-DIVERGED")
        miss = ", ".join(f"`{m}`" for m in missing[:12]) if missing else ""
        lines.append(f"| `{rel}` | {status} | {miss} |")
    lines += ["", f"**{n_stage} donor-only headers to stage · {n_div} "
              "receiver-native headers DIVERGED (stub/partial — stage the "
              "missing symbols as a shared shim header; §232's "
              "c_damagereaction lesson).**"]
    open(os.path.join(out, "closure.md"), "w", encoding="utf-8").write(
        "\n".join(lines) + "\n")

    # --- shim skeleton from the surface stub sheet ------------------------
    surf = open(os.path.join(out, "surface.md"), encoding="utf-8").read()
    row = re.compile(r"^\| `(\w+)` \| \d+ \| (free|method:(\w+))[^|]*\| `([^`]+)`",
                     re.M)
    frees, methods, unknown = [], [], []
    for m in row.finditer(surf):
        name, kind, cls, sig = m.group(1), m.group(2), m.group(3), m.group(4)
        if kind == "free":
            frees.append((name, sig))
        else:
            methods.append((name, cls, sig))
    for m in re.finditer(r"^\| `(\w+)` \| \d+ \| UNKNOWN", surf, re.M):
        unknown.append(m.group(1))
    # v3 gap 3: type-aware pass — forward-declare donor-only types used by
    # pointer/ref in the skeleton; flag by-value uses.
    sys.path.insert(0, TOOLS)
    from api_surface import build_index
    idx = build_index([p for p in (os.path.join(RECEIVER, "src"),
                                   os.path.join(RECEIVER, "include"),
                                   os.path.join(RECEIVER, "libs"))
                       if os.path.isdir(p)])
    BUILTIN = {"void", "int", "bool", "float", "double", "char", "const",
               "unsigned", "signed", "short", "long", "u8", "u16", "u32",
               "u64", "s8", "s16", "s32", "s64", "f32", "f64", "BOOL",
               "size_t", "uintptr_t", "self", "TODO", "adapter", "for"}
    all_sigs = " ".join(s for _, s in frees) + " " + \
               " ".join(s for _, _, s in methods)
    fwd, flagged_types = set(), set()
    for tok in set(re.findall(r"[A-Za-z_]\w{2,}", all_sigs)):
        if tok in BUILTIN or tok in idx:
            continue
        if re.search(re.escape(tok) + r"\s*[*&]", all_sigs):
            fwd.add(tok)
        else:
            flagged_types.add(tok)

    h = [f"#pragma once",
         "// ============================================================",
         f"// AUTO-GENERATED shim skeleton — tag {tag} (enemy_port_kit §230;",
         "// pattern = History's §223/§225 d_ext_ww_actor_shims architecture:",
         "// FREE functions keep DONOR names (source stays verbatim);",
         "// CLASS-METHOD gaps become dExt" + tag + "_ ADAPTERS (call-site",
         "// adapted in Pass 2; libs/JSystem untouched).",
         "// Signatures are VERBATIM donor declarations (cited in surface.md).",
         "// Bodies: implement / no-op / sentinel per the pig receipts.",
         "// ============================================================", ""]
    if fwd:
        h.append("// --- v3: donor-only types used by pointer/ref — opaque "
                 "forward decls (stage the real defs per closure.md when the "
                 "shim body needs them) ---")
        h += [f"class {t};" for t in sorted(fwd)]
        h.append("")
    if flagged_types:
        h.append("// --- v3 [TYPE-GAP] donor-only types used BY VALUE — "
                 "forward decl is NOT enough; stage the definition "
                 "(closure.md) before these shims compile: ---")
        h += [f"//   {t}" for t in sorted(flagged_types)]
        h.append("")
    # §251 v4: demo-path shims are LOAD-BEARING for cutscene actors (§247)
    DEMO_SHIM = re.compile(r"(?i)demo|getP_Bt|JSG")
    def _lb(name):
        return ("  // ** LOAD-BEARING (cutscene actor, §247/§251): must be "
                "REAL, never NULL — see cutscene.md **"
                if is_cutscene and DEMO_SHIM.search(name) else "")
    # §261 (§248 pitfall 4): consult the shared shim ledger — reuse beats
    # rediscovery; a duplicate definition is a link error or divergence bug.
    from shim_ledger import ledger
    known = ledger()
    reuse = []
    def _ledger(name):
        hit = known.get(name) or known.get(f"dExt{tag}_{name}")
        if hit:
            reuse.append(name)
            return (f"// EXISTS — REUSE `{hit[1]}:{hit[2]}` (shim ledger "
                    f"§261), do NOT redefine:\n// ")
        return ""
    h.append("// --- FREE-FUNCTION shims (donor names) ---")
    for name, sig in frees:
        h.append(_ledger(name) + f"{sig.replace('inline ', '')};"
                 f"  // TODO body" + _lb(name))
    h.append("")
    h.append("// --- CLASS-METHOD adapters (call-site adapted in Pass 2) ---")
    for name, cls, sig in methods:
        ret = sig.split(name)[0].replace("virtual ", "").replace("inline ", "").strip() or "void"
        args = sig.split(name, 1)[1] if name in sig else "(/* see surface.md */)"
        args_inner = args[args.find("(") + 1:args.rfind(")")].strip()
        joined = (cls + "* self" + (", " + args_inner if args_inner else ""))
        h.append(_ledger(name) + f"{ret} dExt{tag}_{name}({joined});  "
                 f"// adapter for {cls}::{name}" + _lb(name))
    h.insert(h.index("// --- FREE-FUNCTION shims (donor names) ---"),
             f"// §261 LEDGER SUMMARY: {len(reuse)} of "
             f"{len(frees) + len(methods)} shims already exist "
             f"(_shims-ledger.md) — reuse them; "
             f"{len(frees) + len(methods) - len(reuse)} are new.\n")
    if unknown:
        h.append("")
        h.append("// --- UNKNOWN declarations ([INFERENCE-NEEDED] — locate first) ---")
        h += [f"// TODO locate: {n}" for n in unknown]
    open(os.path.join(out, "shims_skeleton.h"), "w", encoding="utf-8").write(
        "\n".join(h) + "\n")

    # --- README (the §229 recipe) ----------------------------------------
    open(os.path.join(out, "README.md"), "w", encoding="utf-8").write(f"""# Port kit — {tag}

The §229-proven pipeline (updated with the direct-port-plan.md render/crash
laws + the §232 dogfood gaps → v3). Order of work:
-1. **ISLAND CHECK (§239):** confirm the actor appears on YOUR island's roster
   (`port-kits/rosters/<island>.md`, or generate:
   `island_roster.py <stage> <arc> <out>`) — the roster joins placements ×
   donor l_objectName × g_profile source files. If the name is not on the
   roster, it does not belong on this island.
-1b. **CUTSCENE CHECK (§251, v4):** read `cutscene.md`. If the actor is in a
   donor `.stb` cast, it is a CUTSCENE actor: demo-path shims are
   LOAD-BEARING (a NULL shim = crash/blank-face at runtime, §247), cookbook
   recipes 10–15 apply, `create_trace.h` goes in before first spawn, and the
   port may NOT be greened while any demo shim is NULL — the native
   scripted-face path is `docs/WW Linked/getP_BtpData-decode.md`.
0. `closure.md` — the include-closure census (v3): stage every DONOR-ONLY
   header; for every NATIVE-DIVERGED header, stage the missing symbols
   (shared-shim candidates — §232's c_damagereaction lesson: the receiver
   header EXISTING does not mean it carries the donor's types).
0b. **`cascade.md` (§262) — READ BEFORE SEQUENCING:** the dormancy table says
   per missing symbol whether the path is LIVE (CORE = must be real),
   DEMO-ONLY (load-bearing iff cutscene actor), TALK-ONLY, or UNREACHED
   (inert stub faithful); the cascade table says which symbols are flat
   leaves vs chains and names the terminal subsystem a real port drags in —
   the sequencing decision (port now / defer / stub) is made HERE, not at
   compile errors.
1. **START FROM `ported_src/` (§259)** — the codemod has ALREADY applied the
   safe renames (report + receipts: `codemod-report.md`); work its REVIEW
   annotations by hand, then fix every `renames.md` **Pattern audit** hit —
   those are RUNTIME failures (invisibility/AVs), not compile errors.
1b. **`collisions.md` (§259, §248 pitfall 3 — the one check that can HURT
   you):** before ANY shim `#define`, read the verdict per absent donor
   constant: COLLISION = never define the donor value (shim to 0/free bit);
   RENAME-EQUIVALENT = apply the codemod name, don't define.
2. `res-manifest.md` — stage the mod arc + copy the donor res header if
   MISSING, and follow the per-res ROUTING column (§229 DN-3 law: models
   parse-at-consume via `dExtNpcMount_acquireModelData`, anims raw-OK,
   tex/mat never raw-called).
3. `shims_skeleton.h` — FIRST honor the §261 LEDGER SUMMARY line (shims marked
   EXISTS are already in the shared shim header — include it, never redefine);
   then fill the NEW bodies (no-op / sentinel / implement per pig receipts
   §225-§227). After adding shims: rerun `shim_ledger.py` so the next actor
   sees them. Symbols in surface.md's INCLUDE PLAN need an `#include`, not a
   shim (§248 pitfall 7); SRC-ONLY entries are the exception — treat as absent.
4. Drop the donor source in; compile; add NEW renames to the kit dictionary.
5. **RENDER LAW (§229 gotcha 1):** the port split donor `updateDL()` — every
   draw must run `modelCalc()` → `dComIfGd_setList()` → `entryDL()`
   (m_Do_ext.h:390; mount draw d_ext_npc_mount.cpp:9014/9398/9410).
   `play()`+`entryDL()` alone = invisible actor.
6. **64-BIT LAW (§229 gotcha 2):** no `(u32)`/`(s32)` pointer casts —
   `setUserArea((uintptr_t)this)`; the pattern audit lists every candidate.
6b. **CARRY LAW (§238, pig receipt):** if the actor is grabbable, the player's
   grab path forces carried `shape_angle.x` — donor carry poses (pig =
   upside-down) need a per-proc skip in the grab pitch handling.
6c. **WATER LAW (§238, pig receipt):** the port's water probe spans 300 units —
   too short for deep-sea actors; swimming/floating donor actors need the
   extended span (10000, pig precedent).
7. Spawn flip (§228 pattern): fpcNm registration + socket ini + actor_map arg
   (mask arg to valid shape range). Deliberate playtest — room-load crash is
   the failure mode; rollback = one line.
8. ACCEPTANCE: `DUSK_STATE_TAP=1` session →
   `state_gate.py <jsonl> statemap.md --tag {tag.lower()}` (§224 gate).
""")
    print(f"kit written to {out}: {len(frees)} free shims, {len(methods)} "
          f"adapters, {len(unknown)} unknown, {len(arcs)} arcs")

if __name__ == "__main__":
    main()
