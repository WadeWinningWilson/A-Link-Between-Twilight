#!/usr/bin/env python3
"""Scan TP GC ISO room archives for deliver Postman + type-21 EvtArea placements."""

from __future__ import annotations

import argparse
import struct
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass
class IsoEntry:
    name: str
    offset: int
    size: int


@dataclass
class ActorEntry:
    stage: str
    room_arc: str
    name: str
    param: int
    pos: tuple[float, float, float]
    angle: tuple[int, int, int]
    scale: tuple[int, int, int] | None


def read_iso_fst(path: Path) -> list[IsoEntry]:
    with path.open("rb") as f:
        f.seek(0x424)
        fst_off, fst_sz, _max_len = struct.unpack(">III", f.read(12))
        f.seek(fst_off)
        fst_data = f.read(fst_sz)
        num_entries = struct.unpack(">I", fst_data[8:12])[0]

        def entry_u32(idx: int, word: int) -> int:
            off = idx * 12 + word * 4
            return struct.unpack(">I", fst_data[off : off + 4])[0]

        def is_dir(idx: int) -> bool:
            return (entry_u32(idx, 0) & 0xFF000000) != 0

        def entry_name(idx: int) -> str:
            name_off = entry_u32(idx, 0) & 0x00FFFFFF
            string_base = num_entries * 12
            start = string_base + name_off
            end = fst_data.find(b"\x00", start)
            return fst_data[start:end].decode("ascii", errors="replace")

        def child_end(idx: int) -> int:
            return entry_u32(idx, 2)

        out: list[IsoEntry] = []

        def walk(idx: int, prefix: str) -> None:
            name = entry_name(idx)
            full = f"{prefix}/{name}" if prefix and name else (prefix or name)
            if is_dir(idx):
                child = idx + 1
                end = child_end(idx)
                while child < end:
                    walk(child, full)
                    child += 1
            else:
                off = entry_u32(idx, 1)
                size = entry_u32(idx, 2)
                out.append(IsoEntry(full, off, size))

        if num_entries > 0 and is_dir(0):
            walk(0, "")
        return out


def maybe_yaz0(data: bytes) -> bytes:
    if data[:4] != b"Yaz0":
        return data
    dec_size = struct.unpack(">I", data[4:8])[0]
    src = memoryview(data)[0x10:]
    dst = bytearray(dec_size)
    src_i = 0
    dst_i = 0
    bits = 0
    mask = 0
    while dst_i < dec_size and src_i < len(src):
        if mask == 0:
            bits = src[src_i]
            src_i += 1
            mask = 0x80
        if bits & mask:
            if src_i >= len(src):
                break
            dst[dst_i] = src[src_i]
            src_i += 1
            dst_i += 1
        else:
            if src_i + 1 >= len(src):
                break
            b1 = src[src_i]
            b2 = src[src_i + 1]
            src_i += 2
            dist = ((b1 & 0xF) << 8) | b2
            length = (b1 >> 4) + 2
            if dist == 0:
                mask >>= 1
                continue
            copy_from = dst_i - dist
            for _ in range(length):
                if dst_i >= dec_size or copy_from < 0 or copy_from >= dst_i:
                    break
                dst[dst_i] = dst[copy_from]
                dst_i += 1
                copy_from += 1
        mask >>= 1
    return bytes(dst)


def parse_actor_at(data: bytes, base: int, has_scale: bool) -> ActorEntry | None:
    if base + 0x20 > len(data):
        return None
    name = data[base : base + 8].split(b"\x00")[0].decode("ascii", errors="replace")
    if not name:
        return None
    param = struct.unpack(">I", data[base + 8 : base + 12])[0]
    px = struct.unpack(">f", data[base + 12 : base + 16])[0]
    py = struct.unpack(">f", data[base + 16 : base + 20])[0]
    pz = struct.unpack(">f", data[base + 20 : base + 24])[0]
    ax = struct.unpack(">h", data[base + 24 : base + 26])[0]
    ay = struct.unpack(">h", data[base + 26 : base + 28])[0]
    az = struct.unpack(">h", data[base + 28 : base + 30])[0]
    scale = None
    if has_scale and base + 0x24 <= len(data):
        scale = struct.unpack("BBB", data[base + 32 : base + 35])
    return ActorEntry("", "", name, param, (px, py, pz), (ax, ay, az), scale)


def find_named_entries(data: bytes, object_name: str, has_scale: bool) -> list[tuple[int, ActorEntry]]:
    name_field = object_name.encode("ascii")[:8].ljust(8, b"\x00")
    hits: list[tuple[int, ActorEntry]] = []
    idx = 0
    while True:
        base = data.find(name_field, idx)
        if base < 0:
            break
        actor = parse_actor_at(data, base, has_scale)
        if actor is not None and actor.name == object_name:
            hits.append((base, actor))
        idx = base + 1
    return hits


def evtarea_type_and_no(angle_z: int) -> tuple[int, int]:
    typ = angle_z & 0xFF
    no = (angle_z & 0xFF00) >> 8
    if typ == 0xFF:
        typ = 0
    if no == 0xFF:
        no = 0
    return typ, no


def runtime_evtarea_xz_radius(scale_x: float, scale_z: float) -> tuple[float, float]:
    return scale_x * 1000.0, scale_z * 1000.0


