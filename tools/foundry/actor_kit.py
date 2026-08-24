#!/usr/bin/env python3
# ============================================================================
# actor_kit.py — the Actor Kit driver (§329): one command per donor actor.
#
# Composes the §248 instruments into a port scaffold + review + registration:
#   1. donor TU + header pulled from the decomp (<decomp-root>/WW DP)
#   2. ww2tp_codemod AUTO rules applied; REVIEW notes carried into the report
#   3. registration PLAN computed against the live tree (f_pc_name.h index,
#      f_pc_profile_lst.h extern, f_pc_profile_lst.cpp table slot — index
#      asserted to match the enum — files.cmake, OBJNAME rows)
#   4. model arc(s) staged donor-verbatim (arcs/ + arcs_lib/, md5-receipted)
#   5. collision pre-flight (enum/profile/OBJNAME duplicates)
#   6. pitfall checklist (the §-catalog) + DN-registry autodetection:
#      dBgW/Regist in donor source => DN-1 escalation line, activation gated
#
# TWO-STAGE LANDING (the DN/coincidence split):
#   --land      TU + header + name enum + profile extern/table + files.cmake.
#               The actor COMPILES but nothing spawns it (no OBJNAME row) —
#               runtime-inert, safe to build/ship.
#   --activate  adds the OBJNAME row(s) — the actor goes LIVE. Gated on: DN
#               flags resolved (user ruling) + the room's coincidence rule.
#
# Default is scaffold-only (writes to scratchpad, touches nothing).
#
# ============================================================================
# ERA WARNING — RETOOLED 2026-08-21 (Foundry, user order: kits may still carry
# mount/bake/legs-era recipes).
#
# THIS KIT'S LANDING PIPELINE IS FORK-ERA. --land/--activate edit the FORK
# TREE (f_pc_name.h, f_pc_profile_lst, files.cmake, OBJNAME rows) — the
# delivery model the PLUGIN replaces. A user on stock dusklight NEVER receives
# a tree-landed actor. Under the standing plugin-side order:
#   · scaffold/review tiers remain valid for BOTH targets;
#   · --land/--activate are FORK-ONLY conveniences, never delivery;
#   · plugin delivery = the trace method (ww-plugin-outset.md §How ports
#     land) + a registry.cpp above-enum row. A PLUGIN-TARGET LANDING MODE IS
#     NOT BUILT YET — that gap is on Foundry's plate, named, not implied.
# Three checklist rules below were harvested from tree-side ports and have
# been ERA-SPLIT rather than deleted (fork guidance kept, plugin column
# added), because deleting a still-true fork recipe to fix a plugin gap is
# the substitution error inverted.
# ============================================================================
import hashlib
import re
import shutil
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from ww2tp_codemod import apply as codemod_apply

RECEIVER = Path("%USERPROFILE%/Documents/dusklight")
DONOR = Path("<decomp-root>/WW DP")
DONOR_OBJ = Path("<decomp-root>/Ex WW/files/res/Object")
# MOD is the FORK-ERA arc-staging target (model_replacements era) — used only
# by --land arc staging, kept for fork work, NOT a plugin delivery path.
MOD = Path("%USERPROFILE%/AppData/Roaming/TwilitRealm/Dusklight/"
            "model_replacements/WW-Crew-Restoration")
# SCRATCH was hardcoded to a DEAD session's scratchpad (found in the 2026-08-21
# era audit) — every scaffold written after that session died went to a
# directory nothing would ever read. Now session-independent.
import tempfile
SCRATCH = Path(tempfile.gettempdir()) / "actor_kit"

