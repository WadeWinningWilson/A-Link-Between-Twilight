#!/usr/bin/env python3
"""§C.1 absolute BMS volume levels from seq_vol_ramps_*.csv (confirmed i_link/house).

Reports per-track sustained raw values and a volumeMode-0 composed estimate
(raw² × parent chain). Dump track_id is the open_track walker id (0=root);
children are assumed parent=0 unless nested ids appear in the same early window
— for field music open_track is flat under root, so parent=root for tid>0.
"""

from __future__ import annotations

import argparse
import csv
import os
from collections import defaultdict
from pathlib import Path


def load_vol(path: Path):
    rows = []
    with path.open(newline="", encoding="utf-8") as f:
        for row in csv.DictReader(f):
            if row.get("target", "0") != "0":
                continue
            rows.append(
                {
                    "tick": int(row["tick"]),
                    "track": int(row["track_id"]),
                    "value": float(row["value"]) if row["value"] else None,
                    "move": int(row["move_time"]),
                }
            )
    return rows


def analyze(path: Path, early: int = 480) -> None:
    rows = [r for r in load_vol(path) if r["value"] is not None]
    print(f"=== {path.name} absolute levels ===")
    by_track: dict[int, list] = defaultdict(list)
    for r in rows:
        by_track[r["track"]].append(r)

    # Last value per track in early window (sustained after opening snaps).
    early_last = {}
    for tid, rs in sorted(by_track.items()):
        early_rs = [r for r in rs if r["tick"] < early]
        if early_rs:
            early_last[tid] = early_rs[-1]["value"]
        uniq = sorted({round(r["value"], 6) for r in rs})
        mx = max(r["value"] for r in rs)
        print(
            f"  track {tid:2d}: n={len(rs):4d} max={mx:.6f} "
            f"unique={uniq[:6]}{'...' if len(uniq) > 6 else ''}"
        )

    root = early_last.get(0, 1.0)
    root_c = root * root  # volumeMode 0
    print(f"\n  early<{early} last raw -> composed (mode0, parent=root for children):")
    print(f"    track 0: raw={root:.6f} composed={root_c:.6f}")
    louder = []
    for tid, raw in sorted(early_last.items()):
        if tid == 0:
            continue
        child_c = (raw * raw) * root_c
        print(f"    track {tid}: raw={raw:.6f} composed={child_c:.6f}")
        louder.append((child_c, tid, raw))
    louder.sort(reverse=True)
    if louder:
        print("  loudest early children by composed:")
        for c, tid, raw in louder[:8]:
            print(f"    composed={c:.6f} track={tid} raw={raw:.6f}")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("dir", nargs="?", default="")
    ap.add_argument("--early", type=int, default=480)
    args = ap.parse_args()
    root = (
        Path(args.dir)
        if args.dir
        else Path(os.environ.get("APPDATA", ""))
        / r"TwilitRealm\Dusklight\model_replacements\WW-Crew-Restoration\audio\ww_jaudio1"
    )
    files = sorted(root.glob("seq_vol_ramps_*.csv"))
    if not files:
        print(f"no seq_vol_ramps_*.csv under {root}")
        raise SystemExit(1)
    for p in files:
        analyze(p, args.early)
        print()


if __name__ == "__main__":
    main()
