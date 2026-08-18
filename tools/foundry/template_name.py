"""Foundry §258: template_name — mechanized naming pass for NPC-template drafts.

The WW NPC family shares a code skeleton (proven ba1 §257: eventOrder / setStt /
setAnm_tex / plyTexPttrnAnm / event_action / anmAtr family are structurally
identical across actors). Each of those functions SELF-RECEIPTS one or more
member fields — the same evidence used by hand for ba1's naming-map, expressed
here as rules. Offsets differ per actor; the SHAPES don't.

Every rule below cites its receipt. Names carry the ba1/bm1 sibling convention
= [INFERENCE] tier unless the rule is self-evident from the code ([RECEIPT]).

Usage: template_name.py <draft.cpp> <ActorTag e.g. Aj1> [--out named.cpp]
Writes <draft-dir>/naming-map-auto.md + the named draft.
"""
import os, re, sys

def rules(tag):
    """[(rule-name, tier, member-name, regex-with-one-offset-group), ...]
    Regexes run against the whole draft text; group 1 = the hex offset."""
    T = re.escape(tag)
    # The class-name LENGTH is part of the mangling (11daNpc_Aj1_c) and was
    # hardcoded 11 — every rule silently NO-MATCHed for daNpc_So_c (10 chars).
    # A wrong length is not a loud failure, it is a vacuous pass: the tool
    # reports "shape differs" for a draft whose shape is identical.
    # (DECODER, 2026-08-17, found preparing the So propagation pass.)
    L = len("daNpc_%s_c" % tag)
    return [
        # setStt(s8): stores its arg into the state-number field. [RECEIPT]
        ("setStt arg store", "RECEIPT", "mSttNum",
         rf"setStt__{L}daNpc_{T}_cFSc.*?this->unk(\w+) = \(u8\) arg0;"),
        # eventOrder: selector compared to 1 (speak order). [RECEIPT]
        ("eventOrder selector", "RECEIPT", "mOrderType",
         rf"eventOrder__{L}daNpc_{T}_cFv.*?temp_r0 = this->unk(\w+);"),
        # eventOrder: idx = selector - 3. [RECEIPT]
        ("event index", "RECEIPT", "mEventIdx",
         rf"eventOrder__{L}daNpc_{T}_cFv.*?this->unk(\w+) = \(s8\) temp_r0 - 3;"),
        # eventOrder: table indexed by mEventIdx. [RECEIPT]
        ("event id table", "RECEIPT", "mEventIdTable",
         rf"eventOrder__{L}daNpc_{T}_cFv.*?\* 2\)\)->unk(\w+),"),
        # setAnm_tex(s8): compare-then-store btp number. [RECEIPT]
        ("setAnm_tex store", "RECEIPT", "mBtpNum",
         rf"setAnm_tex__{L}daNpc_{T}_cFSc.*?this->unk(\w+) = \(u8\) arg0;"),
        # plyTexPttrnAnm: cLib_calcTimer target = blink timer. [RECEIPT]
        ("blink timer", "RECEIPT", "mBlinkTimer",
         rf"plyTexPttrnAnm__{L}daNpc_{T}_cFv.*?cLib_calcTimer<s>__FPs\(&this->unk(\w+)\)"),
        # plyTexPttrnAnm: frame counter incremented vs frame max. [RECEIPT]
        ("btp frame", "RECEIPT", "mBtpFrame",
         rf"plyTexPttrnAnm__{L}daNpc_{T}_cFv.*?this->unk(\w+) \+= 1"),
        # plyTexPttrnAnm: ->unk6 == J3DAnmBase::mFrameMax holder (§251). [RECEIPT]
        ("btp resource", "RECEIPT", "mpBtpRes",
         rf"plyTexPttrnAnm__{L}daNpc_{T}_cFv.*?this->unk(\w+)->unk6"),
        # event_action: jumptable/switch selector. [RECEIPT]
        ("event action no", "RECEIPT", "mEvtActionNo",
         rf"event_action__{L}daNpc_{T}_cFv.*?temp_r0 = this->unk(\w+);"),
        # setAnm_ATR family: 0xFF = none sentinel. [INFERENCE]
        ("anm attribute", "INFERENCE", "mAnmAtr",
         rf"setStt__{L}daNpc_{T}_cFSc.*?this->unk(\w+) = 0xFF;"),
        # setStt: previous-state save at tail. [RECEIPT]
        ("stt old", "RECEIPT", "mSttNumOld",
         rf"setStt__{L}daNpc_{T}_cFSc.*?this->unk(\w+) = temp_r5;"),
        # setStt: timer zeroed on entry. [INFERENCE]
        ("stt timer", "INFERENCE", "mSttTimer",
         rf"setStt__{L}daNpc_{T}_cFSc.*?this->unk(\w+) = 0;"),
    ]

def main():
    draft_path, tag = sys.argv[1], sys.argv[2]
    out_path = (sys.argv[sys.argv.index("--out") + 1] if "--out" in sys.argv
                else draft_path.replace("_full_draft", "_named_draft"))
    text = open(draft_path, encoding="utf-8", errors="replace").read()
    found, rows = {}, []
    for rule, tier, name, rx in rules(tag):
        m = re.search(rx, text, re.S)
        if not m:
            rows.append(f"| — | `{name}` | [{tier}] | {rule}: NO MATCH (shape "
                        "differs — name by hand, ba1 §257 method) |")
            continue
        off = m.group(1).upper()
        if off in found and found[off] != name:
            rows.append(f"| 0x{off} | `{name}` | [{tier}] | {rule}: COLLIDES "
                        f"with `{found[off]}` — resolve by hand |")
            continue
        found[off] = name
        rows.append(f"| 0x{off} | `{name}` | [{tier}] | {rule} |")
    n = 0
    def sub(m):
        nonlocal n
        key = m.group(1).upper()
        if key in found:
            n += 1
            return found[key]
        return m.group(0)
    named = re.sub(r"\bunk([0-9A-Fa-f]{2,4})\b", sub, text)
    hdr = (f"// NAMED DRAFT (auto, template_name.py §258) — rules + tiers in "
           f"naming-map-auto.md;\n// remaining unkNNN = [INFERENCE-NEEDED]. "
           f"Behavior identical to the full draft.\n")
    open(out_path, "w", encoding="utf-8").write(hdr + named)
    map_path = os.path.join(os.path.dirname(draft_path), "naming-map-auto.md")
    open(map_path, "w", encoding="utf-8").write(
        f"# {tag} auto naming map (template_name.py §258 — the ba1 §257 "
        "rules, mechanized)\n\n"
        "| offset | name | tier | receipt (rule) |\n|---|---|---|---|\n"
        + "\n".join(rows) + f"\n\n{n} renames applied → `{os.path.basename(out_path)}`."
        " Rules that NO-MATCHed or COLLIDED need the manual ba1-method pass.\n")
    print(f"{tag}: {len(found)} fields named, {n} renames -> {out_path}")

if __name__ == "__main__":
    main()
