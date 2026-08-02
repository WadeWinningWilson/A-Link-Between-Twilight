"""Foundry §259 (kit v4, §248 pitfalls 5+6 — History's #2 ranked ask):
the WW->TP RENAME CODEMOD — renames.md as an APPLIED transformation, not a
reading list. Every rule is a §243-§247 campaign receipt (History's recurring
delta set, HISTORY-KIT-FEEDBACK.md).

Two rule classes, honest by design:
  AUTO   — token-guarded, safe to apply mechanically (guards encode the known
           hazards, e.g. `J3DNode*` matches the POINTER token only, so the
           unrelated enum J3DNodeCBCalcTiming_In survives — §248 pitfall 6).
  REVIEW — type-blind or arity-changing deltas: NEVER auto-applied; the line
           is annotated in the report for the porter's hand pass.

Also exports port_name(donor_ident) so value_collision.py can reclassify a
same-value "collision" that is really the RENAMED equivalent (fopMsgStts_X ==
fopMsg_MODE_X by design — §259 coupling).

Usage: ww2tp_codemod.py <src> [src2 ...] --out-dir <dir>
Writes <dir>/<basename> (transformed) + <dir>/codemod-report.md.
"""
import os, re, sys

# --- identifier renames (also consumed by value_collision.port_name) --------
IDENT_RENAMES = {
    "cPhs_State": "cPhs_Step",
    "ARRAY_SSIZE": "ARRAY_SIZE",
    # fopMsgStts_* -> fopMsg_MODE_* (divergent spellings receipted from the
    # §259 collision table; generic prefix rule covers the 1:1 rest)
    "fopMsgStts_MSG_CONTINUES_e": "fopMsg_MODE_MSG_CONTINUE_e",
    "fopMsgStts_MSG_ENDS_e": "fopMsg_MODE_MSG_END_e",
    "fopMsgStts_UNKD_e": "fopMsg_MODE_UNK_D_e",
}
PREFIX_RENAMES = [("fopMsgStts_", "fopMsg_MODE_")]

def port_name(donor_ident):
    if donor_ident in IDENT_RENAMES:
        return IDENT_RENAMES[donor_ident]
    for old, new in PREFIX_RENAMES:
        if donor_ident.startswith(old):
            return new + donor_ident[len(old):]
    return donor_ident

# --- AUTO rules: (regex, replacement, receipt) ------------------------------
AUTO = [
    # §329 Actor-Kit first-run harvest (Otble): port Bgsp() returns a
    # REFERENCE (WW returns pointer); tevstr member renames (alink receipts).
    (r"dComIfG_Bgsp\(\)->", "dComIfG_Bgsp().",
     "port Bgsp() is a reference (alink :5722 idiom)"),
    (r"CrrPos\(\*dComIfG_Bgsp\(\)\)", "CrrPos(dComIfG_Bgsp())",
     "same — deref removed"),
    (r"tevStr\.mRoomNo", "tevStr.room_no", "TP tevstr member (alink :5711)"),
    (r"tevStr\.mEnvrIdxOverride", "tevStr.YukaCol",
     "TP tevstr member (alink :5722)"),

    (r"\(\s*u32\s*\)\s*(this|i_this)\b", r"(uintptr_t)\1",
     "64-BIT LAW §229: pointer puns through u32 truncate on x86_64"),
    (r"\bJ3DNode\s*\*", "J3DJoint*",
     "node-CB J3DNode*->J3DJoint* — POINTER token only (J3DNodeCBCalcTiming_In "
     "guard, §248 pitfall 6)"),
    (r"tevStr\.mRoomNo\b", "tevStr.room_no", "tevStr member rename (§248 #5)"),
    (r"tevStr\.mEnvrIdxOverride\b", "tevStr.YukaCol",
     "tevStr member rename (§248 #5)"),
    (r"dComIfG_Bgsp\(\)\s*->", "dComIfG_Bgsp().",
     "port Bgsp accessor returns reference (§248 #5)"),
    (r"\bgetTextureSRT\(\)", "getTexMtxInfo().mSRT",
     "J3D accessor split (§248 #5)"),
    (r"dComIfGp_evmng_getEventIdx\(([^,()]+)\)",
     r"dComIfGp_evmng_getEventIdx(\1, 0xFF)",
     "port evmng_getEventIdx takes (name, 0xFF) (§248 #5) — 1-arg form only"),
    (r"setPlayerPosAndAngle\(([^,()]+),([^,()]+)\)",
     r"setPlayerPosAndAngle(\1,\2, 0)",
     "port takes a third arg 0 (§248 #5) — 2-arg form only"),
]

