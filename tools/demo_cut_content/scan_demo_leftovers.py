#!/usr/bin/env python3
"""Scan all Demo*.arc files for leftover / beta / orphan cut-content.

Reproduces the archaeology path that found beta Link in Demo01_00 / Demo04_01
and the adapted gameplay pack at D:\\Decomps\\beta link\\Kmdl.arc.

Usage:
  python tools/demo_cut_content/scan_demo_leftovers.py
  python tools/demo_cut_content/scan_demo_leftovers.py --object-dir "D:/XXXXXXX/Ex TP/files/res/Object"
  python tools/demo_cut_content/scan_demo_leftovers.py --extract
"""
from __future__ import annotations

import argparse
import csv
import hashlib
import json
import re
import struct
import sys
from collections import defaultdict
from pathlib import Path

# Reuse companion_mod RARC helpers when available.
_TOOLS = Path(__file__).resolve().parents[1] / "companion_mod"
if str(_TOOLS) not in sys.path:
    sys.path.insert(0, str(_TOOLS))

from extract_rarc import parse_rarc  # noqa: E402


def yaz0_decompress(data: bytes) -> bytes:
    """Correct Yaz0 (handles long-copy third-byte length)."""
    if data[:4] != b"Yaz0":
        raise ValueError("not Yaz0")
    dec_size = struct.unpack(">I", data[4:8])[0]
    out = bytearray()
    src = 16
    while len(out) < dec_size and src < len(data):
        code = data[src]
        src += 1
        for bit in range(8):
            if len(out) >= dec_size:
                break
            if code & (0x80 >> bit):
                if src >= len(data):
                    raise ValueError("Yaz0 truncated (literal)")
                out.append(data[src])
                src += 1
            else:
                if src + 1 >= len(data):
                    raise ValueError("Yaz0 truncated (copy)")
                b1, b2 = data[src], data[src + 1]
                src += 2
                dist = ((b1 & 0x0F) << 8) | b2
                n = b1 >> 4
                if n == 0:
                    if src >= len(data):
                        raise ValueError("Yaz0 truncated (long copy)")
                    n = data[src] + 0x12
                    src += 1
                else:
                    n += 2
                copy_src = len(out) - (dist + 1)
                for _ in range(n):
                    if len(out) >= dec_size:
                        break
                    out.append(out[copy_src])
                    copy_src += 1
    return bytes(out)


# Name tokens that historically mark leftover / alternate / ancestry assets.
LEFTOVER_PATTERNS: list[tuple[str, re.Pattern[str]]] = [
    ("original", re.compile(r"original", re.I)),
    ("high", re.compile(r"(?:^|_|/)high(?:_|$|\.)", re.I)),
    ("tmp", re.compile(r"(?:^|_|/)tmp(?:_|$|\.)|_tmp\.|demo00_", re.I)),
    ("henkei", re.compile(r"henkei", re.I)),  # transform / morph face lineage
    ("hiface", re.compile(r"hiface", re.I)),
    ("beta_cast", re.compile(r"(?:ctz|knight|wb_|_wb_|boar|pig)", re.I)),
    ("lantern", re.compile(r"kantera|lantern|torch", re.I)),
    ("link_body", re.compile(r"link.*(?:bd|hl|bl)|(?:^|/)al(?:_|\.)", re.I)),
]

MODEL_EXTS = {".bmd", ".bdl"}
ANIM_EXTS = {".bck", ".bck", ".btk", ".brk", ".btp", ".bpk", ".blk", ".bas"}


def ensure_rarc(raw: bytes) -> bytes:
    if raw[:4] == b"Yaz0":
        return yaz0_decompress(raw)
    if raw[:4] == b"RARC":
        return raw
    raise ValueError(f"unknown magic {raw[:4]!r}")


def sha16(blob: bytes) -> str:
    return hashlib.sha256(blob).hexdigest()[:16]


def classify(name: str) -> list[str]:
    tags = [tag for tag, pat in LEFTOVER_PATTERNS if pat.search(name)]
    low = name.lower()
    if any(low.endswith(ext) for ext in MODEL_EXTS):
        tags.append("model")
    if any(low.endswith(ext) for ext in ANIM_EXTS):
        tags.append("anim")
    return tags


def list_demo_arcs(object_dir: Path) -> list[Path]:
    return sorted(object_dir.glob("Demo*.arc"))


def scan_arc(path: Path) -> list[dict]:
    raw = path.read_bytes()
    try:
        rarc = ensure_rarc(raw)
    except Exception as exc:  # noqa: BLE001 — keep scanning other arcs
        print(f"  SKIP {path.name}: {exc}", file=sys.stderr)
        return []
    rows: list[dict] = []
    for file_id, name, _off, size, payload in parse_rarc(rarc):
        tags = classify(name)
        leftover_tags = [t for t in tags if t not in ("model", "anim")]
        if not leftover_tags:
            continue
        rows.append(
            {
                "arc": path.name,
                "id": f"0x{file_id:04x}",
                "name": name,
                "size": size,
                "sha16": sha16(payload),
                "tags": ",".join(
                    sorted(
                        set(
                            leftover_tags
                            + [t for t in tags if t in ("model", "anim")]
                        )
                    )
                ),
                "j3d": payload.find(b"J3D2") if payload else -1,
            }
        )
    return rows


