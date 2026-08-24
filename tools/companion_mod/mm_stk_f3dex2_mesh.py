#!/usr/bin/env python3
"""Export MM object_stk Skull Kid meshes from ROM (F3DEX2 display lists) to OBJ+MTL.

Used for side-by-side Blender comparison with TP E_PM. Parses limb/prop display lists
from zeldaret object_stk.xml offsets; does not require ZAPD or Fast64.
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
import xml.etree.ElementTree as ET
from dataclasses import dataclass, field
from io import BytesIO
from pathlib import Path

try:
    from PIL import Image
    import pigment64
except ImportError:
    raise SystemExit("pip install Pillow pigment64")

sys.path.insert(0, str(Path(__file__).resolve().parent))
from mm_extract_object_stk import (  # noqa: E402
    DMADATA_OFF,
    ROM,
    decode_tex,
    load_blob,
    load_texture_defs,
    object_stk_vrom,
    parse_dmadata,
)

# F3DEX2 microcode (Majora's Mask)
G_VTX = 0x01
G_TRI1 = 0x05
G_TRI2 = 0x06
G_QUAD = 0x07
G_DL = 0xDE
G_ENDDL = 0xDF
G_SETTIMG = 0xFD
G_SETTILE = 0xF5
G_LOADBLOCK = 0xF3

SCALE = 100.0  # N64 s16 coords -> Blender-ish units

WORK = Path(r"%USERPROFILE%\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\object_stk")
XML = WORK / "object_stk.xml"
DEFAULT_OUT = WORK / "mesh_export"

# Primary body DLs + head/eyes (head drawn in DmStk_PostLimbDraw on N64)
BODY_DLS = [
    "gSkullKidTorsoDL",
    "gSkullKidPelvisDL",
    "gSkullKidHatBrimDL",
    "gSkullKidHatRingsDL",
    "gSkullKidHatNarrowSectionDL",
    "gSkullKidHatTopDL",
    "gSkullKidLeftUpperArmDL",
    "gSkullKidLeftForearmDL",
    "gSkullKidLeftHandAndFluteDL",
    "gSkullKidRightUpperArmDL",
    "gSkullKidRightForearmDL",
    "gSkullKidRightHandDL",
    "gSkullKidLeftThighDL",
    "gSkullKidLeftShinDL",
    "gSkullKidLeftFootDL",
    "gSkullKidRightThighDL",
    "gSkullKidRightShinDL",
    "gSkullKidRightFootDL",
    "gSkullKidNormalHeadDL",
    "gSkullKidNormalEyesDL",
]

# Hardcoded rest-pose limb translations (StandardLimb.jointPos, mm decomp layout).
# Source: zeldaret/mm object_stk — Flex skeleton limb table (21 limbs).
LIMB_TRANS: dict[str, tuple[int, int, int]] = {
    "gSkullKidRootLimb": (0, 0, 0),
    "gSkullKidPelvisLimb": (0, 0, 0),
    "gSkullKidRightThighLimb": (0, 0, 0),
    "gSkullKidRightShinLimb": (0, 0, 0),
    "gSkullKidRightFootLimb": (0, 0, 0),
    "gSkullKidLeftThighLimb": (0, 0, 0),
    "gSkullKidLeftShinLimb": (0, 0, 0),
    "gSkullKidLeftFootLimb": (0, 0, 0),
    "gSkullKidTorsoLimb": (0, 0, 0),
    "gSkullKidLeftUpperArmLimb": (0, 0, 0),
    "gSkullKidLeftForearmLimb": (0, 0, 0),
    "gSkullKidLeftHandLimb": (0, 0, 0),
    "gSkullKidRightUpperArmLimb": (0, 0, 0),
    "gSkullKidRightForearmLimb": (0, 0, 0),
    "gSkullKidRightHandLimb": (0, 0, 0),
    "gSkullKidNeckLimb": (0, 0, 0),
    "gSkullKidHeadLimb": (0, 0, 0),
    "gSkullKidHatBrimLimb": (0, 0, 0),
    "gSkullKidHatRingsLimb": (0, 0, 0),
    "gSkullKidHatNarrowSectionLimb": (0, 0, 0),
    "gSkullKidHatTopLimb": (0, 0, 0),
}

DL_TEXTURE_HINTS: dict[str, str] = {
    "gSkullKidTorsoDL": "skull_kid_shawl",
    "gSkullKidPelvisDL": "skull_kid_pants_and_wrists",
    "gSkullKidHatBrimDL": "skull_kid_shawl",
    "gSkullKidHatRingsDL": "skull_kid_ring",
    "gSkullKidHatNarrowSectionDL": "skull_kid_shawl",
    "gSkullKidHatTopDL": "skull_kid_shawl",
    "gSkullKidLeftUpperArmDL": "skull_kid_skin",
    "gSkullKidLeftForearmDL": "skull_kid_skin",
    "gSkullKidLeftHandAndFluteDL": "skull_kid_hand",
    "gSkullKidRightUpperArmDL": "skull_kid_skin",
    "gSkullKidRightForearmDL": "skull_kid_skin",
    "gSkullKidRightHandDL": "skull_kid_hand",
    "gSkullKidLeftThighDL": "skull_kid_pants_and_wrists",
    "gSkullKidLeftShinDL": "skull_kid_pants_and_wrists",
    "gSkullKidLeftFootDL": "skull_kid_shoe",
    "gSkullKidRightThighDL": "skull_kid_pants_and_wrists",
    "gSkullKidRightShinDL": "skull_kid_pants_and_wrists",
    "gSkullKidRightFootDL": "skull_kid_shoe",
    "gSkullKidNormalHeadDL": "skull_kid_head_skin",
    "gSkullKidNormalEyesDL": "skull_kid_eye",
}

DL_TO_LIMB: dict[str, str] = {
    "gSkullKidTorsoDL": "gSkullKidTorsoLimb",
    "gSkullKidPelvisDL": "gSkullKidPelvisLimb",
    "gSkullKidHatBrimDL": "gSkullKidHatBrimLimb",
    "gSkullKidHatRingsDL": "gSkullKidHatRingsLimb",
    "gSkullKidHatNarrowSectionDL": "gSkullKidHatNarrowSectionLimb",
    "gSkullKidHatTopDL": "gSkullKidHatTopLimb",
    "gSkullKidLeftUpperArmDL": "gSkullKidLeftUpperArmLimb",
    "gSkullKidLeftForearmDL": "gSkullKidLeftForearmLimb",
    "gSkullKidLeftHandAndFluteDL": "gSkullKidLeftHandLimb",
    "gSkullKidRightUpperArmDL": "gSkullKidRightUpperArmLimb",
    "gSkullKidRightForearmDL": "gSkullKidRightForearmLimb",
    "gSkullKidRightHandDL": "gSkullKidRightHandLimb",
    "gSkullKidLeftThighDL": "gSkullKidLeftThighLimb",
    "gSkullKidLeftShinDL": "gSkullKidLeftShinLimb",
    "gSkullKidLeftFootDL": "gSkullKidLeftFootLimb",
    "gSkullKidRightThighDL": "gSkullKidRightThighLimb",
    "gSkullKidRightShinDL": "gSkullKidRightShinLimb",
    "gSkullKidRightFootDL": "gSkullKidRightFootLimb",
    "gSkullKidNormalHeadDL": "gSkullKidHeadLimb",
    "gSkullKidNormalEyesDL": "gSkullKidHeadLimb",
}


def bit_mask(value: int, shift: int, width: int) -> int:
    return (value >> shift) & ((1 << width) - 1)


def decode_segmented(address: bytes, segments: dict[int, int]) -> int:
    seg = address[0]
    off = int.from_bytes(address[1:4], "big")
    if seg in segments:
        return segments[seg] + off
    return off


def resolve_branch(w1: int, blob_len: int) -> int | None:
    for cand in (
        decode_segmented(w1.to_bytes(4, "big"), {6: 0}),
        w1 & 0xFFFFFF,
        w1 & 0xFFFF,
    ):
        if 0 < cand < blob_len:
            return cand
    return None


@dataclass
class MeshPart:
    name: str
    verts: list[tuple[float, float, float]] = field(default_factory=list)
    uvs: list[tuple[float, float]] = field(default_factory=list)
    faces: list[tuple[int, int, int]] = field(default_factory=list)
    material: str = "default"
    offset: tuple[float, float, float] = (0.0, 0.0, 0.0)


class F3DEX2Parser:
    def __init__(self, blob: bytes, segments: dict[int, int] | None = None):
        self.blob = blob
        self.segments = segments or {6: 0}
        self.vertex_buffer = bytearray(128 * 16)
        self.texture_size = (32, 32)
        self.current_tex: str | None = None

    def _read_vtx(self, w0: int, w1: int) -> None:
        cmd = (w0 << 32) | w1
        num_verts = bit_mask(cmd, 52, 4) + 1
        start_index = bit_mask(cmd, 48, 4)
        data_len = bit_mask(cmd, 32, 16)
        seg_addr = bit_mask(cmd, 0, 32)
        addr = resolve_branch(seg_addr, len(self.blob)) or decode_segmented(
            seg_addr.to_bytes(4, "big"), self.segments
        )
        data = self.blob[addr : addr + data_len]
        for i in range(num_verts):
            base = i * 16
            if base + 16 > len(data):
                break
            x, y, z = struct.unpack_from(">hhh", data, base)
            dst = (start_index + i) * 16
            struct.pack_into(">hhh", self.vertex_buffer, dst, x, y, z)
            self.vertex_buffer[dst + 6 : dst + 16] = data[base + 6 : base + 16]

    def _vtx_pos(self, index: int) -> tuple[float, float, float]:
        o = index * 16
        x, y, z = struct.unpack_from(">hhh", self.vertex_buffer, o)
        # N64 Y-up -> Blender Z-up (Fast64 applies -90° X; we bake a simple swap)
        return (x / SCALE, -z / SCALE, y / SCALE)

    def _vtx_uv(self, index: int) -> tuple[float, float]:
        o = index * 16 + 8
        u, v = struct.unpack_from(">hh", self.vertex_buffer, o)
        tw, th = self.texture_size
        return (u / (32 * tw), 1.0 - v / (32 * th))

    def _tri_indices_f3dex2(self, w0: int, w1: int) -> tuple[int, int, int]:
        packed = w0 & 0xFFFFFF if (w1 & 0xFFFFFF) == 0 else w1 & 0xFFFFFF
        v0 = ((packed >> 16) & 0xFF) // 2
        v1 = ((packed >> 8) & 0xFF) // 2
        v2 = (packed & 0xFF) // 2
        return v0, v1, v2

    def _add_tri(self, part: MeshPart, w0: int, w1: int) -> None:
        i0, i1, i2 = self._tri_indices_f3dex2(w0, w1)
        base = len(part.verts)
        for idx in (i0, i1, i2):
            part.verts.append(self._vtx_pos(idx))
            part.uvs.append(self._vtx_uv(idx))
        part.faces.append((base, base + 1, base + 2))
        if self.current_tex:
            part.material = self.current_tex

    def _add_tri2(self, part: MeshPart, w0: int, w1: int) -> None:
        self._add_tri(part, w0, w1)
        # second triangle indices in w1 high bytes for some encodings
        w0b = (w0 & 0xFFFF) | 0x060000
        self._add_tri(part, w0b, w1)

    def _add_quad(self, part: MeshPart, w0: int, w1: int) -> None:
        packed = w1 & 0xFFFFFF
        v0 = ((packed >> 16) & 0xFF) // 2
        v1 = ((packed >> 8) & 0xFF) // 2
        v2 = (packed & 0xFF) // 2
        v3 = ((w0 >> 16) & 0xFF) // 2
        base = len(part.verts)
        for idx in (v0, v1, v2, v3):
            part.verts.append(self._vtx_pos(idx))
            part.uvs.append(self._vtx_uv(idx))
        part.faces.append((base, base + 1, base + 2))
        part.faces.append((base, base + 2, base + 3))
        if self.current_tex:
            part.material = self.current_tex

    def parse_dl(self, part: MeshPart, start: int, depth: int = 0, seen: set[int] | None = None) -> None:
        if seen is None:
            seen = set()
        off = start
        while off + 8 <= len(self.blob) and off not in seen:
            seen.add(off)
            w0, w1 = struct.unpack_from(">II", self.blob, off)
            op = (w0 >> 24) & 0xFF
            if op == G_VTX:
                self._read_vtx(w0, w1)
            elif op == G_TRI1:
                self._add_tri(part, w0, w1)
            elif op == G_TRI2:
                self._add_tri2(part, w0, w1)
            elif op == G_QUAD:
                self._add_quad(part, w0, w1)
            elif op == G_SETTIMG:
                addr = resolve_branch(w1, len(self.blob))
                if addr is not None:
                    self.current_tex = f"tex_{addr:#x}"
            elif op == G_SETTILE:
                # width/height hints from SETTILE (approximate)
                self.texture_size = (32, 32)
            elif op == G_DL:
                branch = resolve_branch(w1, len(self.blob))
                if branch is not None and depth < 8:
                    self.parse_dl(part, branch, depth + 1, seen)
            elif op == G_ENDDL:
                break
            off += 8


def load_xml_dls(xml_path: Path) -> dict[str, int]:
    root = ET.parse(xml_path).getroot()
    return {el.get("Name"): int(el.get("Offset"), 16) for el in root.iter("DList")}


def export_textures(blob: bytes, tex_defs: list[dict], out_dir: Path) -> dict[str, str]:
    out_dir.mkdir(parents=True, exist_ok=True)
    mapping: dict[str, str] = {}
    for tex in tex_defs:
        off = tex["offset"]
        key = f"tex_{off:#x}"
        raw = blob[off : off + tex["width"] * tex["height"] * 2]
        img = decode_tex(tex["format"], raw, tex["width"], tex["height"])
        fname = f"{tex['out_name']}.png"
        path = out_dir / fname
        img.save(path)
        mapping[key] = tex["out_name"]
        mapping[tex["out_name"]] = fname
    return mapping


def write_obj(parts: list[MeshPart], obj_path: Path, mtl_name: str) -> None:
    obj_path.parent.mkdir(parents=True, exist_ok=True)
    lines = [f"mtllib {mtl_name}", f"o MM_SkullKid"]
    vert_offset = 0
    for part in parts:
        if not part.faces:
            continue
        lines.append(f"g {part.name}")
        ox, oy, oz = part.offset
        for x, y, z in part.verts:
            lines.append(f"v {x + ox:.6f} {y + oy:.6f} {z + oz:.6f}")
        for u, v in part.uvs:
            lines.append(f"vt {u:.6f} {v:.6f}")
        lines.append(f"usemtl {part.material}")
        for a, b, c in part.faces:
            a1, b1, c1 = a + 1 + vert_offset, b + 1 + vert_offset, c + 1 + vert_offset
            lines.append(f"f {a1}/{a1} {b1}/{b1} {c1}/{c1}")
        vert_offset += len(part.verts)
    obj_path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def write_mtl(materials: set[str], tex_dir: Path, mtl_path: Path) -> None:
    lines: list[str] = []
    for mat in sorted(materials):
        lines.append(f"newmtl {mat}")
        lines.append("Ka 1.0 1.0 1.0")
        lines.append("Kd 1.0 1.0 1.0")
        lines.append("Ks 0.0 0.0 0.0")
        lines.append("d 1.0")
        png = tex_dir / f"{mat}.png"
        if png.is_file():
            lines.append(f"map_Kd textures_n64/{mat}.png")
        lines.append("")
    mtl_path.write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser(description="Export MM Skull Kid OBJ from object_stk ROM")
    ap.add_argument("--rom", type=Path, default=ROM)
    ap.add_argument("--xml", type=Path, default=XML)
    ap.add_argument("--out", type=Path, default=DEFAULT_OUT)
    ap.add_argument("--include-props", action="store_true", help="Include mask/flute/ocarina DLs")
    args = ap.parse_args()

    if not args.rom.is_file():
        print("ROM not found:", args.rom, file=sys.stderr)
        return 1
    if not args.xml.is_file():
        print("XML not found:", args.xml, file=sys.stderr)
        return 1

    rom = args.rom.read_bytes()
    blob = load_blob(rom, parse_dmadata(rom, DMADATA_OFF), *object_stk_vrom(rom))
    dls = load_xml_dls(args.xml)
    tex_defs = []
    root = ET.parse(args.xml).getroot()
    for el in root.iter("Texture"):
        tex_defs.append(
            {
                "symbol": el.get("Name"),
                "out_name": el.get("OutName"),
                "format": el.get("Format"),
                "width": int(el.get("Width")),
                "height": int(el.get("Height")),
                "offset": int(el.get("Offset"), 16),
            }
        )

    tex_dir = args.out / "textures_n64"
    tex_key_to_file = export_textures(blob, tex_defs, tex_dir)

    dl_names = list(BODY_DLS)
    if args.include_props:
        dl_names += [
            "gSkullKidFluteDL",
            "gSkullKidOcarinaOfTimeDL",
            "gSkullKidMajorasMask1DL",
            "gSkullKidMajorasMask2DL",
            "gSkullKidMajorasMaskEyesDL",
        ]

    parts: list[MeshPart] = []
    manifest_parts: list[dict] = []
    for name in dl_names:
        if name not in dls:
            continue
        part = MeshPart(name=name)
        parser = F3DEX2Parser(blob)
        parser.parse_dl(part, dls[name])
        part.material = DL_TEXTURE_HINTS.get(name, "skull_kid_skin")
        limb = DL_TO_LIMB.get(name)
        if limb and limb in LIMB_TRANS:
            tx, ty, tz = LIMB_TRANS[limb]
            part.offset = (tx / SCALE, -tz / SCALE, ty / SCALE)
        parts.append(part)
        manifest_parts.append(
            {
                "dl": name,
                "offset": dls[name],
                "triangles": len(part.faces),
                "vertices": len(part.verts),
                "material": part.material,
                "limb": limb,
            }
        )
        print(f"{name}: {len(part.faces)} tris, mat={part.material}")

    materials = {p.material for p in parts if p.faces}
    obj_path = args.out / "MM_SkullKid_body.obj"
    mtl_path = args.out / "MM_SkullKid_body.mtl"
    write_obj(parts, obj_path, mtl_path.name)
    write_mtl(materials, tex_dir, mtl_path)

    manifest = {
        "obj": str(obj_path),
        "mtl": str(mtl_path),
        "textures_n64": str(tex_dir),
        "parts": manifest_parts,
        "total_tris": sum(len(p.faces) for p in parts),
    }
    (args.out / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    print(f"Wrote {obj_path} ({manifest['total_tris']} triangles)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
