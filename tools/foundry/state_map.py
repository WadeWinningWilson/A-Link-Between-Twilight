"""Foundry P13: donor action-state-machine extractor.

Mechanically extracts a donor actor's mode/action structure from its decomp
source — QUOTES the code, never interprets it (covenant donor-verbatim law):
  - enum blocks that look like action/mode enums
  - switch statements over mode-like fields, with their case labels
  - every assignment to mode-like fields (the transition writes), line-cited

Semantic meaning of states/transitions is deliberately left [INFERENCE-NEEDED];
this artifact is the raw state map a restoration implements from and the P2 seq
differ verifies against.

Usage: state_map.py <decomp_actor.cpp> [more .cpp/.h/.inc ...] <out.md>
(§221: pass the actor's header too — thin-shell NPCs keep set_action calls and
state rosters in header inlines.)
"""
import sys, re

MODE_FIELD = re.compile(r"\b(m(?:Action|Mode|ActionMode|Mold)?[A-Za-z_]*(?:mode|Mode|action|Action)[A-Za-z_0-9]*)\b")
ENUM_RE = re.compile(r"enum\s+([A-Za-z_0-9]*)\s*\{([^}]*)\}", re.S)
SWITCH_RE = re.compile(r"switch\s*\(([^)]*)\)")
CASE_RE = re.compile(r"^\s*case\s+([^:]+):")
ASSIGN_RE = re.compile(r"^\s*([A-Za-z_][A-Za-z_0-9>.\-]*(?:mode|Mode|action|Action)[A-Za-z_0-9]*)\s*=\s*([^;]+);")
SETFUNC_RE = re.compile(r"^\s*(?:[A-Za-z_][A-Za-z_0-9:]*\s+)?([a-zA-Z_][A-Za-z_0-9]*::)?(set(?:Action|Mode|Mold)[A-Za-z_0-9]*)\s*\(")

