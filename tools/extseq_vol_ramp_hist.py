#!/usr/bin/env python3
"""Offline §C.1 volume-ramp histogram from seq_vol_ramps_*.csv (engine dump companion)."""

from __future__ import annotations

import argparse
import csv
from collections import Counter
from pathlib import Path


def hist(path: Path, early_tick: int = 480) -> None:
    rows = []
    with path.open(newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            if row.get("target", "0") != "0":
                continue
            rows.append(row)
    snap = sum(1 for r in rows if int(r["move_time"]) <= 0)
    ramp = len(rows) - snap
    print(f"=== {path.name} ===")
    print(f"volume set_param: {len(rows)}  snap(move<=0)={snap}  ramp={ramp}")
    early = [r for r in rows if int(r["tick"]) < early_tick]
    print(f"early tick<{early_tick}: {len(early)}")
    by_track = Counter(r["track_id"] for r in early)
    print("early by track:", dict(sorted(by_track.items(), key=lambda kv: int(kv[0]))))
    by_mt = Counter((r["track_id"], r["move_time"]) for r in early)
    print("early (track, move_time) top:")
    for (tid, mt), n in by_mt.most_common(16):
        print(f"  track={tid} moveTime={mt} count={n}")
    # Distinct sustained values on track 0 early
    vals = Counter(r["value"] for r in early if r["track_id"] == "0")
    print("early track0 values:", dict(vals.most_common(8)))


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "dir",
        nargs="?",
        default="",
        help="Directory with seq_vol_ramps_*.csv (default: WW-Crew package audio)",
    )
    ap.add_argument("--early", type=int, default=480)
    args = ap.parse_args()
    if args.dir:
        root = Path(args.dir)
    else:
        import os

        root = (
            Path(os.environ.get("APPDATA", ""))
            / r"TwilitRealm\Dusklight\model_replacements\WW-Crew-Restoration\audio\ww_jaudio1"
        )
    files = sorted(root.glob("seq_vol_ramps_*.csv"))
    if not files:
        print(f"no seq_vol_ramps_*.csv under {root}")
        raise SystemExit(1)
    for p in files:
        hist(p, args.early)


if __name__ == "__main__":
    main()