# the §-catalog: every landed-port pitfall, cited — the REVIEW checklist
CHECKLIST = [
    # ERA-SPLIT 2026-08-21: the mount recipe is FORK-ONLY. dExtNpcMount is
    # tree-side (57 files, absent from dusklight-main) and on Engine's do-not
    # list; a plugin port routed through it cannot even link. The plugin's
    # model path IS the receiver getRes choke point (trace method step 4).
    ("DN-3 model resolve [FORK]", "model via dExtNpcMount_acquireModelData(arc, "
     "bmd) — NOT dComIfG_getObjectRes+res-header (lamp §327 / toripost §253)"),
    ("DN-3 model resolve [PLUGIN]", "model via the receiver's own getRes choke "
     "point hooked in registry.cpp (lwood R2-R6 template) — dExtNpcMount does "
     "not exist on vanilla; MOUNT IS THE BANNED ERA, not a recipe"),
    ("cPhs/assert idiom", "cPhs_State -> cPhs_Step; JUT_ASSERT -> NULL-guard "
     "(port assert panics; donor retail assert compiles out)"),
    ("particle arity", "particle_set 6-arg -> 10-arg; setSimple 2-arg -> 7-arg "
     "(WW default args made explicit)"),
    ("TEV/lighting [FORK]", "settingTevStruct TEV_TYPE_ACTOR + "
     "setLightTevColorType — check §47 donor-look path on WW host stages"),
    # ERA-SPLIT: §47's host-stage machinery is tree-side. Plugin-side, TEV
    # types are receiver enums consumed through the same choke points the
    # trace method names — and TEV_TYPE_BG0 was an R4 authored-constant
    # failure (wrote 0, truth 1): NEVER hand-author the constant, read it
    # from the receiver header once E2 lands typed includes.
    ("TEV/lighting [PLUGIN]", "TEV type constants from receiver headers "
     "(typed, post-E2), applied at the modelEntryDL/consume boundary — no "
     "hand-authored TEV constants (R4 precedent)"),
    ("modelCalc order", "modelCalc BEFORE btp/face anim consume (pitfall C — "
     "recurring Aryll/Grandma render bug)"),
    ("staff-claim this", "any event staff registration passes the ACTOR ptr "
     "(§270 NULL-actor class — codemod detector should have flagged)"),
    ("anm_prm endianness", "rodata anim-param tables byte-order (§258 class)"),
    ("shims header [FORK]", "missing WW constants -> d_ext_ww_actor_shims.h "
     "if shared, LOCAL value-faithful #defines if TU-only (lamp banner)"),
    # ERA-SPLIT: d_ext_ww_actor_shims.h is a FORK-TREE file, not in
    # dusklight-main. A plugin TU including it fails on the target binary.
    ("shims header [PLUGIN]", "missing WW constants -> plugin-local shim "
     "header inside mods-src/ww_donor_disc/, value-faithful, donor-cited"),
]


def read(p):
    return Path(p).read_text(encoding="utf-8", errors="replace")


def md5(p):
    return hashlib.md5(Path(p).read_bytes()).hexdigest()


def donor_objname_rows(names):
    txt = read(DONOR / "src/d/d_stage.cpp")
    rows = {}
    for nm in names:
        m = re.search(r'OBJNAME\("%s",\s*(\w+),\s*(\d+)' % re.escape(nm), txt)
        if m:
            rows[nm] = (m.group(1), int(m.group(2)))
    return rows


def next_name_index():
    txt = read(RECEIVER / "include/f_pc/f_pc_name.h")
    idxs = [int(m.group(1), 16) for m in
            re.finditer(r"/\*\s*0x([0-9A-Fa-f]+)\s*\*/\s*X\(fpcNm_", txt)]
    return max(idxs) + 1


def profile_table_count():
    """Real entries in the LIVE g_fpcPfLst_ProfileList table (the last `= {`
    block), excluding the single NULL terminator — must equal the next
    f_pc_name.h enum index for positional lookup to hold."""
    txt = read(RECEIVER / "src/f_pc/f_pc_profile_lst.cpp")
    start = txt.rindex("g_fpcPfLst_ProfileList[] = {")
    body = txt[start:txt.index("};", start)]
    entries = re.findall(r"^\s+(?:&g_profile_\w+|NULL,|nullptr,)", body,
                         re.MULTILINE)
    return len(entries) - 1  # trailing NULL terminator


REPO = Path(__file__).resolve().parents[2]