def index_arc_files(path: Path) -> dict[str, tuple[int, str]]:
    """name -> (size, sha16) for file entries."""
    raw = path.read_bytes()
    rarc = ensure_rarc(raw)
    out: dict[str, tuple[int, str]] = {}
    for _fid, name, _off, size, payload in parse_rarc(rarc):
        out[name] = (size, sha16(payload))
    return out


def extract_named(arc_path: Path, names: set[str], out_dir: Path) -> list[Path]:
    raw = arc_path.read_bytes()
    rarc = ensure_rarc(raw)
    written: list[Path] = []
    out_dir.mkdir(parents=True, exist_ok=True)
    for file_id, name, _off, size, payload in parse_rarc(rarc):
        if name not in names:
            continue
        dest = out_dir / f"{arc_path.stem}__{file_id:04x}_{name}"
        dest.write_bytes(payload)
        written.append(dest)
    return written


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--object-dir",
        type=Path,
        default=Path(r"D:/XXXXXXX/Ex TP/files/res/Object"),
        help="Disc Object directory containing Demo*.arc",
    )
    ap.add_argument(
        "--beta-kmdl",
        type=Path,
        default=Path(r"D:/XXXXXXX/beta link/Kmdl.arc"),
        help="Adapted beta Link Kmdl pack (proof of prior restore)",
    )
    ap.add_argument(
        "--out-dir",
        type=Path,
        default=Path(__file__).resolve().parent / "out",
        help="Catalog + extract output directory",
    )
    ap.add_argument(
        "--extract",
        action="store_true",
        help="Extract high-priority leftover models into out/extract/",
    )
    args = ap.parse_args()

    object_dir: Path = args.object_dir
    if not object_dir.is_dir():
        print(f"ERROR: object dir missing: {object_dir}", file=sys.stderr)
        return 1

    demos = list_demo_arcs(object_dir)
    print(f"Scanning {len(demos)} Demo*.arc under {object_dir}")

    all_rows: list[dict] = []
    by_arc: dict[str, int] = {}
    for arc in demos:
        rows = scan_arc(arc)
        by_arc[arc.name] = len(rows)
        all_rows.extend(rows)
        if rows:
            print(f"  {arc.name}: {len(rows)} tagged leftovers")

    args.out_dir.mkdir(parents=True, exist_ok=True)
    csv_path = args.out_dir / "demo_leftovers.csv"
    with csv_path.open("w", newline="", encoding="utf-8") as f:
        w = csv.DictWriter(
            f, fieldnames=["arc", "id", "name", "size", "sha16", "tags", "j3d"]
        )
        w.writeheader()
        w.writerows(all_rows)

    # Priority summary: arcs with original/high/tmp/henkei/link
    priority_tags = {"original", "high", "tmp", "henkei", "hiface", "link_body", "beta_cast", "lantern"}
    priority = [
        r
        for r in all_rows
        if any(t in r["tags"].split(",") for t in priority_tags)
        and ("model" in r["tags"] or r["name"].lower().endswith((".bmd", ".bdl")))
    ]

    # Compare beta Kmdl vs retail + Demo01 bodies
    compare: dict = {"beta_kmdl": None, "retail_kmdl": None, "demo_matches": []}
    retail_kmdl = object_dir / "Kmdl.arc"
    if args.beta_kmdl.is_file() and retail_kmdl.is_file():
        beta_idx = index_arc_files(args.beta_kmdl)
        retail_idx = index_arc_files(retail_kmdl)
        compare["beta_kmdl"] = {
            "path": str(args.beta_kmdl),
            "disk_size": args.beta_kmdl.stat().st_size,
            "files": {n: {"size": s, "sha16": h} for n, (s, h) in sorted(beta_idx.items())},
        }
        compare["retail_kmdl"] = {
            "path": str(retail_kmdl),
            "disk_size": retail_kmdl.stat().st_size,
            "files": {n: {"size": s, "sha16": h} for n, (s, h) in sorted(retail_idx.items())},
        }
        size_diffs = []
        for n in sorted(set(beta_idx) | set(retail_idx)):
            bs = beta_idx.get(n)
            rs = retail_idx.get(n)
            if bs != rs:
                size_diffs.append(
                    {
                        "name": n,
                        "beta": None if not bs else {"size": bs[0], "sha16": bs[1]},
                        "retail": None if not rs else {"size": rs[0], "sha16": rs[1]},
                    }
                )
        compare["kmdl_diffs"] = size_diffs

        # Cross-hash Demo leftovers against beta Kmdl payloads
        beta_by_sha = {h: (n, s) for n, (s, h) in beta_idx.items()}
        for r in all_rows:
            hit = beta_by_sha.get(r["sha16"])
            if hit:
                compare["demo_matches"].append(
                    {
                        "demo_arc": r["arc"],
                        "demo_name": r["name"],
                        "beta_name": hit[0],
                        "size": r["size"],
                        "sha16": r["sha16"],
                    }
                )

    # Known seed arcs from community trail
    seed = {}
    for name in ("Demo01_00.arc", "Demo04_01.arc", "Demo01_01.arc"):
        p = object_dir / name
        if p.is_file():
            seed[name] = [
                {"name": n, "size": s, "sha16": h}
                for n, (s, h) in sorted(index_arc_files(p).items())
                if any(
                    k in n.lower()
                    for k in (
                        "original",
                        "high",
                        "tmp",
                        "henkei",
                        "hiface",
                        "link",
                        "wb_",
                        "ctz",
                        "kantera",
                        "demo00",
                    )
                )
            ]

    summary = {
        "demo_arc_count": len(demos),
        "tagged_file_count": len(all_rows),
        "arcs_with_hits": sum(1 for n, c in by_arc.items() if c),
        "priority_model_count": len(priority),
        "by_arc_hits": {k: v for k, v in sorted(by_arc.items()) if v},
        "tag_histogram": {
            tag: sum(1 for r in all_rows if tag in r["tags"].split(","))
            for tag in sorted(priority_tags | {"model", "anim"})
        },
        "compare": compare,
        "seed_arc_highlights": seed,
        "csv": str(csv_path),
    }

    summary_path = args.out_dir / "demo_leftovers_summary.json"
    summary_path.write_text(json.dumps(summary, indent=2), encoding="utf-8")

    md_path = args.out_dir / "DEMO_LEFTOVERS.md"
    lines = [
        "# Demo arc leftovers — scan report",
        "",
        f"- Object dir: `{object_dir}`",
        f"- Demo arcs scanned: **{len(demos)}**",
        f"- Tagged leftover files: **{len(all_rows)}**",
        f"- Arcs with hits: **{summary['arcs_with_hits']}**",
        f"- Priority models: **{len(priority)}**",
        "",
        "## Seed trail (community → disc)",
        "",
        "| Claim | Disc evidence |",
        "|---|---|",
        "| Beta Link in Demo01_00 | `demo01_link_cut00_bd_original_o.bmd` / `_high_o.bmd` |",
        "| First transform faces in Demo04_01 | `…hiface…` vs `…henkeiface…` |",
        "| Demo00 ancestry leftovers | `demo00_*_tmp` names inside later demos |",
        "| Adapted gameplay pack | `D:/XXXXXXX/beta link/Kmdl.arc` (same names as retail Kmdl, different BMDs) |",
        "",
        "## Arcs with leftover hits",
        "",
    ]
    for arc_name, count in sorted(
        ((k, v) for k, v in by_arc.items() if v), key=lambda kv: (-kv[1], kv[0])
    ):
        lines.append(f"- `{arc_name}` — {count}")
    lines += ["", "## Priority models (extract candidates)", ""]
    for r in sorted(priority, key=lambda x: (x["arc"], x["name"])):
        lines.append(
            f"- `{r['arc']}` / `{r['name']}` — {r['size']} B — tags=`{r['tags']}` — sha16=`{r['sha16']}`"
        )

    if compare.get("kmdl_diffs"):
        lines += ["", "## Beta vs retail Kmdl diffs", ""]
        for d in compare["kmdl_diffs"]:
            b = d["beta"]
            r = d["retail"]
            lines.append(
                f"- `{d['name']}`: beta={b['size'] if b else None} retail={r['size'] if r else None}"
            )
    if compare.get("demo_matches"):
        lines += ["", "## Exact SHA matches: demo file ↔ beta Kmdl member", ""]
        for m in compare["demo_matches"]:
            lines.append(
                f"- `{m['demo_arc']}` `{m['demo_name']}` == beta `{m['beta_name']}` ({m['size']} B)"
            )
    else:
        lines += [
            "",
            "## Exact SHA matches: demo file ↔ beta Kmdl member",
            "",
            "_None — confirms the beta Kmdl pack is an **adapted** remount, not a raw demo extract._",
            "",
        ]

    lines += [
        "",
        "## Reproduce",
        "",
        "```bat",
        "python tools/demo_cut_content/scan_demo_leftovers.py --extract",
        "```",
        "",
        f"Full CSV: `{csv_path.name}`",
        "",
    ]
    md_path.write_text("\n".join(lines), encoding="utf-8")

    if args.extract:
        extract_root = args.out_dir / "extract"
        want: dict[str, set[str]] = defaultdict(set)
        for arc_name, files in seed.items():
            for x in files:
                want[arc_name].add(x["name"])
        for r in priority:
            want[r["arc"]].add(r["name"])
        total = 0
        for arc_name, names in sorted(want.items()):
            if not names:
                continue
            written = extract_named(
                object_dir / arc_name, names, extract_root / Path(arc_name).stem
            )
            total += len(written)
            print(f"Extracted {len(written)} from {arc_name}")
        print(f"Extract total: {total} -> {extract_root}")

    print(f"Wrote {csv_path}")
    print(f"Wrote {summary_path}")
    print(f"Wrote {md_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