# --- REVIEW rules: (regex, advice) — annotated, never applied ---------------
REVIEW = [
    # §329 harvest: port GetTriPla is bool(polyInfo, cM3dGPla*) — WW's
    # returns-pointer form needs the alink :5770 out-param idiom by hand.
    (r"GetTriPla\([^,)]*\)\s*->", "port GetTriPla takes (polyInfo, cM3dGPla*) "
     "and returns bool — rewrite per alink :5770; do not deref"),

    (r"->mStatus\b|\.mStatus\b",
     "msg_class::mStatus -> .mode in the port — TYPE-BLIND rename: confirm the "
     "receiver object is the msg class, then rename by hand"),
    (r"\.mRoomNo\b",
     "mRoomNo -> room_no only on tevStr (auto-applied there); other receivers: "
     "verify the type first"),
    (r"\bmDoHIO_\w*entry\w*\(|\bremoveHIO|\bentryHIO",
     "HIO entry/remove is #if DEBUG-only in the port — wrap or drop (§248 #5)"),
    (r"\.init\((?:[^();]*,){7,}[^()]*\)",
     "btp/btk anim init is 9->7 args in the port (drop trailing modify flag "
     "pair) — re-count this call's args by hand"),
    (r"dComIfGp_evmng_getMyStaffId\(\s*[^,()]+\s*\)",
     "port signature is (name, actor, 0) — needs the actor expression, "
     "cannot auto-fill"),
    (r"dComIfGp_evmng_getMyStaffId\([^,()]+,\s*NULL",
     "§270 KNOWN BUG: NULL actor -> staff claim FAILS -> actor never joins "
     "its event -> event completes at frame 0 (ba1 tale freeze). The actor "
     "expression is `this` in the NPC-template isEventEntry shape — fix it"),
    (r"dDemo_setDemoData\(",
     "donor 6-arg -> port 8-arg — add the two trailing args per the seagull/"
     "Aryll call sites"),
    (r"\bfopAcM_ct_Retail\b",
     "History's delta list maps this to the port's ct path — confirm the "
     "call form at a prior port's call site before renaming"),
]

def apply(text):
    counts, notes = [], []
    for name, new in IDENT_RENAMES.items():
        text, n = re.subn(r"\b" + re.escape(name) + r"\b", new, text)
        if n:
            counts.append((name + " -> " + new, n, "identifier rename"))
    for old, new in PREFIX_RENAMES:
        text, n = re.subn(r"\b" + re.escape(old) + r"(\w+)", new + r"\1", text)
        if n:
            counts.append((old + "* -> " + new + "*", n, "prefix rename"))
    for rx, rep, receipt in AUTO:
        text, n = re.subn(rx, rep, text)
        if n:
            counts.append((rx, n, receipt))
    for i, ln in enumerate(text.splitlines(), 1):
        for rx, advice in REVIEW:
            if re.search(rx, ln):
                notes.append((i, ln.strip()[:100], advice))
    return text, counts, notes

def main():
    argv = sys.argv[1:]
    out_dir = argv[argv.index("--out-dir") + 1]
    srcs = argv[:argv.index("--out-dir")]
    os.makedirs(out_dir, exist_ok=True)
    report = ["# Codemod report (§248 pitfalls 5+6 — AUTO applied, REVIEW "
              "annotated)", ""]
    for s in srcs:
        text = open(s, encoding="utf-8", errors="replace").read()
        new, counts, notes = apply(text)
        base = os.path.basename(s)
        open(os.path.join(out_dir, base), "w", encoding="utf-8").write(new)
        report.append(f"## `{base}`\n")
        report.append("| AUTO rule | hits | receipt |\n|---|---|---|")
        report += [f"| `{r[:60]}` | {n} | {rec} |" for r, n, rec in counts] \
            or ["| (none hit) | | |"]
        if notes:
            report.append("\n**REVIEW (hand pass — never auto-applied):**\n")
            report += [f"- L{i}: `{ln}`\n  - {adv}" for i, ln, adv in notes]
        report.append("")
    open(os.path.join(out_dir, "codemod-report.md"), "w",
         encoding="utf-8").write("\n".join(report) + "\n")
    print(f"codemod: {len(srcs)} file(s) -> {out_dir} (see codemod-report.md)")

if __name__ == "__main__":
    main()
