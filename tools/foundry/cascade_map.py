"""Foundry §262 (kit v4, §248 pitfalls 1+2 — ask #5): CASCADE GRAPH + DORMANCY MAP.

Pitfall 1 (cascade blindness): surface.md treats each missing symbol as a flat
leaf. Aryll's "telescope demo" was a CASCADE: demo -> scope-message subsystem ->
daPyStts0_TELESCOPE_LOOK_e -> the player telescope ITEM (a whole absent player
feature). This tool resolves, for every missing symbol, what its own DONOR
DEFINITION needs, transitively (depth-capped, memoized) — and names the
TERMINAL SUBSYSTEM (the donor file the chain bottoms out in).

Pitfall 2 (live vs dormant): the per-symbol question that decides stub strategy.
Mechanized as REACHABILITY over the TU's local call graph from lifecycle roots:
  CORE        reachable from _create/_execute/_draw/CreateHeap -> the actor
              runs it at spawn: shim MUST BE REAL.
  DEMO-ONLY   reachable only via demo()/cut_*/eMove_*/event paths -> dormant
              until a cutscene fires; LOAD-BEARING iff the actor is a
              cutscene actor (§251 tier decides).
  TALK-ONLY   reachable only via talk/getMsg/msg paths -> dormant until talked
              to; for a placed, talkable NPC treat as live.
  UNREACHED   no local caller path from any root -> dead/cut path: inert stub
              is faithful-in-effect.
Verdicts are REACHABILITY FACTS + the §251 cutscene bit — the porter applies
judgment on top; the tool never guesses beyond what the call graph shows.

Usage: cascade_map.py <donor.cpp> [donor.h ...] [--cutscene] [--out cascade.md]
"""
import os, re, sys
from collections import deque

TOOLS = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, TOOLS)
from api_surface import build_index, SKIP, CALL  # noqa: E402

DONOR_SRC = r"D:\XXXXXXX\WW DP\src"
RECEIVER = r"C:\Users\xxxxx\Documents\dusklight"

CORE_ROOT = re.compile(r"(_create|_execute|_draw|_delete|CreateHeap|"
                       r"createInit|Draw$|Execute$|Create$)", re.I)
DEMO_ROOT = re.compile(r"(^demo|^cut_|^eInit_|^eMove_|event|Event|_action)", )
TALK_ROOT = re.compile(r"(talk|Talk|getMsg|next_msg|Msg)")

DEF_RE = re.compile(
    r"^[A-Za-z_][\w\s:*&<>,~]*?\b(?:\w+::)?(\w+)\s*\([^;{()]*\)\s*"
    r"(?:const\s*)?\{", re.M)

def local_functions(text):
    """name -> body (brace-matched) for every function defined in the TU."""
    out = {}
    for m in DEF_RE.finditer(text):
        name, start = m.group(1), m.end() - 1
        depth, i = 0, start
        while i < len(text):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        out.setdefault(name, "")
        out[name] += text[start:i + 1]
    return out

def reachability(funcs):
    """fn -> {CORE, DEMO, TALK} labels via BFS from each root class."""
    calls = {f: set(re.findall(r"\b(\w+)\s*\(", body)) & set(funcs)
             for f, body in funcs.items()}
    labels = {f: set() for f in funcs}
    for cls, root_rx in (("CORE", CORE_ROOT), ("DEMO", DEMO_ROOT),
                         ("TALK", TALK_ROOT)):
        seeds = [f for f in funcs if root_rx.search(f)]
        seen = set(seeds)
        dq = deque(seeds)
        while dq:
            f = dq.popleft()
            labels[f].add(cls)
            for g in calls.get(f, ()):
                if g not in seen:
                    seen.add(g)
                    dq.append(g)
    return labels

def donor_def_index():
    """symbol -> (path, def-start-offset) across donor src (first hit)."""
    idx = {}
    for dp, _d, fs in os.walk(DONOR_SRC):
        for fn in fs:
            if not fn.endswith((".cpp", ".c", ".inc")):
                continue
            p = os.path.join(dp, fn)
            try:
                t = open(p, encoding="utf-8", errors="replace").read()
            except OSError:
                continue
            for m in DEF_RE.finditer(t):
                idx.setdefault(m.group(1), (p, m.end() - 1))
    return idx

def body_at(path, start):
    t = open(path, encoding="utf-8", errors="replace").read()
    depth, i = 0, start
    while i < len(t):
        if t[i] == "{":
            depth += 1
        elif t[i] == "}":
            depth -= 1
            if depth == 0:
                break
        i += 1
    return t[start:i + 1]