def _stamp_lineage(txt, actor):
    """§427 — emit the §426 KIT-LINEAGE tag on every port this kit produces.

    Every port the kit generates is a DIRECT DONOR PORT by construction: the
    input is the donor TU and the output is that TU codemodded onto receiver
    calls. So `native-port` is the only honest default here, and stamping it is
    not a guess about the file — it is a statement about how the file was made.

    Two rules this deliberately does NOT break:
      * It never RE-tags. If a human later reclassifies a TU (a port that turned
        out to be imitation machinery is `bridge-owed`), a re-run must not
        silently overwrite that judgement.
      * It never invents `bridge-owed` or `host-plumbing`. Those are lineage
        RULINGS about intent, and the kit has no standing to make them — the
        same reason the lint refuses to infer lineage from prose.

    Without this, a freshly generated port lands untagged, the §426 pre-flight
    reads UNKNOWN, and the kit blocks its own output on a tag only it was in a
    position to write.
    """
    if "// KIT-LINEAGE:" in txt:
        return txt
    # The tag line stays EXACTLY the ratified spelling and nothing else.
    # First attempt appended the rationale to the same line, and the lint's
    # grep-exact regex (which anchors to end-of-line, as §426 requires) could
    # not read the kit's own output. Explanation goes BELOW the tag.
    tag = ("// KIT-LINEAGE: native-port" + chr(10)
           + "// §427 stamped by actor_kit: this file is the donor TU "
             "codemodded onto" + chr(10)
           + "// receiver calls, so native-port is a statement about how it was "
             "made, not" + chr(10)
           + "// a guess. Reclassify BY HAND if it turns out to be imitation "
             "machinery" + chr(10)
           + "// rather than the donor's own behaviour — the kit never "
             "overwrites that." + chr(10))
    lines = txt.splitlines(keepends=True)
    ins = 0
    for i, l in enumerate(lines[:80]):
        s = l.strip()
        if s.endswith("*/"):
            ins = i + 1
            break
        if s.startswith("#include") or s.startswith("#ifndef"):
            ins = i
            break
    lines.insert(ins, tag)
    return "".join(lines)


def _emit_plugin_landing(actor):
    """--land-plugin: the plugin-era counterpart of _apply_registrations.

    WHAT THE FORK PATH EDITS (f_pc_name.h / profile lst / files.cmake /
    OBJNAME rows), THE PLUGIN PATH EXPRESSES AS REGISTRY ROWS: an above-enum
    index const (`kWwProfileBase + N`), a profile-table row, and placement
    rows in `kObjectNames` -- the only path that reaches a user's own
    unmodified binary. This emits all three, derived not guessed:

      - N = max existing `kWwProfileBase + n` in registry.cpp, plus one
      - placement rows from the DONOR'S OWN OBJNAME table (never authored)
      - insertion anchors located by grep AT EMIT TIME (line numbers drift)

    What it CANNOT emit is the profile itself: `s_<actor>Profile` is a
    trace-method port (donor profile -> method table -> create/draw), not a
    table row. The block carries that as an explicit TODO naming the donor
    symbols, because a generator that stubbed a profile would be authoring
    exactly what DN-10 says must be ported."""
    reg = Path("%USERPROFILE%/Documents/dusklight/mods-src/ww_donor_disc/registry.cpp")
    txt = read(reg)
    used = [int(m) for m in re.findall(r"kWwProfileBase \+ (\d+)", txt)]
    nxt = max(used) + 1 if used else 0
    short = actor[4:] if actor.startswith("d_a_") else actor
    camel = "".join(w.capitalize() for w in short.split("_"))
    names_arg = ""
    if "--objnames" in sys.argv:
        names_arg = sys.argv[sys.argv.index("--objnames") + 1]
    names = [n for n in names_arg.split(",") if n]
    rows = donor_objname_rows(names) if names else {}
    missing = [n for n in names if n not in rows]

    block = []
    block.append("// ==== %s: PLUGIN LANDING (generated by actor_kit --land-plugin) ====" % actor)
    block.append("// Paste targets are ANCHORS below -- registry.cpp is Engine's file;")
    block.append("// this block was EMITTED, not applied. Trace method governs the port.")
    block.append("const short k%sIndex = kWwProfileBase + %d;  // %d" % (camel, nxt, 4096 + nxt))
    block.append("// TODO (trace method, NOT generatable): port s_%sProfile from the" % short)
    block.append("//   donor's own profile (d_a_%s: l_daPROC_%s_Method chain) -- a" % (short, camel))
    block.append("//   generator that stubbed this would AUTHOR what DN-10 says to PORT.")
    block.append("// profile-table row (anchor: s_profiles):")
    block.append("    {k%sIndex, static_cast<const void*>(&s_%sProfile), true}," % (camel, short))
    if rows:
        block.append("// placement rows (anchor: kObjectNames), donor-cited:")
        for nm, (proc, arg) in rows.items():
            block.append('    // Donor d_stage.cpp OBJNAME("%s", %s, %s).' % (nm, proc, arg))
            block.append('    {"%s", k%sIndex, %s},' % (nm, camel, arg))
    if missing:
        block.append("// !! NO DONOR OBJNAME ROW for: %s -- NOT emitted (never authored)." % ",".join(missing))
    out = SCRATCH / actor
    out.mkdir(parents=True, exist_ok=True)
    dest = out / "land_plugin_registry_block.txt"
    dest.write_text("\n".join(block) + "\n", encoding="utf-8")

    def anchor(pat):
        for i, ln in enumerate(txt.splitlines(), 1):
            if pat in ln:
                last = i
        return last
    print("PLUGIN LANDING BLOCK EMITTED (nothing touched): %s" % dest)
    print("  next above-enum index : kWwProfileBase + %d  (derived from %d existing)" % (nxt, len(used)))
    print("  anchors in registry.cpp AS OF NOW (re-grep before pasting, lines drift):")
    print("    index consts region ~ line %d (last kWwProfileBase use)" % anchor("kWwProfileBase +"))
    print("    profile table end   ~ line %d" % anchor("&s_"))
    print("    kObjectNames end    ~ line %d" % anchor("kObjectNames["))
    if missing:
        print("  WARNING: %d name(s) have no donor OBJNAME row and were NOT emitted." % len(missing))
    print("  The profile itself is a TRACE-METHOD PORT -- see the TODO in the block.")
    return 0



