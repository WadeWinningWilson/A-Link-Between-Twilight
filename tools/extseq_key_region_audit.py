#!/usr/bin/env python3
"""§76 key-region audit: diff engine seq_key_regions_engine.csv vs Bridge ibnk_initvol.csv.

Engine phys_ibnk is the load-slot index (0, 1, …). Bridge uses WW bank ids
(0, 21, …). Slots map in load order: ibnk_0 → 0, ibnk_21 → 21.
"""

from __future__ import annotations

import argparse
import csv
import os
from pathlib import Path

# loadOnePair order in ja1_bank.cpp
ENGINE_SLOT_TO_WW_BANK = {0: 0, 1: 21}


def load(path: Path, remap_engine: bool) -> list:
    rows = []
    with path.open(newline="", encoding="utf-8") as f:
        for r in csv.DictReader(f):
            bi = int(r["phys_ibnk"])
            if remap_engine:
                bi = ENGINE_SLOT_TO_WW_BANK.get(bi, bi)
            rows.append(
                (
                    bi,
                    int(r["prog"]),
                    int(r["key_region"]),
                    int(r["high_key"]),
                    int(r["vel_region"]),
                    int(r["max_vel"]),
                    int(r["wave_id"]),
                    round(float(r["pitch_scale"]), 6),
                    round(float(r["vel_vol_scale"]), 6),
                )
            )
    return rows


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("dir", nargs="?", default="")
    args = ap.parse_args()
    root = (
        Path(args.dir)
        if args.dir
        else Path(os.environ.get("APPDATA", ""))
        / r"TwilitRealm\Dusklight\model_replacements\WW-Crew-Restoration\audio\ww_jaudio1"
    )
    eng = root / "seq_key_regions_engine.csv"
    br = root / "ibnk_initvol.csv"
    if not eng.is_file():
        print(f"missing engine dump: {eng}")
        raise SystemExit(1)
    if not br.is_file():
        print(f"missing Bridge golden: {br}")
        raise SystemExit(1)
    er = load(eng, remap_engine=True)
    br_rows = load(br, remap_engine=False)
    se, sb = set(er), set(br_rows)
    only_e = sorted(se - sb)
    only_b = sorted(sb - se)
    print(f"engine rows={len(er)} bridge rows={len(br_rows)} (slot->WW bank remap applied)")
    print(f"only_engine={len(only_e)} only_bridge={len(only_b)}")
    for k in only_e[:8]:
        print(f"  only_engine {k}")
    for k in only_b[:8]:
        print(f"  only_bridge {k}")
    if not only_e and not only_b:
        print("MATCH - key/velo/wave selection tables identical to Bridge.")
        print("Headline 3 next: runtime path (bank/prog from BMS, setKey/baseKey, pitch).")
        raise SystemExit(0)
    print("DIVERGE - static IBNK parse / resolveInst table mismatch.")
    raise SystemExit(2)


if __name__ == "__main__":
    main()
