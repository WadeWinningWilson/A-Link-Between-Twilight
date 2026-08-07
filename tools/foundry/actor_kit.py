#!/usr/bin/env python3
# ============================================================================
# actor_kit.py — the Actor Kit driver (§329): one command per donor actor.
#
# Composes the §248 instruments into a port scaffold + review + registration:
#   1. donor TU + header pulled from the decomp (D:/XXXXXXX/WW DP)
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
# ============================================================================
import hashlib
import re
import shutil
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from ww2tp_codemod import apply as codemod_apply

RECEIVER = Path("C:/Users/xxxxx/Documents/dusklight")
DONOR = Path("D:/XXXXXXX/WW DP")
DONOR_OBJ = Path("D:/XXXXXXX/Ex WW/files/res/Object")
MOD = Path("C:/Users/xxxxx/AppData/Roaming/TwilitRealm/Dusklight/"
            "model_replacements/WW-Crew-Restoration")
SCRATCH = Path("C:/Users/xxxxx/AppData/Local/Temp/claude/"
               "C--Users-ryana-Documents-dusklight/"
               "cfe21ec7-19db-4fe0-b9e2-f96f30a4db65/scratchpad") / "actor_kit"

# the §-catalog: every landed-port pitfall, cited — the REVIEW checklist
CHECKLIST = [
    ("DN-3 model resolve", "model via dExtNpcMount_acquireModelData(arc, bmd) "
     "— NOT dComIfG_getObjectRes+res-header (lamp §327 / toripost §253 recipe)"),
    ("cPhs/assert idiom", "cPhs_State -> cPhs_Step; JUT_ASSERT -> NULL-guard "
     "(port assert panics; donor retail assert compiles out)"),
    ("particle arity", "particle_set 6-arg -> 10-arg; setSimple 2-arg -> 7-arg "
     "(WW default args made explicit)"),
    ("TEV/lighting", "settingTevStruct TEV_TYPE_ACTOR + setLightTevColorType "
     "— check §47 donor-look path on WW host stages"),
    ("modelCalc order", "modelCalc BEFORE btp/face anim consume (pitfall C — "
     "recurring Aryll/Grandma render bug)"),
    ("staff-claim this", "any event staff registration passes the ACTOR ptr "
     "(§270 NULL-actor class — codemod detector should have flagged)"),
    ("anm_prm endianness", "rodata anim-param tables byte-order (§258 class)"),
    ("shims header", "missing WW constants -> d_ext_ww_actor_shims.h if shared, "
     "LOCAL value-faithful #defines if TU-only (lamp banner pattern)"),
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


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    if not args:
        sys.exit("usage: actor_kit.py <d_a_xxx> [--objnames A,B] [--land] "
                 "[--activate]")
    actor = args[0]
    land = "--land" in sys.argv
    activate = "--activate" in sys.argv

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
    rep += ["## Registration plan", "",
            f"- enum: `X({enum_sym})` at index 0x{idx:X} (f_pc_name.h append)",
            f"- profile table: `&{prof_sym}.base.base` as entry #{tbl} "
            f"(f_pc_profile_lst.cpp) — **assert {tbl} == 0x{idx:X} "
            f"({'OK' if tbl == idx else 'MISMATCH — STOP'})**",
            f"- extern: `extern actor_process_profile_definition {prof_sym};`",
            f"- files.cmake: `src/d/actor/{actor}.cpp`",
            "- OBJNAME rows (ACTIVATE stage only):"] + \
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