def scan_iso(iso_path: Path) -> tuple[list[ActorEntry], list[ActorEntry]]:
    entries = read_iso_fst(iso_path)
    room_arcs = [e for e in entries if "/res/Stage/" in e.name and e.name.endswith(".arc")]
    posts: list[ActorEntry] = []
    evt21: list[ActorEntry] = []

    with iso_path.open("rb") as iso:
        for ent in room_arcs:
            parts = ent.name.replace("\\", "/").split("/")
            if len(parts) < 2:
                continue
            stage = parts[-2]
            room_arc = parts[-1].replace(".arc", "")
            iso.seek(ent.offset)
            try:
                data = maybe_yaz0(iso.read(ent.size))
            except Exception:
                continue

            for _base, actor in find_named_entries(data, "Post", has_scale=False):
                if (actor.param & 0xFF) != 1:
                    continue
                actor.stage = stage
                actor.room_arc = room_arc
                posts.append(actor)

            for _base, actor in find_named_entries(data, "EvtArea", has_scale=True):
                typ, _no = evtarea_type_and_no(actor.angle[2])
                if typ != 21:
                    continue
                actor.stage = stage
                actor.room_arc = room_arc
                evt21.append(actor)

    return posts, evt21


def pair_posts_with_rings(
    posts: list[ActorEntry], evt21: list[ActorEntry]
) -> list[tuple[ActorEntry, list[ActorEntry]]]:
    by_key: dict[tuple[str, str], list[ActorEntry]] = {}
    for e in evt21:
        by_key.setdefault((e.stage, e.room_arc), []).append(e)
    return [(p, by_key.get((p.stage, p.room_arc), [])) for p in posts]


def summarize(pairs: list[tuple[ActorEntry, list[ActorEntry]]]) -> None:
    radii_x: list[float] = []
    radii_z: list[float] = []
    ring_counts: list[int] = []
    outer_triggers: list[float] = []
    center_dists: list[float] = []

    print("Vanilla deliver Postman + type-21 EvtArea survey\n")
    print(
        f"{'Stage':<10} {'Room':<8} {'Rings':>5}  {'Scale XZ':>12}  "
        f"{'Runtime XZ':>14}  {'Outer trig':>10}  {'Post-Ring dist':>14}"
    )
    print("-" * 90)

    for post, rings in sorted(pairs, key=lambda t: (t[0].stage, t[0].room_arc)):
        ring_str = "none"
        outer = 0.0
        center = 0.0
        if rings:
            ring_counts.append(len(rings))
            r = rings[0]
            sx, sy, sz = r.scale or (0, 0, 0)
            rx, rz = runtime_evtarea_xz_radius(float(sx), float(sz))
            radii_x.append(rx)
            radii_z.append(rz)
            outer = max(rx, rz) - 700.0
            outer_triggers.append(outer)
            dx = post.pos[0] - r.pos[0]
            dz = post.pos[2] - r.pos[2]
            center = (dx * dx + dz * dz) ** 0.5
            center_dists.append(center)
            ring_str = f"{sx}x{sz} -> {rx:.0f}/{rz:.0f}"

        print(
            f"{post.stage:<10} {post.room_arc:<8} {len(rings):>5}  {ring_str:>12}  "
            f"{(f'{outer:.0f}' if rings else '-'):>14}  "
            f"{(f'{center:.0f}' if rings else '-'):>10}  param=0x{post.param:04X}"
        )
        for i, r in enumerate(sorted(rings, key=lambda x: evtarea_type_and_no(x.angle[2])[1])):
            sx, sy, sz = r.scale or (0, 0, 0)
            rx, rz = runtime_evtarea_xz_radius(float(sx), float(sz))
            _typ, ring_no = evtarea_type_and_no(r.angle[2])
            print(
                f"    ring[{i}] no={ring_no} pos=({r.pos[0]:.1f},{r.pos[1]:.1f},{r.pos[2]:.1f}) "
                f"scale=({sx},{sy},{sz}) runtimeXZ=({rx:.0f},{rz:.0f}) angleY={r.angle[1]}"
            )
        print(
            f"    post pos=({post.pos[0]:.1f},{post.pos[1]:.1f},{post.pos[2]:.1f}) angleY={post.angle[2]}"
        )
        print()

    def stats(vals: list[float]) -> str:
        if not vals:
            return "n/a"
        return f"min={min(vals):.0f} avg={sum(vals)/len(vals):.0f} max={max(vals):.0f} (n={len(vals)})"

    print("Aggregate (first ring per delivery site):")
    print(f"  Sites with rings:    {len([p for p, r in pairs if r])} / {len(pairs)} deliver Postmen")
    print(f"  Rings per site:      {stats([float(x) for x in ring_counts])}")
    print(f"  Runtime X radius:    {stats(radii_x)}")
    print(f"  Runtime Z radius:    {stats(radii_z)}")
    print(f"  Outer-edge trigger:  {stats(outer_triggers)}  (dist >= scale.x - 700)")
    print(f"  Post-ring offset:    {stats(center_dists)}")
    print()
    print("ALBW North Faron (current mod values):")
    print("  scale (2,1,2) -> runtime XZ radii 2000 / 2000")
    print("  trigger: inside ellipse (ALBW bypasses vanilla outer-edge gate)")
    print("  cooldown clear distance: 2500 units from mail spawn")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument(
        "--iso",
        default=r"D:\Taking a Dumpy\Minerva_Myrient\Redump\Nintendo - GameCube - NKit RVZ [zstd-19-128k]\Legend of Zelda, The - Twilight Princess (USA).iso",
    )
    args = ap.parse_args()
    iso_path = Path(args.iso)
    if not iso_path.is_file():
        print(f"ISO not found: {iso_path}", file=sys.stderr)
        return 1
    posts, evt21 = scan_iso(iso_path)
    print(f"Found {len(posts)} deliver Postman placements and {len(evt21)} type-21 EvtAreas\n")
    summarize(pair_posts_with_rings(posts, evt21))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
