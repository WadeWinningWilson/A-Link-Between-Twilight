"""Foundry P2 verdict tool: compare two probe-event JSONL streams.

Modes:
  census  A.jsonl B.jsonl [--site S] [--a-window MM:SS-MM:SS] [--b-window ...]
      Per (site,key) counts + rates for both streams; verdict per row and overall.
      Empty input => UNKNOWN (never MATCH) — negative-control rule.
  seq     A.jsonl B.jsonl --site S [windows]
      difflib alignment on the key-tuple stream: similarity %, first divergence.
  profile A.jsonl --site S [--key K] [--bucket SECS]
      Single-stream rate profile (events/bucket over time) — e.g. windline 0x31.

Verdicts: MATCH (ratio within --tol, default 0.25) | DRIFT | A-ONLY | B-ONLY | UNKNOWN.
"""
import sys, json, argparse, difflib
from collections import Counter, defaultdict

def load(path, site=None, window=None):
    evs = []
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            ev = json.loads(line)
            if site and ev["site"] != site:
                continue
            if window and not (window[0] <= ev["t_s"] <= window[1]):
                continue
            evs.append(ev)
    return evs

def parse_window(s):
    def t(x):
        parts = x.split(":")
        parts = [float(p) for p in parts]
        while len(parts) < 2:
            parts.insert(0, 0.0)
        return parts[-2] * 60 + parts[-1] + (parts[-3] * 3600 if len(parts) > 2 else 0)
    a, b = s.split("-")
    return (t(a), t(b))

def keystr(ev):
    return "/".join(str(x) for x in ev["key"]) if ev["key"] else "-"

def span_s(evs):
    return max(1e-9, evs[-1]["t_s"] - evs[0]["t_s"]) if len(evs) > 1 else 1e-9

def census(a, b, tol):
    if not a or not b:
        print("VERDICT: UNKNOWN — one or both streams are EMPTY "
              f"(A={len(a)} B={len(b)}). Empty is not a match.")
        return 3
    ca = Counter((ev["site"], keystr(ev)) for ev in a)
    cb = Counter((ev["site"], keystr(ev)) for ev in b)
    sa, sb = span_s(a), span_s(b)
    rows, drift = [], 0
    for k in sorted(set(ca) | set(cb)):
        na, nb = ca.get(k, 0), cb.get(k, 0)
        ra, rb = na / sa, nb / sb
        if na == 0:
            v = "B-ONLY"
        elif nb == 0:
            v = "A-ONLY"
        else:
            rel = abs(ra - rb) / max(ra, rb)
            v = "MATCH" if rel <= tol else f"DRIFT({rel:.0%})"
        if not v.startswith("MATCH") and max(na, nb) >= 5:
            drift += 1
        rows.append((k, na, nb, ra, rb, v))
    print(f"| site | key | A n | B n | A/s | B/s | verdict |")
    print(f"|---|---|---|---|---|---|---|")
    for (site, key), na, nb, ra, rb, v in rows:
        print(f"| {site} | {key} | {na} | {nb} | {ra:.2f} | {rb:.2f} | {v} |")
    ok = drift == 0
    print(f"\nVERDICT: {'MATCH' if ok else 'DRIFT'} — {drift} significant "
          f"divergent rows of {len(rows)} (tol {tol:.0%}, spans {sa:.0f}s/{sb:.0f}s)")
    return 0 if ok else 1

def seq(a, b):
    if not a or not b:
        print(f"VERDICT: UNKNOWN — empty stream (A={len(a)} B={len(b)}).")
        return 3
    ka = [keystr(ev) for ev in a]
    kb = [keystr(ev) for ev in b]
    sm = difflib.SequenceMatcher(None, ka, kb, autojunk=False)
    sim = sm.ratio()
    # first divergence
    div = None
    for op, i1, i2, j1, j2 in sm.get_opcodes():
        if op != "equal":
            div = (i1, a[i1]["t"] if i1 < len(a) else "-",
                   j1, b[j1]["t"] if j1 < len(b) else "-", op)
            break
    print(f"similarity {sim:.1%} over A={len(ka)} B={len(kb)} events")
    if div:
        print(f"first divergence: A[{div[0]}]@{div[1]} vs B[{div[2]}]@{div[3]} ({div[4]})")
    else:
        print("streams identical")
    print(f"VERDICT: {'MATCH' if sim >= 0.98 else 'DRIFT'}")
    return 0 if sim >= 0.98 else 1

def profile(a, key, bucket):
    if not a:
        print("VERDICT: UNKNOWN — empty stream.")
        return 3
    if key:
        a = [ev for ev in a if keystr(ev) == key]
        if not a:
            print(f"VERDICT: UNKNOWN — no events with key {key}.")
            return 3
    t0 = a[0]["t_s"]
    buckets = defaultdict(int)
    for ev in a:
        buckets[int((ev["t_s"] - t0) // bucket)] = buckets[int((ev["t_s"] - t0) // bucket)] + 1
    print(f"{len(a)} events over {span_s(a):.0f}s; bucket={bucket}s; t0={a[0]['t']}")
    peak = max(buckets.values())
    for i in sorted(buckets):
        bar = "#" * int(40 * buckets[i] / peak)
        print(f"{i*bucket:6.0f}s  {buckets[i]:5d}  {bar}")
    print(f"mean {len(a)/span_s(a):.2f}/s  peak {peak/bucket:.2f}/s")
    return 0

def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["census", "seq", "profile"])
    ap.add_argument("a")
    ap.add_argument("b", nargs="?")
    ap.add_argument("--site")
    ap.add_argument("--key")
    ap.add_argument("--tol", type=float, default=0.25)
    ap.add_argument("--bucket", type=float, default=10.0)
    ap.add_argument("--a-window")
    ap.add_argument("--b-window")
    args = ap.parse_args()
    aw = parse_window(args.a_window) if args.a_window else None
    bw = parse_window(args.b_window) if args.b_window else None
    a = load(args.a, args.site, aw)
    if args.mode == "profile":
        sys.exit(profile(a, args.key, args.bucket))
    b = load(args.b, args.site, bw)
    sys.exit(census(a, b, args.tol) if args.mode == "census" else seq(a, b))

if __name__ == "__main__":
    main()