def cascade(sym, recv, ddefs, memo, depth=0, path=None):
    """-> (max_depth, transitive_missing_set, terminal_files)"""
    if sym in memo:
        return memo[sym]
    if depth >= 3:
        return (depth, set(), set())
    hit = ddefs.get(sym)
    if not hit:
        memo[sym] = (depth, set(), {"(no donor definition found — constant/"
                                    "inline/data: leaf)"})
        return memo[sym]
    body = body_at(*hit)
    rel = os.path.relpath(hit[0], DONOR_SRC)
    kids = {n for n in re.findall(r"\b([A-Za-z_]\w{3,})\s*\(", body)
            if n not in SKIP and n not in recv and n != sym}
    if not kids:
        memo[sym] = (depth, set(), {rel})
        return memo[sym]
    md, miss, terms = depth, set(kids), set()
    for k in sorted(kids)[:12]:
        d, m2, t2 = cascade(k, recv, ddefs, memo, depth + 1)
        md, miss, terms = max(md, d), miss | m2, terms | t2
    if not terms:
        terms = {rel}
    memo[sym] = (md, miss, terms)
    return memo[sym]

def main():
    argv = sys.argv[1:]
    out = None
    if "--out" in argv:
        out = argv[argv.index("--out") + 1]
        argv.remove("--out"); argv.remove(out)
    is_cut = "--cutscene" in argv
    if is_cut:
        argv.remove("--cutscene")
    text = "\n".join(open(s, encoding="utf-8", errors="replace").read()
                     for s in argv)
    text = re.sub(r"/\*.*?\*/", " ", text, flags=re.S)
    text = re.sub(r"//[^\n]*", " ", text)
    funcs = local_functions(text)
    labels = reachability(funcs)
    recv = build_index([p for p in (os.path.join(RECEIVER, "src"),
                                    os.path.join(RECEIVER, "include"),
                                    os.path.join(RECEIVER, "libs"))
                        if os.path.isdir(p)])
    # missing external identifiers per local function
    missing_by_fn = {}
    for f, body in funcs.items():
        ext = {n for n in re.findall(r"\b([A-Za-z_]\w{3,})\s*\(", body)
               if n not in SKIP and n not in recv and n not in funcs}
        if ext:
            missing_by_fn[f] = ext
    all_missing = sorted(set().union(*missing_by_fn.values())
                         if missing_by_fn else set())
    # dormancy verdicts
    rows_d = []
    for sym in all_missing:
        users = [f for f, ms in missing_by_fn.items() if sym in ms]
        cls = set().union(*(labels[f] for f in users))
        if "CORE" in cls:
            reach, verdict = "CORE", "**MUST BE REAL** (runs at spawn)"
        elif "DEMO" in cls:
            reach = "DEMO-ONLY"
            verdict = ("**LOAD-BEARING (cutscene actor, §251)**" if is_cut
                       else "stub-safe UNTIL the actor joins a cutscene")
        elif "TALK" in cls:
            reach, verdict = "TALK-ONLY", "live for any talkable placement"
        else:
            reach, verdict = "UNREACHED", "inert stub is faithful-in-effect"
        rows_d.append((sym, ", ".join(sorted(users)[:4]), reach, verdict))
    # cascades
    ddefs = donor_def_index()
    memo = {}
    rows_c = []
    for sym in all_missing:
        d, miss, terms = cascade(sym, recv, ddefs, memo)
        if d > 0 or miss:
            rows_c.append((sym, d + 1, len(miss),
                           "; ".join(sorted(terms)[:3])))
    rows_c.sort(key=lambda r: (-r[1], -r[2]))
    lines = ["# Cascade graph + dormancy map (§248 pitfalls 1+2 / §262)", "",
             "## Dormancy — per missing symbol: LIVE or gated? "
             "(decides the shim strategy UP FRONT)", "",
             "| missing symbol | used by | reachability | verdict |",
             "|---|---|---|---|"]
    lines += [f"| `{s}` | {u} | {r} | {v} |" for s, u, r, v in rows_d]
    lines += ["", "## Cascades — what each missing symbol ITSELF needs "
              "(depth-capped 3; deepest first)", "",
              "> depth 1 = a flat leaf (shim and done). depth ≥2 = porting it "
              "for real drags a chain; the TERMINAL column names the donor "
              "subsystem the chain bottoms out in — the §248 'this path "
              "bottoms out at player-feature X' line, mechanized.", "",
              "| missing symbol | cascade depth | transitive missing | "
              "terminal subsystem(s) |", "|---|---|---|---|"]
    lines += [f"| `{s}` | {d} | {n} | `{t}` |" for s, d, n, t in rows_c]
    if not rows_c:
        lines.append("| (all missing symbols are flat leaves) | | | |")
    body = "\n".join(lines) + "\n"
    if out:
        os.makedirs(os.path.dirname(out) or ".", exist_ok=True)
        open(out, "w", encoding="utf-8").write(body)
        print(f"wrote {out}: {len(rows_d)} symbols, "
              f"{sum(1 for r in rows_c if r[1] >= 2)} true cascades")
    else:
        print(body)

if __name__ == "__main__":
    main()