def main():
    *srcs, out_md = sys.argv[1:]
    text = ""
    lines = []
    for src_path in srcs:
        t = open(src_path, encoding="utf-8", errors="replace").read()
        text += t + "\n"
        lines += [f"{ln}\x01{src_path}" for ln in t.splitlines()]
    # strip the source tag for matching; keep for citation
    tagged = [(ln.split("\x01")[0], ln.split("\x01")[1]) for ln in lines]
    lines = [t[0] for t in tagged]
    src_of = [t[1] for t in tagged]

    out = [f"# Donor state map — {', '.join('`'+s+'`' for s in srcs)}",
           "",
           "> Mechanical extraction (Foundry P13). Every row is a verbatim quote with",
           "> a line number. State SEMANTICS are [INFERENCE-NEEDED] by design — name",
           "> them only from decomp evidence, never invention.", ""]

    out.append("## Enums (candidate state vocabularies)")
    for m in ENUM_RE.finditer(text):
        name = m.group(1) or "(anonymous)"
        body = " ".join(m.group(2).split())
        if len(body) > 700:
            body = body[:700] + " …[truncated]"
        line = text[:m.start()].count("\n") + 1
        out.append(f"- L{line} `enum {name}` {{ {body} }}")

    # §224 third idiom (kb-style): switches over ANONYMOUS fields (partial-decomp
    # names like m420). Pass 1: any field switched on repeatedly IS a state field;
    # pass 2 below captures assignments to those fields as transitions.
    anon_switch = re.compile(r"switch\s*\(\s*\w+(?:->|\.)((?:m|field_0x)[0-9A-Fa-f_]+)\s*\)")
    anon_fields = {}
    for ln in lines:
        m = anon_switch.search(ln)
        if m:
            anon_fields[m.group(1)] = anon_fields.get(m.group(1), 0) + 1
    anon_fields = {f for f, c in anon_fields.items() if c >= 2}

    out.append("\n## switch() dispatchers over mode-like expressions")
    for i, ln in enumerate(lines, 1):
        m = SWITCH_RE.search(ln)
        if m and (re.search(r"mode|Mode|action|Action|proc|Proc", m.group(1)) or
                  any(f in m.group(1) for f in anon_fields)):
            cases = []
            depth = 0
            for j in range(i, min(i + 400, len(lines))):
                depth += lines[j - 1].count("{") - lines[j - 1].count("}")
                cm = CASE_RE.match(lines[j - 1])
                if cm:
                    cases.append(cm.group(1).strip())
                if depth <= 0 and j > i:
                    break
            if cases:
                out.append(f"- L{i} `switch ({m.group(1).strip()})` → cases: "
                           + ", ".join(f"`{c}`" for c in cases[:40]))

    out.append("\n## Transition writes (mode-like assignments, line-cited)")
    n = 0
    for i, ln in enumerate(lines, 1):
        m = ASSIGN_RE.match(ln)
        if m:
            out.append(f"- L{i} `{ln.strip()}`")
            n += 1
    out.append(f"\n({n} transition writes)")

    # §224: helper-call transition idiom (kb-style `xx_set_mode(i_this, N)` /
    # `setMode(N)`) — capture the numeric target so the state gate's legal-value
    # set includes helper-routed transitions.
    helper_re = re.compile(
        r"\b\w*[sS]et\w*(?:[Mm]ode|[Aa]ction)\w*\s*\(([^;]*?)(-?\d+)\s*\)\s*;")
    out.append("\n## Helper-call transition writes (numeric targets, line-cited)")
    hn = 0
    for i, ln in enumerate(lines, 1):
        m = helper_re.search(ln)
        if m:
            out.append(f"- L{i} `{ln.strip()}`  → target `mMode = {m.group(2)}`")
            hn += 1
    out.append(f"\n({hn} helper-call writes)")

    if anon_fields:
        out.append("\n## Anonymous state-field transitions "
                   f"(fields {', '.join(sorted(anon_fields))} — switched on ≥2×; "
                   "[INFERENCE-NEEDED: donor field names pending decomp naming])")
        an = 0
        anon_assign = re.compile(
            r"(?:->|\.)(" + "|".join(re.escape(f) for f in sorted(anon_fields)) +
            r")\s*=\s*(-?(?:0x[0-9A-Fa-f]+|\d+))\s*;")
        for i, ln in enumerate(lines, 1):
            m = anon_assign.search(ln)
            if m:
                out.append(f"- L{i} `{ln.strip()}`")
                an += 1
        out.append(f"\n({an} anonymous-field transitions)")
    out.append("\n## setAction/setMode-style calls (line-cited, first 60)")
    k = 0
    for i, ln in enumerate(lines, 1):
        if SETFUNC_RE.match(ln) and k < 60:
            out.append(f"- L{i} `{ln.strip()}`")
            k += 1

    # WW NPC idiom (§219): states are MEMBER FUNCTION POINTERS installed via
    # set_action(&Class::method, arg) — the transition graph is the set of
    # set_action call sites; the state roster is the *_action method set.
    npc_set = re.compile(r"set_action\s*\(")
    npc_state = re.compile(r"::(\w*(?:action|Action)\w*)\s*\(")
    out.append("\n## NPC idiom — set_action transitions (function-pointer states)")
    m_count = 0
    for i, ln in enumerate(lines, 1):
        if npc_set.search(ln):
            out.append(f"- L{i} `{ln.strip()}`")
            m_count += 1
    out.append(f"\n({m_count} set_action transitions)")
    roster = sorted({m.group(1) for ln in lines for m in [npc_state.search(ln)] if m})
    if roster:
        out.append("\n## NPC idiom — action-method state roster")
        out.append(", ".join(f"`{r}`" for r in roster))

    open(out_md, "w", encoding="utf-8").write("\n".join(out) + "\n")
    print(f"wrote {out_md} ({n} transitions)")

if __name__ == "__main__":
    main()