def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if not args:
        sys.exit("usage: actor_kit.py <d_a_xxx> [--objnames A,B] [--land] "
                 "[--activate] [--land-plugin]")
    actor = args[0]
    land = "--land" in sys.argv
    activate = "--activate" in sys.argv
    # ========================================================================
    # PLUGIN-TARGET LANDING (era retool follow-through, 2026-08-21). Emits a
    # PASTE-READY registry block and TOUCHES NOTHING -- registry.cpp is
    # Engine's shipping file; Foundry emits, the owning lane lands. Runs
    # BEFORE the fork-era gates because it writes no tree file for them to
    # protect.
    # ========================================================================
    if "--land-plugin" in sys.argv:
        return _emit_plugin_landing(sys.argv[1])

    # ========================================================================
    # §426 BLOCKING PRE-FLIGHT — the five sky-campaign laws (§423), enforced.
    #
    # Green-lit by the user once lineage tags landed, so N/A verdicts are
    # truthful from the very first gated run rather than a guess about scope.
    #
    # It gates --land and --activate, NOT analysis: running the kit to look at a
    # donor actor stays free. What is refused is WRITING a port into the tree
    # while a law is broken — which is the only moment the refusal is worth
    # anything.
    #
    # A missing // KIT-LINEAGE tag reads UNKNOWN and also blocks: an untagged TU
    # cannot have its laws scoped, so a PASS from it would not be evidence.
    # ========================================================================
    if land or activate:
        import kit_laws
        targets = [REPO / f"src/d/actor/{actor}.cpp"]
        targets = [q for q in targets if q.is_file()]
        if targets:
            blocked = []
            for q in targets:
                rows, lin, ref = kit_laws.check(q)
                for name, verdict, why in rows:
                    if verdict in ("VIOLATION", "UNKNOWN"):
                        blocked.append((q.name, name, verdict, why))
            if blocked:
                print("\n=== §426 PRE-FLIGHT REFUSED — the five laws (§423) ===")
                for fn, law, verdict, why in blocked:
                    print(f"  [{verdict}] {fn}: law {law}")
                    print(f"      {why}")
                print("\nNothing was written. Fix the law (or declare the lineage) and re-run.\nAnalysis without --land/--activate is never gated.")
                return 3
            print("§426 pre-flight: all five laws satisfied for "
                  + ", ".join(q.name for q in targets))
    objnames = []
    if "--objnames" in sys.argv:
        objnames = sys.argv[sys.argv.index("--objnames") + 1].split(",")

    src_tu = DONOR / f"src/d/actor/{actor}.cpp"
    src_h = DONOR / f"include/d/actor/{actor}.h"
    assert src_tu.is_file(), f"donor TU missing: {src_tu}"
    out = SCRATCH / actor
    out.mkdir(parents=True, exist_ok=True)
    rep = [f"# Actor Kit — {actor}", ""]

    donor_txt = read(src_tu)

    # ---- DN autodetection -------------------------------------------------
    dn_flags = []
    if re.search(r"\bdBgW\b|\bRegist\s*\(", donor_txt):
        dn_flags.append("**DN-1 SURFACE: donor registers BG/collision (dBgW/"
                        "Regist).** Activation requires the user's ruling — "
                        "room id must come from the loader-stamped actor room "
                        "(data-authored row), never guessed. LAND (inert) is "
                        "safe; ACTIVATE is gated.")
    if re.search(r"JPABaseEmitter|particle_set|setSimple", donor_txt):
        dn_flags.append("particle calls present — port the donor's CALL "
                        "verbatim (§209 lesson: receiver-call divergence, "
                        "never donor data).")

    # ---- codemod ----------------------------------------------------------
    new_txt, counts, notes = codemod_apply(donor_txt)
    new_txt = _stamp_lineage(new_txt, actor)
    (out / f"{actor}.cpp").write_text(new_txt, encoding="utf-8")
    if src_h.is_file():
        h_txt, h_counts, h_notes = codemod_apply(read(src_h))
        (out / f"{actor}.h").write_text(h_txt, encoding="utf-8")
        counts += h_counts
        notes += h_notes

    # ---- registration plan ------------------------------------------------
    prof = re.search(r"g_profile_(\w+)", donor_txt)
    prof_sym = f"g_profile_{prof.group(1)}" if prof else "??"
    donor_name_h = read(DONOR / "include/f_pc/f_pc_name.h")
    enum_m = re.search(r"fpcNm_%s_e" % prof.group(1), donor_name_h) if prof \
        else None
    enum_sym = f"fpcNm_{prof.group(1)}_e" if prof else "??"
    idx = next_name_index()
    tbl = profile_table_count()
    rows = donor_objname_rows(objnames)

    port_name_h = read(RECEIVER / "include/f_pc/f_pc_name.h")
    port_lst_h = read(RECEIVER / "include/f_pc/f_pc_profile_lst.h")
    port_stage = read(RECEIVER / "src/d/d_stage.cpp")
    clashes = []
    if enum_sym in port_name_h:
        clashes.append(f"enum {enum_sym} already in port f_pc_name.h")
    if prof_sym in port_lst_h:
        clashes.append(f"profile {prof_sym} already declared")
    for nm in objnames:
        if re.search(r'OBJNAME\("%s"' % re.escape(nm), port_stage):
            clashes.append(f'OBJNAME "{nm}" already registered')

    rep += ["## DN / escalation flags", ""] + \
        ([f"- {f}" for f in dn_flags] or ["- none detected"]) + [""]
    # ERA-LABELLED 2026-08-21 (History/Bridge caught it on Salvage step 5):
    # this line was the ONE registration-plan entry with no [FORK]/[PLUGIN]
    # split, unlike its checklist neighbours -- a MISMATCH here reads as a
    # halt on the PLUGIN path when the plugin never touches f_pc_profile_lst
    # (registry.cpp:1335 kWwProfileBase / :1338 isWwProcName / :180
    # fpcPf_Get hook rides the OOB guard instead). The tempting "fix" is a
    # receiver enum edit to satisfy an instrument -- exactly the mutation
    # DN-10 forbids. [PLUGIN] line makes that impossible to reach for.
    rep += ["## Registration plan", "",
            f"- enum: `X({enum_sym})` at index 0x{idx:X} (f_pc_name.h append) [FORK]",
            f"- profile table: `&{prof_sym}.base.base` as entry #{tbl} "
            f"(f_pc_profile_lst.cpp) — **assert {tbl} == 0x{idx:X} "
            f"({'OK' if tbl == idx else 'MISMATCH — STOP'})** [FORK — this STOP "
            f"does not bind a plugin-side port]",
            f"- [PLUGIN] registration: `kWwProfileBase`-relative index via "
            f"registry.cpp (`--land-plugin`); f_pc_profile_lst.cpp is never "
            f"written and this enum's numeric VALUE is not used for placement "
            f"— but donor code that REFERENCES {enum_sym} BY NAME (creates, "
            f"searches, proc-name compares) still needs translation to the "
            f"plugin-assigned proc name at the consumption boundary (DN-10 "
            f"step 2). Do not edit the receiver enum to satisfy this assert.",
            f"- extern: `extern actor_process_profile_definition {prof_sym};` [FORK]",
            f"- files.cmake: `src/d/actor/{actor}.cpp` [FORK]",
            "- OBJNAME rows (ACTIVATE stage only) [FORK]:"] + \
        [f"  - `OBJNAME(\"{nm}\", {enum_sym}, {sub})`  (donor proc {dproc})"
         for nm, (dproc, sub) in rows.items()] + [""]
    rep += ["## Collision pre-flight", ""] + \
        ([f"- **{c}**" for c in clashes] or ["- clean"]) + [""]

    # ---- model arc staging ------------------------------------------------
    arcs = sorted(set(re.findall(r'getObjectRes\("(\w+)"', donor_txt) +
                      re.findall(r'"(\w+)\.arc"', donor_txt)))
    rep += ["## Model arcs", ""]
    # §375 ARC-NAME SHADOW GUARD. Staging a donor arc whose NAME also exists in
    # the receiver silently re-points the TP actor of the same name at WW data
    # (TP res indices vs WW archive => wrong members or NULL => crash). Cost:
    # the §374/§375 Ep crash, twice. Never stage a colliding name; report it and
    # require an alias decided with the twin's port.
    # §388 RETRACTED (user pulled the ruling back — it was made without the
    # user having the information to judge it, and Foundry should not have
    # self-approved it). Until a ruling exists, EVERY colliding arc name is
    # reported and NOTHING is auto-allowed: the guard stays maximally cautious.
    INTENDED_WW_ARCS = set()
    tp_arc_names = set()
    stage_src = read(RECEIVER / "src/d/d_stage.cpp")
    for m in re.finditer(r'dComIfG_resLoad\([^,]+,\s*"(\w+)"', stage_src):
        tp_arc_names.add(m.group(1))
    for tu in (RECEIVER / "src/d/actor").glob("d_a_*.cpp"):
        try:
            t = tu.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        for m in re.finditer(r'(?:dComIfG_resLoad\([^,]+,|getObjectRes\()\s*"(\w+)"', t):
            tp_arc_names.add(m.group(1))
    for a in arcs:
        if a in INTENDED_WW_ARCS:
            rep.append(f"- `{a}.arc` staged — INTENDED WW load (the receiver's "
                       f"actor at this name IS the WW port; §388 ruling)")
        if a in tp_arc_names and a not in INTENDED_WW_ARCS:
            rep.append(f"- `{a}.arc` **NOT STAGED — ARC-NAME SHADOW**: the receiver "
                       f"already loads an arc named `{a}` (TP actor of the same "
                       f"name would read WW data). Stage under an alias when the "
                       f"WW twin lands.")
            continue
        srcarc = DONOR_OBJ / f"{a}.arc"
        if not srcarc.is_file():
            rep.append(f"- `{a}.arc` — NOT in donor Object dir (resolve by hand)")
            continue
        h = md5(srcarc)
        for dest in (MOD / "arcs", MOD / "arcs_lib"):
            d = dest / f"{a}.arc"
            if not d.is_file():
                shutil.copy2(srcarc, d)
        rep.append(f"- `{a}.arc` staged x2, md5 `{h[:12]}…` (donor-verbatim)")
    rep.append("")

    # ---- codemod + checklist ---------------------------------------------
    rep += ["## Codemod (AUTO applied)", ""] + \
        [f"- `{r[:70]}` ×{n} — {rec}" for r, n, rec in counts] + [""]
    if notes:
        rep += ["## Codemod REVIEW notes (hand pass)", ""] + \
            [f"- L{i}: `{ln}` — {adv}" for i, ln, adv in notes] + [""]
    rep += ["## Pitfall checklist (§-catalog — tick each)", ""] + \
        [f"- [ ] **{t}** — {d}" for t, d in CHECKLIST]

    (out / "report.md").write_text("\n".join(rep) + "\n", encoding="utf-8")
    print(f"scaffold -> {out}")
    for line in rep:
        if "MISMATCH" in line or line.startswith("- **DN") or \
           (line.startswith("- **") and "already" in line):
            print("!!", line)

    # --activate on an already-landed actor EXPECTS enum/profile present;
    # only an OBJNAME duplicate is a real collision for activation.
    blocking = clashes if land else [c for c in clashes if "OBJNAME" in c]
    if activate and not land:
        assert enum_sym in port_name_h, \
            f"--activate but {enum_sym} not landed — run --land first"
    if blocking and (land or activate):
        sys.exit(f"collisions present — refusing: {blocking}")

    if land:
        # TU + header into the tree; registrations; files.cmake. NO OBJNAME.
        shutil.copy2(out / f"{actor}.cpp", RECEIVER / f"src/d/actor/{actor}.cpp")
        if (out / f"{actor}.h").is_file():
            shutil.copy2(out / f"{actor}.h",
                         RECEIVER / f"include/d/actor/{actor}.h")
        _apply_registrations(actor, enum_sym, prof_sym, idx, tbl)
        print(f"LANDED (inert): {actor} — build to verify; no OBJNAME row yet")
    if activate:
        _apply_objnames(rows, enum_sym)
        print(f"ACTIVATED: OBJNAME rows for {list(rows)}")
    return 0


def _apply_registrations(actor, enum_sym, prof_sym, idx, tbl):
    assert tbl == idx, f"profile table count {tbl} != enum index {idx}"
    p = RECEIVER / "include/f_pc/f_pc_name.h"
    t = read(p)
    last = list(re.finditer(r"/\*\s*0x[0-9A-Fa-f]+\s*\*/\s*X\(fpcNm_\w+\) \\\n", t))[-1]
    t = t[:last.end()] + f"/* 0x{idx:X} */ X({enum_sym}) \\\n" + t[last.end():]
    p.write_text(t, encoding="utf-8")

    p = RECEIVER / "include/f_pc/f_pc_profile_lst.h"
    t = read(p)
    last = list(re.finditer(
        r"extern actor_process_profile_definition[^\n]*\n", t))[-1]
    t = t[:last.end()] + (f"extern actor_process_profile_definition {prof_sym};"
                          f"  // §329 Actor-Kit port ({actor})\n") + t[last.end():]
    p.write_text(t, encoding="utf-8")

    p = RECEIVER / "src/f_pc/f_pc_profile_lst.cpp"
    t = read(p)
    last = list(re.finditer(r"    &g_profile_\w+[^\n]*\n", t))[-1]
    t = t[:last.end()] + (f"    // §329 Actor-Kit: index MUST match {enum_sym} "
                          f"(0x{idx:X}).\n    &{prof_sym}.base.base,\n") + \
        t[last.end():]
    p.write_text(t, encoding="utf-8")

    p = RECEIVER / "files.cmake"
    t = read(p)
    anchor = "        src/d/actor/d_a_npc_ba1.cpp\n"
    assert anchor in t, "files.cmake anchor missing"
    t = t.replace(anchor, anchor + f"        src/d/actor/{actor}.cpp\n", 1)
    p.write_text(t, encoding="utf-8")


def _apply_objnames(rows, enum_sym):
    p = RECEIVER / "src/d/d_stage.cpp"
    t = read(p)
    anchor = re.search(r'    OBJNAME\("Ba1",[^\n]*\n', t)
    add = "".join(f'    OBJNAME("{nm}", {enum_sym}, {sub}),  // §329 Actor-Kit\n'
                  for nm, (_, sub) in rows.items())
    t = t[:anchor.end()] + add + t[anchor.end():]
    p.write_text(t, encoding="utf-8")


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
